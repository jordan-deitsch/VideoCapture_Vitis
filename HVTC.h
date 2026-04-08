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


#define VTC_ADDR (XPAR_V_TC_0_BASEADDR)

typedef struct TimingController
{
	u16 ActiveVsize;
    u16 ActiveHsize;
} TimingController;

/************************** Peripheral Device Declarations *****************************/


/************************** Function Declarations *****************************/
int HVTC_Init();
int HVTC_EnableController(bool Enable);

#endif