/*
 * Copyright 2024 Olympus Veran Technologies.
 * Do not reproduce without written permission.
 * All rights reserved.
 *
 * Created by Jordan Deitsch.
 */

#ifndef HINTC_H
#define HINTC_H

// Include Xilinx driver files
#include "xparameters.h"
#include "xil_exception.h"
#include "xintc.h"

#include "HIIC.h"


#define INTC_DEVICE_ADDR	(XPAR_MICROBLAZE_0_AXI_INTC_BASEADDR)

extern XIntc IntrCtrInst;
extern u32 StubInterruptCounter;


int HINTC_InitializeInterruptSystem(XIntc 	*IntcInstPtr,	u32	IntrCtrAddr);
void HINTC_StubIntrHandler(void *CallBackRef);

int HINTC_SetupPeripheralInterrupt(XIntc 	*IntcInstPtr,
								IicBus 		**IicBusList, 	u16 NumIicDrivers);

int HINTC_SetupNewInterrupt(XIntc *IntcInstPtr,
							u8 InterId,
							XInterruptHandler IntrHandler,
					  	  	void *CallBackRef);

#endif
