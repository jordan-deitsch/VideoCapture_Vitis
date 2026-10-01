/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HSI570.h"
#include "xil_printf.h"
#include <xstatus.h>

/************************** Device Instance Definitions *****************************/
SI570Device SI570Inst;

/************************** Internal Definitions *****************************/
static int HSI570_GetReg(SI570Device *SI570InstPtr, const u8 RegAddr, u8 *ValuePtr);
static int HSI570_SetReg(SI570Device *SI570InstPtr, const u8 RegAddr, const u8 Value);


// Initialize Programmable Clock Source
int HSI570_Init(SI570Device *SI570InstPtr, IicBus *I2cBusPtr, u8 Address)
{
	if((SI570InstPtr == NULL) || (I2cBusPtr == NULL)){
        return XST_FAILURE;
    }

	SI570InstPtr->IicBusPtr = I2cBusPtr;
	SI570InstPtr->Address = Address;

	return XST_SUCCESS;
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