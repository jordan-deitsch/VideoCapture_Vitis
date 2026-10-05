/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HSI570.h"
#include "HIIC.h"
#include "HTCA9548.h"
#include <xstatus.h>

#define POR_OUTPUT_FREQ (156250000.0f)	// Power-on Reset output frequency = 156.25 MHz
#define RFREQ_MULTIPLIER (268435456.0f)	// FXP multiplier for RFREQ = 2^28
#define HSDIV_LOOKUP_LENGTH (8)	// Length of lookup table for HSDIV

/************************** Device Instance Definitions *****************************/
SI570Device SI570Inst;

/************************** Internal Definitions *****************************/
static int HSI570_FindIndexHSDIV(u8 Value);
static int HSI570_ResetRegisters(SI570Device *SI570InstPtr);
static int HSI570_ReadConfig(SI570Device *SI570InstPtr);
static int HSI570_GetReg(SI570Device *SI570InstPtr, const u8 RegAddr, u8 *ValuePtr);
static int HSI570_SetReg(SI570Device *SI570InstPtr, const u8 RegAddr, const u8 Value);

/************************** Constant Definitions *****************************/
SI570FreqSettings SI570Freq_POR = {
    .OutputFreq = POR_OUTPUT_FREQ,
    .HSDIV      = 0,		// Will get during initialization
    .N1         = 0,		// Will get during initialization
    .RFREQ      = 0x0UL		// Will get during initialization
};

SI570FreqSettings SI570Freq_297_MHz = {
    .OutputFreq = 297000000.0f,
    .HSDIV      = 9,
    .N1         = 2,
    .RFREQ      = 0x0UL		// Will calculate during initialization based on FreqXTAL
};

SI570FreqSettings SI570Freq_296p7_MHz = {
    .OutputFreq = 296700000.0f,
    .HSDIV      = 9,
    .N1         = 2,
    .RFREQ      = 0x0UL		// Will calculate during initialization based on FreqXTAL
};

SI570FreqSettings *SI570FreqPtrArr[] = {&SI570Freq_297_MHz, &SI570Freq_296p7_MHz};
u8 HSDIV_Lookup[] = {4, 5, 6, 7, 0, 9, 0, 11};	// Values corresponding to the [2:0] HSDIV register slice

// Initialize Programmable Clock Source
int HSI570_Init(SI570Device *SI570InstPtr, IicBus *I2cBusPtr, u8 Address)
{
	if((SI570InstPtr == NULL) || (I2cBusPtr == NULL)){
        return XST_FAILURE;
    }

	int Status = XST_SUCCESS;

	SI570InstPtr->IicBusPtr = I2cBusPtr;
	SI570InstPtr->Address = Address;
	SI570InstPtr->SwitchPos = e_Switch_SI570;
	SI570InstPtr->CurrentSettings = &SI570Freq_POR;

	// Set switch to allow communication with endpoint device
	Status = HTCA9548_SwitchSel(&TCA9548Inst_Clocks, SI570InstPtr->SwitchPos);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Reset the NVM registers at initialization
	Status = HSI570_ResetRegisters(SI570InstPtr);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Read the configutration registers to calculate the XTAL frequency
	Status = HSI570_ReadConfig(SI570InstPtr);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Calculate the RFREQ values for the preset frequncies
	for(int i=0; i<e_SI570_TOTAL_PRESETS; i++)
	{
		float FreqDCO = SI570FreqPtrArr[i]->OutputFreq * SI570FreqPtrArr[i]->HSDIV * SI570FreqPtrArr[i]->N1;
		float RFFREQ_float = (FreqDCO / SI570InstPtr->FreqXTAL) * RFREQ_MULTIPLIER;
		SI570FreqPtrArr[i]->RFREQ = (u64)RFFREQ_float;
	}

	// Set the registers for default output frequency
	HSI570_SetFrequency(SI570InstPtr, e_SI570_297_MHz);
	if(Status != XST_SUCCESS){
		return Status;
	}
	
	return XST_SUCCESS;
}

