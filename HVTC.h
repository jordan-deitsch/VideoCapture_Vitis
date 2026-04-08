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
#include <stdbool.h>


#define VTC_ADDR (XPAR_V_TC_0_BASEADDR)

/************************** Function Declarations *****************************/
int HVTC_Init();
int HVTC_EnableController(bool Enable);

#endif