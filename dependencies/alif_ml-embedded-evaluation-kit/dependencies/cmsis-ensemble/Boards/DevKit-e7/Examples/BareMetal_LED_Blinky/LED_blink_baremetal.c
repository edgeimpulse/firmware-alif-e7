/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     LED_blink_baremetal.c
 * @author   Girish BN
 * @email    girish.bn@alifsemi.com
 * @version  V1.0.0
 * @date     23-April-2021
 * @brief    DEMO application for LED blink.
 *           - Toggle LED ds1 and ds2 alternately
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "Driver_GPIO.h"
#include <stdio.h>
#include <RTE_Components.h>
#include CMSIS_device_header

/* Comment this if output is to be observed on debugger console via semihosting */
#define DISABLE_PRINTF

#ifdef DISABLE_PRINTF
    #define printf(fmt, ...) (0)
          /* Also Disable Semihosting */
          #if __ARMCC_VERSION >= 6000000
                    __asm(".global __use_no_semihosting");
          #elif __ARMCC_VERSION >= 5000000
                    #pragma import(__use_no_semihosting)
          #else
                    #error Unsupported compiler
          #endif

          void _sys_exit(int return_code) {
                    while (1)
                              ;
          }
#endif

#define GPIO1_PIN14                     (14) /*< ds1 led connected to this gpio pin >*/
#define GPIO1_PIN15                     (15) /*< ds2 led connected to this gpio pin >*/
#define GPIO1                           1 /* Use LEDS in the GPIO1 port */
uint32_t volatile ms_ticks=0;

void SysTick_Handler (void) {
  ms_ticks++;
}
void delay(uint32_t nticks)
{
      uint32_t c_ticks;

      c_ticks = ms_ticks;
      while ((ms_ticks - c_ticks) < nticks) ;
}


void led_blink_app (void)
{
  /*
   * gpio1 pin14 is connected active high led ds1.
   * gpio1 pin15 is connected active high led ds2.
   *
   * This demo application is about.
   *   - Keeping ds1 led state 'on' and ds2 led state 'off' alternately
   *   - This cycle will keep repeating infinite times.
   */

    extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO1);
    ARM_DRIVER_GPIO *ptrDrv = &ARM_Driver_GPIO_(GPIO1);
    uint8_t led_ds1 = GPIO1_PIN14;
    uint8_t led_ds2 = GPIO1_PIN15;
    uint32_t ret1 = 0;
    uint32_t ret2 = 0;

    printf("led blink demo application started\n\n");

    ret1 = ptrDrv->Initialize(led_ds1, NULL);
    ret2 = ptrDrv->Initialize(led_ds2, NULL);

    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to initialize\n");
        return;
    }

    ret1 = ptrDrv->PowerControl(led_ds1, ARM_POWER_FULL);
    ret2 = ptrDrv->PowerControl(led_ds2, ARM_POWER_FULL);

    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to powered full\n");
        goto error_uninitialize;
    }

    ret1 = ptrDrv->SetDirection(led_ds1, GPIO_PIN_DIRECTION_OUTPUT);
    ret2 = ptrDrv->SetDirection(led_ds2, GPIO_PIN_DIRECTION_OUTPUT);

    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to configure\n");
        goto error_power_off;
    }

    ret1 = ptrDrv->SetValue(led_ds1, GPIO_PIN_OUTPUT_STATE_HIGH);
    ret2 = ptrDrv->SetValue(led_ds2, GPIO_PIN_OUTPUT_STATE_LOW);

    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to configure\n");
        goto error_power_off;
    }

    while (1)
    {
        ret1 = ptrDrv->SetValue(led_ds1, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        ret2 = ptrDrv->SetValue(led_ds2, GPIO_PIN_OUTPUT_STATE_TOGGLE);

        if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }

        /* Delay for .5 sec */
        delay(500);

    }

error_power_off:

    ret1 = ptrDrv->PowerControl(led_ds1, ARM_POWER_OFF);
    ret2 = ptrDrv->PowerControl(led_ds2, ARM_POWER_OFF);

	if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to power off \n");
    } else {
        printf("LEDs power off \n");
    }

error_uninitialize:

    ret1 = ptrDrv->Uninitialize (led_ds1);
    ret2 = ptrDrv->Uninitialize (led_ds2);

    if ((ret1 != ARM_DRIVER_OK) || (ret2 != ARM_DRIVER_OK)) {
        printf("Failed to Un-initialize \n");
    } else {
        printf("Un-initialized \n");
    }
}

/* Define main entry point.  */
int main (void)
{
    /* Configure Systick for each millisec */
    SysTick_Config(SystemCoreClock/1000);

	led_blink_app();
	return 0;
}
