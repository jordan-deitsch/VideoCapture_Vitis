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

#define VTPG_CTRL_REG               (0x00)
#define VTPG_ACTIVE_HEIGHT_REG      (0x10)
#define VTPG_ACTIVE_WIDTH_REG       (0x18)
#define VTPG_BACKGROUND_PATT_REG    (0x20)
#define VTPG_OVERLAY_ID             (0x28)
#define VTPG_MASK_ID_REG            (0x30)
#define VTPG_MOTION_SPEED_REG       (0x38)
#define VTPG_COLOR_FORMAT_REG       (0x40)
#define VTPG_CROSS_HORIZONTAL_REG   (0x48)
#define VTPG_CROSS_VERTICAL_REG     (0x50)

#define VTPG_CTRL_REGAPP_START_MASK  (0x1)


int HVTPG_Init()
{
    return XST_SUCCESS;
}

int HVTPG_EnableController(bool Enable)
{
    return XST_SUCCESS;
}

