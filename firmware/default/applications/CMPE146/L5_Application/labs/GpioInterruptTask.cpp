#include "GpioInterruptTask.hpp"

SemaphoreHandle_t Sem = xSemaphoreCreateBinary();

///////////////////////////////////////////////////////////////////////////////////////////////////

GpioInterruptTask::GpioInterruptTask(uint8_t priority) : scheduler_task("Interrupt_Lab", 2048, priority)
{
	pins_init();
	puts("GPIO ports initialized.");

	EINT3_init(7);
	puts("Interrupt from P2.7 initialized.");
}

void GpioInterruptTask::pins_init()
{
	LPC_GPIO2->FIODIR |=  (1 << 7);
}

// void GpioInterruptTask::EINT2_init(int pin)
// {
// 	LPC_SC->EXTINT 			= 	(1 << 2);
// 	LPC_SC->EXTMODE 		= 	(1 << 2);
// 	LPC_SC->EXTPOLAR		= 	(1 << 2);

// 	LPC_GPIOINT->IO2IntEnR 	|= 	0xFFFFFFFF;
// 	LPC_GPIOINT->IO2IntEnF 	 =  0;
// 	isr_register(EINT2_IRQn, &EINT2_myHandler);
// 	NVIC_EnableIRQ(EINT2_IRQn);
// }

void GpioInterruptTask::EINT3_init(int pin)
{
	NVIC_DisableIRQ(EINT3_IRQn);

	LPC_SC->EXTMODE 		= 	(1 << 3);
//	LPC_SC->EXTPOLAR		= 	(1 << 3);
	LPC_SC->EXTINT 			= 	(1 << 3);

	LPC_GPIOINT->IO2IntEnR 	= 	(1 << pin);
	LPC_GPIOINT->IO2IntEnF 	=  0;

	NVIC_EnableIRQ(EINT3_IRQn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void button_press_isr()
{
	Led0::getInstance().SetLed();
	Led1::getInstance().SetLed();
	Led2::getInstance().SetLed();
	Led3::getInstance().SetLed();
	LPC_GPIO2->FIOCLR |= (1 << 7);
	long yield = 0;
	xSemaphoreGiveFromISR(Sem, &yield);
	portYIELD_FROM_ISR(yield);
}

extern "C" 
{
	void EINT3_IRQHandler()
	{
		button_press_isr();

		// LPC_SC->EXTINT 		   = (1 << 3);
		LPC_GPIOINT->IO2IntClr = 0xFFFFFFFF;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool GpioInterruptTask::run(void *p)
{
	if ( Button0::getInstance().IsPressed() ) {
		puts("SW0 pressed");
		LPC_GPIO2->FIOSET |= (1 << 7);
	}

	if ( Button1::getInstance().IsPressed() ) {
		Led0::getInstance().ClearLed();
		Led1::getInstance().ClearLed();
		Led2::getInstance().ClearLed();
		Led3::getInstance().ClearLed();
	}

	if ( Button2::getInstance().IsPressed() ) {
		Led0::getInstance().SetLed();
		Led1::getInstance().SetLed();
		Led2::getInstance().SetLed();
		Led3::getInstance().SetLed();
	}

	vTaskDelay(100);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool sw0_callback::run(void *p)
{
	if ( xSemaphoreTake(Sem, portMAX_DELAY) ) {
		printf("Taking Semaphore...\n");
	}

	return true;
}

bool sw1_callback::run(void *p)
{
	if ( xSemaphoreTake(Sem, portMAX_DELAY) && (LPC_GPIO2->FIOPIN & (1 << 7)) ) {
		LPC_GPIO1->FIOSET |= (1 << 8);
		LPC_GPIO2->FIOCLR |= (1 << 7);
		puts("---------> SW1 took semaphore.");
	}
	else {
		xSemaphoreGive(Sem);
		taskYIELD();
	}

	return true;
}