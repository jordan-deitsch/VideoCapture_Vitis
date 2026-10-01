/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HTCA9548.h"
#include "xil_printf.h"
#include <xstatus.h>

/************************** Device Instance Definitions *****************************/
TCA9548Device TCA9548Inst;

/************************** Internal Definitions *****************************/
static int HTCA9548_GetReg(TCA9548Device *TCA9548InstPtr, u8 *ValuePtr);
static int HTCA9548_SetReg(TCA9548Device *TCA9548InstPtr, const u8 Value);


// Initialize I2C Switch
int HTCA9548_Init(TCA9548Device *TCA9548InstPtr, IicBus *I2cBusPtr, u8 Address)
{
    if((TCA9548InstPtr == NULL) || (I2cBusPtr == NULL)){
        return XST_FAILURE;
    }

    TCA9548InstPtr->IicBusPtr = I2cBusPtr;
    TCA9548InstPtr->Address = Address;

    int Status = HTCA9548_SwitchSel(TCA9548InstPtr, 0);

    return Status;
}


int HTCA9548_SwitchSel(TCA9548Device *TCA9548InstPtr, u8 SwitchSel)
{
    if(NULL == TCA9548InstPtr){
		return XST_FAILURE;
	}

    TCA9548InstPtr->SwitchPosition = SwitchSel;
    return HTCA9548_SetReg(TCA9548InstPtr, SwitchSel);
}


static int HTCA9548_GetReg(TCA9548Device *TCA9548InstPtr, u8 *ValuePtr)
{
    if((TCA9548InstPtr == NULL) || (ValuePtr == NULL)){
        return XST_FAILURE;
    }
    
    // Pass the current switch position as the write register address for the write cycle of the read operation
    return HIIC_ReadData(TCA9548InstPtr->IicBusPtr, TCA9548InstPtr->Address, TCA9548InstPtr->SwitchPosition, ValuePtr, 1, TCA9548_THDDAT_TIME_NSEC);
}

static int HTCA9548_SetReg(TCA9548Device *TCA9548InstPtr, const u8 Value)
{
	if(NULL == TCA9548InstPtr){
		return XST_FAILURE;
	}

	int NumBytes = 1;
	u8 MsgData[NumBytes];

	//Setup Data packet for write
	MsgData[0] = Value;
    
    return HIIC_WriteData(TCA9548InstPtr->IicBusPtr, TCA9548InstPtr->Address, MsgData, NumBytes, TCA9548_THDDAT_TIME_NSEC);
}