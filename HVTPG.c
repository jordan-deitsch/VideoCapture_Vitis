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

#define VTPG_CTRL_APP_START_MASK    (0x1)

/************************** Device Instance Definitions *****************************/
PatternGenerator PatternGenInst;


/************************** Internal Definitions *****************************/
static void HVTPG_SetReg(u32 Offset, u32 Value);
static u32 HVTPG_GetReg(u32  Offset);


int HVTPG_Init(PatternGenerator *PattGenInstPtr)
{
    if(NULL == PattGenInstPtr) {
        return XST_FAILURE;
    }
    
    return XST_SUCCESS;
}

int HVTPG_ConfigureFrame(PatternGenerator *PattGenInstPtr, int Pattern)
{
    if(NULL == PattGenInstPtr) {
        return XST_FAILURE;
    }

    HVTC_GetTimingSettings(&TimingControllerInst);

    HVTPG_SetReg(VTPG_ACTIVE_HEIGHT_REG, (u32)TimingControllerInst.ActiveVsize);
    HVTPG_SetReg(VTPG_ACTIVE_WIDTH_REG, (u32)TimingControllerInst.ActiveHsize);
    HVTPG_SetReg(VTPG_BACKGROUND_PATT_REG, (u32)Pattern);
    
    return XST_SUCCESS;
}

int HVTPG_GenerateFrame(PatternGenerator *PattGenInstPtr)
{
    if(NULL == PattGenInstPtr) {
        return XST_FAILURE;
    }

    u32 RegValue = HVTPG_GetReg(VTPG_CTRL_REG);
    RegValue = RegValue | VTPG_CTRL_APP_START_MASK;
    HVTPG_SetReg(VTPG_CTRL_REG, RegValue);
    
    return XST_SUCCESS;
}

static void HVTPG_SetReg(u32 Offset, u32 Value)
{
    Xil_Out32(VTPG_BASE_ADDR + Offset, Value);
    return;
}

static u32 HVTPG_GetReg(u32  Offset)
{
    return Xil_In32(VTPG_BASE_ADDR + Offset);
}

