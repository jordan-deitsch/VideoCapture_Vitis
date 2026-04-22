/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HVTPG.h"
#include "HVTC.h"
#include "xil_printf.h"
#include "xil_io.h"
#include <xstatus.h>

#define VTPG_CTRL_REG               (0x00)
#define VTPG_ACTIVE_HEIGHT_REG      (0x10)
#define VTPG_ACTIVE_WIDTH_REG       (0x18)
#define VTPG_BACKGROUND_PATT_REG    (0x20)
#define VTPG_OVERLAY_ID_REG         (0x28)
#define VTPG_MASK_ID_REG            (0x30)
#define VTPG_MOTION_SPEED_REG       (0x38)
#define VTPG_COLOR_FORMAT_REG       (0x40)
#define VTPG_CROSS_HORIZONTAL_REG   (0x48)
#define VTPG_CROSS_VERTICAL_REG     (0x50)

#define VTPG_CTRL_APP_START_MASK    (0x01)
#define VTPG_CTRL_APP_AUTORUN_MASK  (0x80)

/************************** Device Instance Definitions *****************************/
PatternGenerator PatternGenInst;


/************************** Internal Definitions *****************************/
static void HVTPG_SetReg(u32 Offset, u32 Value);
static u32 HVTPG_GetReg(u32  Offset);

static u32 s_TargetHorizontal = 0;
static u32 s_TargetVertical = 0;
static bool s_HorizontalUp = true;
static bool s_VerticalUp = true;


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

    PattGenInstPtr->ActiveHsize = TimingControllerInst.ActiveHsize;
    PattGenInstPtr->ActiveVsize = TimingControllerInst.ActiveVsize;

    HVTPG_SetReg(VTPG_ACTIVE_HEIGHT_REG, (u32)TimingControllerInst.ActiveVsize);
    HVTPG_SetReg(VTPG_ACTIVE_WIDTH_REG, (u32)TimingControllerInst.ActiveHsize);
    HVTPG_SetReg(VTPG_BACKGROUND_PATT_REG, (u32)Pattern);
    HVTPG_SetReg(VTPG_COLOR_FORMAT_REG, 0x2);
    
    return XST_SUCCESS;
}

int HVTPG_GenerateContinuous(PatternGenerator *PattGenInstPtr)
{
    if(NULL == PattGenInstPtr) {
        return XST_FAILURE;
    }

    u32 RegValue = HVTPG_GetReg(VTPG_CTRL_REG);
    RegValue = RegValue | VTPG_CTRL_APP_START_MASK | VTPG_CTRL_APP_AUTORUN_MASK;
    HVTPG_SetReg(VTPG_CTRL_REG, RegValue);
    
    return XST_SUCCESS;
}

int HVTPG_MovingTarget(PatternGenerator *PattGenInstPtr, int Pattern)
{
    HVTPG_SetReg(VTPG_BACKGROUND_PATT_REG, Pattern);
    HVTPG_SetReg(VTPG_OVERLAY_ID_REG, 0x2);

    if(s_HorizontalUp)
    {
        s_TargetHorizontal++;
    }
    else
    {
        s_TargetHorizontal--;;
    }    

    if(s_VerticalUp)
    {
        s_TargetVertical++;
    }
    else
    {
        s_TargetVertical--;
    }

    if((s_TargetHorizontal <= 0) || (s_TargetHorizontal >= PattGenInstPtr->ActiveHsize))
    {
        s_HorizontalUp = !s_HorizontalUp;
    }
    
    if((s_TargetVertical <= 0) || (s_TargetVertical >= PattGenInstPtr->ActiveVsize))
    {
        s_VerticalUp = !s_VerticalUp;
    }
    

    HVTPG_SetReg(VTPG_CROSS_HORIZONTAL_REG, s_TargetHorizontal);
    HVTPG_SetReg(VTPG_CROSS_VERTICAL_REG, s_TargetVertical);
    
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

