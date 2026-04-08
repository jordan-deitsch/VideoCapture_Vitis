/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HVTPG_H
#define HVTPG_H

#include "xparameters.h"
#include <stdbool.h>


#define VTPG_ADDR (XPAR_V_HDMI_TPG_BASEADDR)

/************************** Function Declarations *****************************/
int HVTPG_Init();
int HVTPG_EnableController(bool Enable);

#endif