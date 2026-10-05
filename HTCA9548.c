/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HTCA9548.h"
#include <sleep.h>
#include <xstatus.h>


/************************** Device Instance Definitions *****************************/
TCA9548Device TCA9548Inst_Clocks;
TCA9548Device TCA9548Inst_SFP;

/************************** Internal Definitions *****************************/
static int HTCA9548_Init(TCA9548Device *TCA9548InstPtr, IicBus *I2cBusPtr, u8 Address);
static int HTCA9548_GetReg(TCA9548Device *TCA9548InstPtr, u8 *ValuePtr);
static int HTCA9548_SetReg(TCA9548Device *TCA9548InstPtr, u8 RegValue);


// Setup all I2C switches
int HTCA9548_Setup()
{
    int Status = XST_SUCCESS;

    Status = HTCA9548_Init(&TCA9548Inst_Clocks, &IicBusInstMain, TCA9548_CLOCKS_DEVICE_ADDR);
    if(Status != XST_SUCCESS){
        return Status;
    }

    Status = HTCA9548_Init(&TCA9548Inst_SFP, &IicBusInstMain, TCA9548_SFP_DEVICE_ADDR);
    if(Status != XST_SUCCESS){
        return Status;
    }

    return Status;
}

// Initialize I2C Switch
int HTCA9548_Init(TCA9548Device *TCA9548InstPtr, IicBus *I2cBusPtr, u8 Address)
{
    if((TCA9548InstPtr == NULL) || (I2cBusPtr == NULL)){
        return XST_FAILURE;
    }

    int Status = XST_SUCCESS;
    
    TCA9548InstPtr->IicBusPtr = I2cBusPtr;
    TCA9548InstPtr->Address = Address;  

    Status = HTCA9548_SwitchSel(TCA9548InstPtr, e_Switch_DEFAULT);
    if(Status != XST_SUCCESS){
        TCA9548InstPtr->IicPresent = false;
        return Status;
    }
    else {
        TCA9548InstPtr->IicPresent = true;
    }

    return Status;
}


int HTCA9548_SwitchSel(TCA9548Device *TCA9548InstPtr, TCA9548SwitchPosition SwitchPos)
{
    if(NULL == TCA9548InstPtr){
		return XST_FAILURE;
	}

    int Status = XST_SUCCESS;

    TCA9548InstPtr->SwitchPosition = SwitchPos;

    // Default to switch OFF (all 0) if invalid switch position is entered
    u8 RegValue = 0;
    if(SwitchPos < e_Switch_DEFAULT) {
        RegValue = 0x1 << SwitchPos;
    }
    
    Status = HTCA9548_SetReg(TCA9548InstPtr, RegValue);
    if(Status != XST_SUCCESS){
        return Status;
    }

    usleep(TCA9548_SWITCH_DELAY_USEC);  // Allow delay for switch activation
    return Status;
}


static int HTCA9548_GetReg(TCA9548Device *TCA9548InstPtr, u8 *ValuePtr)
{
    if((TCA9548InstPtr == NULL) || (ValuePtr == NULL)){
        return XST_FAILURE;
    }
    
    // Pass the current switch position as the write register address for the write cycle of the read operation
    return HIIC_ReadData(TCA9548InstPtr->IicBusPtr, TCA9548InstPtr->Address, (u8)(1 << TCA9548InstPtr->SwitchPosition), ValuePtr, 1, TCA9548_THDDAT_TIME_NSEC);
}

static int HTCA9548_SetReg(TCA9548Device *TCA9548InstPtr, u8 RegValue)
{
	if(NULL == TCA9548InstPtr){
		return XST_FAILURE;
	}

	int NumBytes = 1;
	u8 MsgData[NumBytes];

	//Setup Data packet for write
	MsgData[0] = RegValue;
    
    return HIIC_WriteData(TCA9548InstPtr->IicBusPtr, TCA9548InstPtr->Address, MsgData, NumBytes, TCA9548_THDDAT_TIME_NSEC);
}