static int HSI570_FindIndexHSDIV(u8 Value)
{
    for (int i = 0; i < HSDIV_LOOKUP_LENGTH; i++)
    {
        if (HSDIV_Lookup[i] == Value)
            return i;
    }

    return -1;
}


int HSI570_SetFrequency(SI570Device *SI570InstPtr, SI570PresetFrequencies FreqSel)
{
	if(NULL == SI570InstPtr){
		return XST_FAILURE;
	}

	if(FreqSel >= e_SI570_TOTAL_PRESETS){
		return XST_FAILURE;
	}
	
	int Status = XST_SUCCESS;

	// Set switch to allow communication with endpoint device
	Status = HTCA9548_SwitchSel(&TCA9548Inst_Clocks, SI570InstPtr->SwitchPos);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Freeze the DCO
	Status = HSI570_SetReg(SI570InstPtr, SI570_FREEZE_DCO_REG, SI570_FREEZE_DCO_MASK);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Find HSDIV index to map value
	int HSDIV_Index = HSI570_FindIndexHSDIV(SI570FreqPtrArr[FreqSel]->HSDIV);
	if(HSDIV_Index < 0){
		return XST_FAILURE;
	}	

	// Update all configuration registers with continuous IIC write
	u8 TxMsg[SI570_NUM_CONFIG_REGS+1];

	TxMsg[0] = SI570_DIVIDERS_REG; 	// Start write with the dividers register address
	
	TxMsg[1] = (((u8)HSDIV_Index << SI570_DIVIDERS_HSDIV_OFFSET) & SI570_DIVIDERS_HSDIV_MASK) | 
					(((SI570FreqPtrArr[FreqSel]->N1 - 1) >> SI570_DIVIDERS_N1_6_2_OFFSET) & SI570_DIVIDERS_N1_6_2_MASK );

	TxMsg[2] = (((SI570FreqPtrArr[FreqSel]->N1 - 1) << SI570_DIVIDERS_N1_1_0_OFFSET) & SI570_DIVIDERS_N1_1_0_MASK ) |
					(u8)((SI570FreqPtrArr[FreqSel]->RFREQ >> 32) & SI570_RFREQ_37_32_MASK);

	TxMsg[3] = (u8)((SI570FreqPtrArr[FreqSel]->RFREQ >> 24) & SI570_RFREQ_31_0_MASK);
	TxMsg[4] = (u8)((SI570FreqPtrArr[FreqSel]->RFREQ >> 16) & SI570_RFREQ_31_0_MASK);
	TxMsg[5] = (u8)((SI570FreqPtrArr[FreqSel]->RFREQ >> 8 ) & SI570_RFREQ_31_0_MASK);
	TxMsg[6] = (u8)( SI570FreqPtrArr[FreqSel]->RFREQ        & SI570_RFREQ_31_0_MASK);

	Status = HIIC_WriteData(SI570InstPtr->IicBusPtr, SI570InstPtr->Address, TxMsg, SI570_NUM_CONFIG_REGS+1, SI570_THDDAT_TIME_NSEC);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Unfreeze DCO
	Status = HSI570_SetReg(SI570InstPtr, SI570_FREEZE_DCO_REG, 0x00);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Assert New Frequency bit (must happen within 10 msec of Unfreeze DCO)
	Status = HSI570_SetReg(SI570InstPtr, SI570_RESET_MEM_CTRL_REG, SI570_NEW_FREQ_MASK);
	if(Status != XST_SUCCESS){
		return Status;
	}

	// Update current frequency settings of SI570
	SI570InstPtr->CurrentSettings = SI570FreqPtrArr[FreqSel];

	return Status;
}


