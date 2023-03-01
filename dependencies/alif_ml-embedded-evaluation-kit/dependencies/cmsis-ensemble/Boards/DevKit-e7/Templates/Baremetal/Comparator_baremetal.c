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
 * @file     Comparator_baremetal.c
 * @author   Nisarga A M
 * @email    nisarga.am@alifsemi.com
 * @version  V1.0.0
 * @date     20-June-2022
 * @brief    Baremetal code for analog Comparator.
 *              - CMP0 instance is used - in RTE_Device.h we set the input muxes
 *              - Input A is set by RTE_CMP0_SEL_POSITIVE
 *              - Input A is set to RTE_CMP0_POSITIVE_PIN_PO_00 (analog pin P0_0)
 *              - Input B is set by RTE_CMP0_SEL_NEGATIVE
 *              - Input B is set to RTE_CMP_NEGATIVE_PO_16 (analog pin P0_16)
 *              Hardware setup (2 wires needed):
 *              - Use a wire to connect P0_0 to J410 pin 1 (it will be labeled)
 *              - Use a wire to connect P0_16 to J408 pin 32 (digital pin P1_14)
 *              - This test expects the LED Blinky application is running and
 *                the pin P1_14 should be toggling every 1 second
 *              - When the comparator input changes from HIGH to LOW or from LOW
 *                to HIGH, interrupt will be generated
 *              - This test sets the ADC_VREF_BUF output to 1.5V on J410 pin 1
 ******************************************************************************/

/* System Includes */
#include "Driver_GPIO.h"
#include <stdio.h>
#include "system_utils.h"

/* include for Comparator Driver */
#include"Driver_Comparator.h"

/* For Release build disable printf and semihosting */
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
            while (1);
    }
#endif

/* LED configurations */
#define GPIO1_PIN14                     (14) /*< ds1 led connected to this gpio pin >*/
#define GPIO1                           1    /* Use LEDS in the GPIO1 port */
#define INIT                            1    /* GPIO initialization */
#define TOGGLE                          0    /* LED toggle */

extern  ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO1);
ARM_DRIVER_GPIO *ledDrv = &ARM_Driver_GPIO_(GPIO1);

extern ARM_DRIVER_CMP Driver_CMP0;
static ARM_DRIVER_CMP *CMPdrv = &Driver_CMP0;

#define CMP_CALLBACK_EVENT_SUCCESS  1

volatile int32_t call_back_event = 0;
volatile uint32_t call_back_counter = 0;

/**
 * @fn        led_config(int arg)
 * @brief     - Initialize the led _ds1
 *            - Enable the power for led_ds1
 *            - Set direction for led_ds1
 *            - Set value for led_ds1
 *            - set led_ds1 for toggle
 * @param[in]   arg: - INIT will initialize, power
 *                     and set value for led_ds1.
 *                   - TOGGLE will toggle the led_ds1.
 * return     None
 */
