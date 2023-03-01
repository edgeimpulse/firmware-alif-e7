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
 * @file     QEC_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     30-October-2022
 * @brief    Baremetal demo application for QEC0 channel.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#include <stdio.h>
#include "Driver_UTIMER.h"
#include "Driver_GPIO.h"
#include "Driver_PINMUX_AND_PINPAD.h"

/* GPIO related definitions */
#define GPIO1                          1
#define GPIO1_PIN0                     0
#define GPIO1_PIN1                     1
#define GPIO1_PIN2                     2

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

/* GPIO1 Driver instance */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO1);
ARM_DRIVER_GPIO *ptrDrv = &ARM_Driver_GPIO_(GPIO1);

volatile uint32_t cb_qec_status = 0;

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

/**
 * @function    int pinmux_config()
 * @brief       QEC h/w pin init using pinmux driver
 * @note        none
 * @param       none
 * @retval      execution status
 */
static int32_t pinmux_config()
{
    int32_t ret;

    ret = PINMUX_Config (PORT_NUMBER_2, PIN_NUMBER_28, PINMUX_ALTERNATE_FUNCTION_5);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = PINMUX_Config (PORT_NUMBER_2, PIN_NUMBER_29, PINMUX_ALTERNATE_FUNCTION_5);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = PINMUX_Config (PORT_NUMBER_2, PIN_NUMBER_30, PINMUX_ALTERNATE_FUNCTION_4);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    return ARM_DRIVER_OK;
}

/**
 * @function    int gpio_init()
 * @brief       GPIO initialization using gpio driver
 * @note        none
 * @param       none
 * @retval      execution status
 */
static int32_t gpio_init()
{
    int32_t ret = ARM_DRIVER_OK;

    /* init P1_0 as GPIO */
    ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_0, PINMUX_ALTERNATE_FUNCTION_0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }
    ret = PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_0, (PAD_FUNCTION_READ_ENABLE|PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = ptrDrv->Initialize(GPIO1_PIN0, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to initialize GPIO1_PIN0 as GPIO\n");
        return -1;
    }

    ret = ptrDrv->PowerControl(GPIO1_PIN0, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to Powered the GPIO1_PIN0\n");
        return -1;
    }

    ret = ptrDrv->SetDirection(GPIO1_PIN0, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set direction for GPIO1_PIN0\n");
        return -1;
    }

    ret = ptrDrv->SetValue(GPIO1_PIN0, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN0\n");
        return -1;
    }

    /* init P1_1 as GPIO */
    ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_1, PINMUX_ALTERNATE_FUNCTION_0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }
    ret = PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_1, (PAD_FUNCTION_READ_ENABLE|PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = ptrDrv->Initialize(GPIO1_PIN1, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to initialize GPIO1_PIN1 as GPIO\n");
        return -1;
    }

    ret = ptrDrv->PowerControl(GPIO1_PIN1, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to Powered the GPIO1_PIN1\n");
        return -1;
    }

    ret = ptrDrv->SetDirection(GPIO1_PIN1, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set direction for GPIO1_PIN1\n");
        return -1;
    }

    ret = ptrDrv->SetValue(GPIO1_PIN1, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN1\n");
        return -1;
    }

    /* init P1_2 as GPIO */
    ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_2, PINMUX_ALTERNATE_FUNCTION_0);
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }
    ret = PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_2, (PAD_FUNCTION_READ_ENABLE|PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
    if(ret != ARM_DRIVER_OK) {
        printf("\r\n Error in PINMUX.\r\n");
        return -1;
    }

    ret = ptrDrv->Initialize(GPIO1_PIN2, NULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to initialize GPIO1_PIN2 as GPIO\n");
        return -1;
    }

    ret = ptrDrv->PowerControl(GPIO1_PIN2, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to Powered the GPIO1_PIN2\n");
        return -1;
    }

    ret = ptrDrv->SetDirection(GPIO1_PIN2, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set direction for GPIO1_PIN2\n");
        return -1;
    }

    ret = ptrDrv->SetValue(GPIO1_PIN2, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set value for GPIO1_PIN2\n");
        return -1;
    }

    return ARM_DRIVER_OK;
}

/**
 * @function    void qec_cb_func(event)
 * @brief       QEC callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void qec_cb_func(uint32_t event)
{
    if (event == ARM_QEC_COMPARE_A_EVENT) {
        cb_qec_status = 1;
    }
}

/**
 * @function    void qec0_app(void)
 * @brief       QEC0 demo application
 * @note        none
 * @param       none
 * @retval      none
 */
