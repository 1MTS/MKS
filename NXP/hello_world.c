/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "peripherals.h"
#include "fsl_power.h"
#include "fsl_powerquad.h"
#include "math.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
	#define TABLE_LENGHT 256
	#define PI 3.14
	#define USE_POWERQUAD 1

	char ch;
    float Sample_f;
    uint32_t index=0;
    float SinTable_f[TABLE_LENGHT];


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Generate_sin_table_float(float *table, uint32_t length,float amplitude, float frequency)
{
    	uint32_t index;
    	float theta, sample_time, result;
    	sample_time=1.0f/(frequency * (float)length);
    	for(index=0;index < length;index++)
    	{
    		theta= 2.0f * PI * frequency * sample_time * (float)index;
#if USE_POWERQUAD == 1
    		PQ_SinF32(&theta, &result);
    		table[index]=amplitude * result;
#else
    		table[index]=amplitude * sin(theta);
#endif
    	}
}

/*********************************************************************
* IRQ
*********************************************************************/
void TimerIRQ(uint32_t flags)
{
	Sample_f=SinTable_f[index];
	index++;
	if(index==TABLE_LENGHT)index=0;
}

uint32_t strcmp_safe(const char *s1, const char *s2, const uint16_t size)
{
	uint16_t counter_t = 0;
	uint16_t counter_f = 0;

	for (uint16_t i = 0; i < size ;i++){
		if (s1[i] == s2[i]) {
			counter_t = counter_t + 1;
		}
		else {
			counter_f = counter_f + 1;
		}
	}

	if (counter_t == size){
		return 0;
	} else {
		return 1;
	}

}
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{


    /* Init board hardware. */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();
    BOARD_InitPeripherals();
    PQ_Init(POWERQUAD);

    uint32_t DWT1, DWT2;
    char password_stored[20] = "1234";
    char input [20];
    uint16_t status;


#if !defined(DONT_ENABLE_FLASH_PREFETCH)
    /* enable flash prefetch for better performance */
    SYSCON->FMCCR |= SYSCON_FMCCR_PREFEN_MASK;
#endif

    PRINTF("hello world.\r\n");
    CTIMER_StartTimer(CTIMER1_PERIPHERAL);

    Generate_sin_table_float(&SinTable_f[0],TABLE_LENGHT,1.0f, 1.0f);

    while (1)
    {
    	PRINTF("\r\nEnter password: ");
    	SCANF("%s", input);

        DWT1 = DWT ->CYCCNT;
        status = strcmp_safe(input, password_stored, strlen(password_stored));
        DWT2 = DWT->CYCCNT;
        PRINTF("\r\nCycle in function: %d", DWT2 - DWT1);

        PRINTF("\r\n input: %s", input);
        if(status == 0) {
        	PRINTF("\r\n input correct");
        }
        else {
        	PRINTF("\r\n input invalid");
        }


        /*
        ch = GETCHAR();
        PUTCHAR(ch);
		*/

    }
}