void led_config(int arg)
{
    uint8_t led_ds1 = GPIO1_PIN14;
    uint32_t ret1 = 0;

    if(arg)
    {
        /* Initialize the led_ds1 */
        ret1 = ledDrv->Initialize(led_ds1, NULL);
        if (ret1 != ARM_DRIVER_OK) {
            printf("ERROR: Failed to initialize\n");
            return;
        }

        /* Enable the power for led_ds1 */
        ret1 = ledDrv->PowerControl(led_ds1, ARM_POWER_FULL);
        if (ret1 != ARM_DRIVER_OK) {
            printf("ERROR: Failed to powered full\n");
            goto error_uninitialize_LED;
        }

       ret1 = ledDrv->SetDirection(led_ds1, GPIO_PIN_DIRECTION_OUTPUT);
       if (ret1 != ARM_DRIVER_OK) {
            printf("ERROR: Failed to configure\n");
            goto error_power_off;
        }

        ret1 = ledDrv->SetValue(led_ds1, GPIO_PIN_OUTPUT_STATE_HIGH);
        if (ret1 != ARM_DRIVER_OK) {
            printf("ERROR: Failed to configure\n");
            goto error_power_off;
        }
    }
    else
    {
        ret1 = ledDrv->SetValue(led_ds1, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        if (ret1 != ARM_DRIVER_OK) {
            printf("ERROR: Failed to toggle LEDs\n");
            goto error_power_off;
        }
    }
    return;

error_power_off:
    /* Power-off the LED */
    ret1 = ledDrv->PowerControl(led_ds1, ARM_POWER_OFF);
    if (ret1 != ARM_DRIVER_OK)  {
        printf("ERROR: Failed to power off \n");
    }

error_uninitialize_LED:
    /* Uninitialize the led)ds1 */
    ret1 = ledDrv->Uninitialize (led_ds1);
    if (ret1 != ARM_DRIVER_OK)  {
        printf("Failed to Un-initialize \n");
    }
}

/**
 * @fn       void CMP_filter_callback(uint32_t event)
 * @brief    - This code expects the LED Blinky application to be running
 *             and the pin P1_14 should toggle every 1 second.
 *           - The comparator compares the voltage of P1_14 which is connected
 *             to negative comparator input which is compared to the 1.5V ref on
 *             J410 pin 1 (or 1.2V ref on J428).
 *           - When the comparator input changes from HIGH to LOW or from LOW to HIGH,
 *             interrupt will be generated and it will set the call back event
 *           - According to the interrupt generation the call_back_counter will be incremented.
 * @return   None
 */
static void CMP_filter_callback(uint32_t event)
{
    if (event & ARM_CMP_FILTER_EVENT_OCCURRED)
    {
        /* Received Comparator filter event */
        call_back_event = CMP_CALLBACK_EVENT_SUCCESS;
    }
    call_back_counter++;
}

void CMP_demo_entry()
{
    uint32_t ret = 0;
    uint32_t loop_count = 10;
    ARM_DRIVER_VERSION version;
    ARM_COMPARATOR_CAPABILITIES capabilities;

    printf("\r\n >>> Comparator demo threadX starting up!!! <<< \r\n");

    version = CMPdrv->GetVersion();
    printf("\r\n Comparator version api:%X driver:%X...\r\n",version.api, version.drv);

    /* Initialize the Comparator driver */
    ret = CMPdrv->Initialize(CMP_filter_callback);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator init failed\n");
        return;
    }

    /* Initialize the configurations for led_ds1 */
    led_config(INIT);

    /* Enable the power for Comparator */
    ret = CMPdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Power up failed\n");
        goto error_uninitialize;
    }

    /* windowing capability to define when to look at comparator input */
    ret = CMPdrv->Control(ARM_CMP_WINDOW_CONTROL, 0);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Window control failed\n");
        goto error_poweroff;
    }

    /* Filter function for analog comparator*/
    ret = CMPdrv->Control(ARM_CMP_FILTER_CONTROL, 5);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Filter control failed\n");
        goto error_poweroff;
    }

    /* Prescaler function for the comparator */
    ret = CMPdrv->Control(ARM_CMP_PRESCALER_CONTROL, 8);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Prescaler control failed\n");
        goto error_poweroff;
    }

    /* Start the Comparator module */
    ret = CMPdrv->Start();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Start failed\n");
        goto error_poweroff;
    }

    while (loop_count--)
    {
        /* Toggle the led_ds1 */
        led_config(TOGGLE);

        /* wait for the call back event */
        while(call_back_event == 0);
        call_back_event = 0;

        PMU_delay_loop_us(100000);

        printf("\n >>> Filter event occurred <<< \n");
    }

    /* Stop the Comparator module */
    ret = CMPdrv->Stop();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Stop failed\n");
        goto error_poweroff;
    }

    printf("\n Comparator Filter event completed and the call_back_counter value is %d\n",call_back_counter );

error_poweroff:
    /* Power off Comparator peripheral */
    ret = CMPdrv->PowerControl(ARM_POWER_OFF);
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: Comparator Power OFF failed.\r\n");
    }

error_uninitialize:
    /* UnInitialize comparator driver */
    ret = CMPdrv->Uninitialize();
    if(ret != ARM_DRIVER_OK){
        printf("\r\n Error: comparator Un-initialize failed.\r\n");
    }

}
/* Define main entry point.  */
int main()
{
    /* Enter the demo Application.  */
    CMP_demo_entry();

    return 0;
}