static void qec0_app()
{
    int32_t ret;
    uint32_t buf, buf1;
    uint8_t channel = 12;
    uint32_t count_array[ARM_QEC_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_CAPTURING,
        .direction = ARM_UTIMER_COUNT_DIRECTION_TRIANGLE,
        .count_array = count_array,
        .count_number = ARM_QEC_COUNT_NUMBER
    };

    UTIMER_GET_OPERATION_CONFIG get_cntr = {
        .operation_type = ARM_UTIMER_GET_COUNT_OF_CURRENT_RUNNING_TIMER,
        .count = &buf
    };

    UTIMER_GET_OPERATION_CONFIG get_capt = {
        .operation_type = ARM_UTIMER_GET_COUNT_OF_DRIVE_A_CAPTURE_VALUE,
        .count = &buf1
    };

    /*
     * utimer channel 12 is configured as QEC0.
     * QEC pins are connected to motors/actuators to measure speed and direction.
     * For testing purpose, QEC inputs are connected to gpio's. And GPIO's are toggled accordingly.
     *
     * H/W connection : short P1_0 and P2_28, short P1_1 and P2_29, short P1_2 and P2_30.
     **/

    printf("*** QEC demo application started ***\n");

    count_array[0] = 0;        /*< initial count value >*/
    count_array[1] = 100;      /*< overflow count value >*/

    ret = pinmux_config();
    if (ret != ARM_DRIVER_OK) {
        printf("pinmux failed\n");
        return;
    }

    ret = gpio_init();
    if (ret != ARM_DRIVER_OK) {
        printf("gpio init failed\n");
        return;
    }

    ret = ptrUTIMER->Initialize (channel, qec_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_qec_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_qec_poweroff;
    }

    /* Config Trigger for up count */
    ret = ptrUTIMER->Configure_Trigger (channel, ARM_UTIMER_TRIGGER_FOR_UPCOUNT, \
        (ARM_UTIMER_DRIVE_A_RISING_B_0|ARM_UTIMER_DRIVE_A_FALLING_B_1|ARM_UTIMER_DRIVE_B_FALLING_A_0|ARM_UTIMER_DRIVE_B_RISING_A_1));
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("utimer channel %d triggered for up count \n", channel);
    }

    /* Config Trigger for down count */
    ret = ptrUTIMER->Configure_Trigger (channel, ARM_UTIMER_TRIGGER_FOR_DOWNCOUNT, \
        (ARM_UTIMER_DRIVE_A_FALLING_B_0|ARM_UTIMER_DRIVE_A_RISING_B_1|ARM_UTIMER_DRIVE_B_RISING_A_0|ARM_UTIMER_DRIVE_B_FALLING_A_1));
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("utimer channel %d triggered for down count \n", channel);
    }

    /* Config Trigger for capture counter value */
    ret = ptrUTIMER->Configure_Trigger (channel, ARM_UTIMER_TRIGGER_FOR_CAPTURE_A, \
            (ARM_UTIMER_TRIG0_RISING|ARM_UTIMER_TRIG0_FALLING));
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("utimer channel %d triggered for capture counter value \n", channel);
    }

    /* Toggling gpio's connected to x/y for 10 times to increment cnt value, so cnt value will be 20 */
    for(int i=0;i<10;i++)
    {
        ret = ptrDrv->SetValue(GPIO1_PIN0, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        ret = ptrDrv->SetValue(GPIO1_PIN1, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set data \n");
        }
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_cntr);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d reading counter value failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("current counter value : %d\n", buf);
    }

    /* Toggling gpio's connected to x/y for 5 times to decrement cnt value, so cnt value is reduced by 10 */
    for(int i=0;i<5;i++)
    {
        ret = ptrDrv->SetValue(GPIO1_PIN1, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        ret = ptrDrv->SetValue(GPIO1_PIN0, GPIO_PIN_OUTPUT_STATE_TOGGLE);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set data \n");
        }
    }

    /* Making z event as high to capture count value */
    ret = ptrDrv->SetValue(GPIO1_PIN2, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK) {
        printf("ERROR: Failed to set data \n");
    }

    /* Check for an interrupt */
    if (cb_qec_status) {
        printf("Capture A interrupt occurred\n");
    } else {
        printf("Capture A interrupt not occurred\n");
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_capt);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_qec_poweroff;
    } else {
        printf("capture A value : %d\n", buf1);
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_qec_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_qec_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: QEC completed *** \r\n\n");
}

int main()
{
    qec0_app();
}
