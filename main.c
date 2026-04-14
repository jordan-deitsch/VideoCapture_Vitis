#include "HHDMI.h"
#include "HINTC.h"
#include "HIIC.h"
#include "HVTC.h"
#include "HVTPG.h"
#include "xil_printf.h"
#include <stdbool.h>

int main()
{
    int Status = 0;
    
    Status = HINTC_InitializeInterruptSystem(&IntrCtrInst, INTC_DEVICE_ADDR);
	if (Status != XST_SUCCESS) {
		return Status;
	}

    Status = HIIC_Setup();
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HINTC_SetupPeripheralInterrupt(&IntrCtrInst, IicBusList, NUM_IIC_DRIVERS);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HVTC_Init(&TimingControllerInst);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HVTPG_Init(&PatternGenInst);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HHDMI_Init(&HdmiInst, &IicBusInstHdmi, HDMI_MAIN_ADDR);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	
	xil_printf("Finished initializing\r\n");

	bool GenVideo = false;
	
	while(1)
	{
		if(HDMI_INTERRUPT_VECTOR != 0)
		{
			HHDMI_ConnectionEvent(&HdmiInst);

			if(HdmiInst.DisplayPresent)
			{
				HVTC_UpdateRegisters(&TimingControllerInst);
				HVTC_EnableController(&TimingControllerInst, true);
				HVTPG_ConfigureFrame(&PatternGenInst, e_HorizontalRamp);
				GenVideo = true;
			}
			else
			{
				HVTC_EnableController(&TimingControllerInst, false);
				GenVideo = false;
			}
		}

		if(GenVideo)
		{
			HVTPG_GenerateFrame(&PatternGenInst);
		}
	}
    
  return 0;
}