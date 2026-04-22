/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HVTPG_H
#define HVTPG_H

#include "HVTC.h"
#include "xparameters.h"
#include "xil_types.h"
#include <stdbool.h>


#define VTPG_BASE_ADDR (XPAR_V_HDMI_TPG_BASEADDR)

enum PatternId{
    e_VideoPassThrough,
    e_HorizontalRamp,
    e_VerticalRamp,
    e_TemporalRamp,
    e_SolidRed,
    e_SolidGreen,
    e_SolidBlue,
    e_SolidBlack,
    e_SolidWhile,
    e_ColorBars,
    e_ZonePlate,
    e_TartanColorBars,
    e_CrossHatch,
    e_ColorSweep,
    e_ComboVertHorizRamp,
    e_Checkerboard,
    e_PseudoRandom,
    e_NumPatterns
};

typedef struct PatternGenerator
{
	u16 ActiveVsize;
    u16 ActiveHsize;
} PatternGenerator;

/************************** Peripheral Device Declarations *****************************/
extern PatternGenerator PatternGenInst;

/************************** Function Declarations *****************************/
int HVTPG_Init(PatternGenerator *PattGenInstPtr);
int HVTPG_ConfigureFrame(PatternGenerator *PattGenInstPtr, int Pattern);
int HVTPG_GenerateContinuous(PatternGenerator *PattGenInstPtr);
int HVTPG_MovingTarget(PatternGenerator *PattGenInstPtr, int Pattern);

#endif