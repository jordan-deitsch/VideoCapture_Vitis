/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#include "HVTC.h"
#include "xil_printf.h"
#include "xil_io.h"
#include <xstatus.h>

#define VTC_CTRL_REG                (0x00)
#define VTC_STATUS_REG              (0x04)
#define VTC_ERROR_REG               (0x08)
#define VTC_IRQ_ENABLE_REG          (0x0C)
#define VTC_GEN_ACTIVE_SIZE_F0_REG  (0x60)
#define VTC_GEN_TIMING_STATUS_REG   (0x64)
#define VTC_GEN_ENCODING_REG        (0x68)
#define VTC_GEN_POLARITY_REG        (0x6C)
#define VTC_GEN_HSIZE_REG           (0x70)
#define VTC_GEN_VSIZE_REG           (0x74)
#define VTC_GEN_HSYNC_REG           (0x78)
#define VTC_GEN_F0_VBLANK_H_REG     (0x7C)
#define VTC_GEN_F0_VSYNC_V_REG      (0x80)
#define VTC_GEN_F0_VSYNC_H_REG      (0x84)
#define VTC_GEN_ACTIVE_SIZE_F1_REG  (0x94)
#define VTC_GEN_FRAME_SYNC_REG      (0x100)
#define VTC_GEN_GLOBAL_DELAY_REG    (0x140)

#define VTC_CTRL_REG_DEFAULT            (0x07F7EF00)    // 0000_0111_1111_0111_1110_1111_0000_0000
#define VTC_CTRL_REG_GEN_ENABLE_MASK    (0x4)
#define VTC_CTRL_REG_UPDATE_MASK        (0x2)
#define VTC_CTRL_REG_SW_ENABLE_MASK     (0x1)

/************************** Device Instance Definitions *****************************/
TimingController TimingControllerInst;


/************************** Variable Definitions *****************************/


/************************** Internal Definitions *****************************/
// static void HVTC_SetReg(u32 Offset, u32 Value);
// static u32 HVTC_GetReg(u32 Offset);


int HVTC_Init(TimingController *TimeCtrlInstPtr)
{    
    if(NULL == TimeCtrlInstPtr) {
        return XST_FAILURE;
    }
    
    HVTC_SetReg(VTC_CTRL_REG, VTC_CTRL_REG_DEFAULT);

    // TEMPORARY SETTINGS
    // HVTC_SetReg(VTC_GEN_ACTIVE_SIZE_F0_REG, (u32)((50 << 16) | 50));
    // HVTC_SetReg(VTC_GEN_HSIZE_REG,          (u32)64);
    // HVTC_SetReg(VTC_GEN_VSIZE_REG,          (u32)64);
    // HVTC_SetReg(VTC_GEN_HSYNC_REG,          (u32)((60 << 16) | 55));
    // HVTC_SetReg(VTC_GEN_F0_VBLANK_H_REG,    (u32)((63 << 16) | 50));    // When is horizontal blanking during vertical blanking lines
    // HVTC_SetReg(VTC_GEN_F0_VSYNC_V_REG,     (u32)((60 << 16) | 55));    // Which lines assert VSYNC
    // HVTC_SetReg(VTC_GEN_F0_VSYNC_H_REG,     (u32)((63 << 16) | 0));    // Which pixel in VSYNC lines assert VSYNC

    HVTC_UpdateRegisters(TimeCtrlInstPtr);
    HVTC_GetTimingSettings(TimeCtrlInstPtr);
    
    return XST_SUCCESS;
}

int HVTC_EnableController(TimingController *TimeCtrlInstPtr, bool Enable)
{
    if(NULL == TimeCtrlInstPtr) {
        return XST_FAILURE;
    }
    
    u32 RegValue = HVTC_GetReg(VTC_CTRL_REG);
    u32 MaskValue = VTC_CTRL_REG_GEN_ENABLE_MASK | VTC_CTRL_REG_SW_ENABLE_MASK;
    
    if(Enable){
        RegValue = RegValue | MaskValue;
    }
    else{
        RegValue = RegValue & (~MaskValue);
    }

    HVTC_SetReg(VTC_CTRL_REG, RegValue);
    return XST_SUCCESS;
}

int HVTC_UpdateRegisters(TimingController *TimeCtrlInstPtr)
{
    if(NULL == TimeCtrlInstPtr) {
        return XST_FAILURE;
    }

    u32 RegValue = HVTC_GetReg(VTC_CTRL_REG);
    u32 MaskValue = VTC_CTRL_REG_UPDATE_MASK;
    RegValue = RegValue | MaskValue;
    
    HVTC_SetReg(VTC_CTRL_REG, RegValue);
    
    return XST_SUCCESS;
}

int HVTC_GetTimingSettings(TimingController *TimeCtrlInstPtr)
{
    if(NULL == TimeCtrlInstPtr) {
        return XST_FAILURE;
    }

    u32 RegValue = HVTC_GetReg(VTC_GEN_ACTIVE_SIZE_F0_REG);
    
    TimeCtrlInstPtr->ActiveHsize = (u16)RegValue;
    TimeCtrlInstPtr->ActiveVsize = (u16)(RegValue >> 16);
    
    return XST_SUCCESS;
}

void HVTC_SetReg(u32 Offset, u32 Value)
{
    Xil_Out32(VTC_BASE_ADDR + Offset, Value);
    return;
}

u32 HVTC_GetReg(u32 Offset)
{
    return Xil_In32(VTC_BASE_ADDR + Offset);
}