static int HSI570_ResetRegisters(SI570Device *SI570InstPtr)
{
	if(NULL == SI570InstPtr){
		return XST_FAILURE;
	}
	
	int Status = XST_SUCCESS;
	

	Status = HSI570_SetReg(SI570InstPtr, SI570_RESET_MEM_CTRL_REG, SI570_RECALL_MASK);
	if(Status != XST_SUCCESS){
		SI570InstPtr->IicPresent = false;
		return Status;
	}
	else {
		SI570InstPtr->IicPresent = true;
	}
	
	u8 RegValue = 0;
	while(1)
	{
		// Poll NVM Recall bit to confirm initial starting conditions
		Status = HSI570_GetReg(SI570InstPtr, SI570_RESET_MEM_CTRL_REG, &RegValue);
		if(Status != XST_SUCCESS){
			return Status;
		}

		// Wait for NVM Recall bit to self-clear
		if((RegValue & SI570_RECALL_MASK) == 0){
			break;
		}
	}
	
	return XST_SUCCESS;
}


static int HSI570_ReadConfig(SI570Device *SI570InstPtr)
{
	if(NULL == SI570InstPtr){
		return XST_FAILURE;
	}

	int Status = XST_SUCCESS;

	// Read back all timing configuration registers
	u8 ConfigRegs[SI570_NUM_CONFIG_REGS];
	Status = HIIC_ReadData(SI570InstPtr->IicBusPtr, SI570InstPtr->Address, SI570_DIVIDERS_REG, ConfigRegs, SI570_NUM_CONFIG_REGS, SI570_THDDAT_TIME_NSEC);
	
	// Parse timing configuration registers
	SI570InstPtr->CurrentSettings->HSDIV = HSDIV_Lookup[(ConfigRegs[0] & SI570_DIVIDERS_HSDIV_MASK) >> SI570_DIVIDERS_HSDIV_OFFSET];
	
	SI570InstPtr->CurrentSettings->N1 = (((ConfigRegs[0] & SI570_DIVIDERS_N1_6_2_MASK) << SI570_DIVIDERS_N1_6_2_OFFSET) | 
										 ((ConfigRegs[1] & SI570_DIVIDERS_N1_1_0_MASK) >> SI570_DIVIDERS_N1_1_0_OFFSET)   ) + 1; 
						
	SI570InstPtr->CurrentSettings->RFREQ =  (((u64)ConfigRegs[1] & SI570_RFREQ_37_32_MASK) << 32) |
						   					((u64)ConfigRegs[2] << 24) |
						   					((u64)ConfigRegs[3] << 16) |
						   					((u64)ConfigRegs[4] << 8) |
						   					(u64)ConfigRegs[5];
	
	SI570InstPtr->FreqXTAL = SI570InstPtr->CurrentSettings->OutputFreq * 
							(float)SI570InstPtr->CurrentSettings->HSDIV * 
							(float)SI570InstPtr->CurrentSettings->N1 /
							((float)SI570InstPtr->CurrentSettings->RFREQ / RFREQ_MULTIPLIER);
	
	return Status;
}


static int HSI570_GetReg(SI570Device *SI570InstPtr, const u8 RegAddr, u8 *ValuePtr)
{
    if((SI570InstPtr == NULL) || (ValuePtr == NULL)){
        return XST_FAILURE;
    }
    
    return HIIC_ReadData(SI570InstPtr->IicBusPtr, SI570InstPtr->Address, RegAddr, ValuePtr, 1, SI570_THDDAT_TIME_NSEC);
}

static int HSI570_SetReg(SI570Device *SI570InstPtr, const u8 RegAddr, const u8 Value)
{
	if(NULL == SI570InstPtr){
		return XST_FAILURE;
	}

	int NumBytes = 2;
	u8 MsgData[NumBytes];

	//Setup Data packet for write
	MsgData[0] = RegAddr;
	MsgData[1] = Value;
    
    return HIIC_WriteData(SI570InstPtr->IicBusPtr, SI570InstPtr->Address, MsgData, NumBytes, SI570_THDDAT_TIME_NSEC);
}