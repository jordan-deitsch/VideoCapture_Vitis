/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HVTC_H
#define HVTC_H

#include "xparameters.h"
#include "xil_types.h"
#include <stdbool.h>


#define VTC_BASE_ADDR (XPAR_V_TC_0_BASEADDR)

typedef struct TimingController
{
	u16 ActiveVsize;
    u16 ActiveHsize;
} TimingController;

/************************** Peripheral Device Declarations *****************************/
extern TimingController TimingControllerInst;

/************************** Function Declarations *****************************/
int HVTC_Init(TimingController *TimeCtrlInstPtr);
int HVTC_EnableController(TimingController *TimeCtrlInstPtr, bool Enable);
int HVTC_GetTimingSettings(TimingController *TimeCtrlInstPtr);
int HVTC_UpdateRegisters(TimingController *TimeCtrlInstPtr);

// TEMP
void HVTC_SetReg(u32 Offset, u32 Value);
u32 HVTC_GetReg(u32 Offset);

#endif