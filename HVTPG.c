/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HVTPG.h"
#include "xil_printf.h"
#include "xil_io.h"
#include <xstatus.h>

#define VTPG_CTRL_REG                (0x00)

#define VTPG_CTRL_REGAPP_START_MASK  (0x1)


int HVTPG_Init()
{
    return XST_SUCCESS;
}

int HVTPG_EnableController(bool Enable)
{
    return XST_SUCCESS;
}

