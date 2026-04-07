#include "HHDMI.h"
#include "HVTC.h"
#include "HINTC.h"
#include "HIIC.h"
#include "xil_printf.h"

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

	Status = HHDMI_Init(&HdmiInst, &IicBusInstHdmi, HDMI_MAIN_ADDR);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = HVTC_Init();
	if (Status != XST_SUCCESS) {
		return Status;
	}
	
	xil_printf("Finished initializing\r\n");
	
	bool VideoActive = false;
	while(1)
	{
		if(HDMI_INTERRUPT_VECTOR != 0)
		{
			HHDMI_ConnectionEvent(&HdmiInst);
		}

		if((HdmiInst.DisplayPresent == true) && (VideoActive == false))
		{
			
		}
	}
    
  return 0;
}