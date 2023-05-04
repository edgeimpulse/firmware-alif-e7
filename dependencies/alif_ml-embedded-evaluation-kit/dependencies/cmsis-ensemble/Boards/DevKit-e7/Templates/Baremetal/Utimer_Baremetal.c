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
 * @file     Utimer_Baremetal.c
 * @author   Manoj A Murudi
 * @email    manoj.murudi@alifsemi.com
 * @version  V1.0.0
 * @date     19-August-2022
 * @brief    Baremetal demo application for UTIMER.
 *           - Configuring the UTIMER Channel 0 for 500ms basic mode.
 *           - Configuring the UTIMER Channel 1 for 500ms, 1000ms, 1500ms buffering mode.
 *           - Configuring the UTIMER Channel 3 for counter start triggering mode.
 *           - Configuring the UTIMER Channel 4 for driver A, double buffering capture mode.
 *           - Configuring the UTIMER Channel 5 for driver A, double buffering compare mode.
 *           - Configuring the UTIMER Channel 6 for DT mode.
 * @bug      None.
 * @Note     None
 ******************************************************************************/
#include <stdio.h>
#include "Driver_UTIMER.h"
#include "Driver_GPIO.h"
#include "Driver_PINMUX_AND_PINPAD.h"

#include "RTE_Components.h"
#include CMSIS_device_header

/* GPIO related definitions */
#define GPIO1                          1
#define GPIO1_PIN0                     0
#define GPIO1_PIN1                     1
#define GPIO1_PIN2                     2
#define GPIO1_PIN3                     3

/* UTIMER0 Driver instance */
extern ARM_DRIVER_UTIMER DRIVER_UTIMER0;
ARM_DRIVER_UTIMER *ptrUTIMER = &DRIVER_UTIMER0;

/* GPIO1 Driver instance */
extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(GPIO1);
ARM_DRIVER_GPIO *ptrDrv = &ARM_Driver_GPIO_(GPIO1);

volatile uint32_t cb_basic_status = 0;
volatile uint32_t cb_buffer_status = 0;
volatile uint32_t cb_trigger_status = 0;
volatile uint32_t cb_capture_status = 0;
volatile uint32_t cb_compare_a_status = 0;
volatile uint32_t cb_compare_a_buf1_status = 0;
volatile uint32_t cb_compare_a_buf2_status = 0;
volatile uint32_t cb_compare_b_status = 0;
volatile uint32_t cb_compare_b_buf1_status = 0;
volatile uint32_t cb_compare_b_buf2_status = 0;
volatile uint32_t cb_dt_status = 0;

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
 * @function    int pinmux_config(channel)
 * @brief       UTIMER hardware pin initialization using pinmux driver
 * @note        none
 * @param       channel
 * @retval      execution status
 */
static int32_t pinmux_config(UTIMER_MODE mode)
{
    int32_t ret;

    /* Trigger mode is configured on UTIMER channel 3, Configure P1_10 as UT3_A and P1_11 as UT3_B */
    if (mode == ARM_UTIMER_MODE_TRIGGERING) {
        ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_10, PINMUX_ALTERNATE_FUNCTION_4);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }

        ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_11, PINMUX_ALTERNATE_FUNCTION_4);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
    }

    /* Capture mode is configured on UTIMER channel 4. Configure P1_12 as UT4_A and P1_13 as UT4_B */
    if (mode == ARM_UTIMER_MODE_CAPTURING) {
        ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_12, PINMUX_ALTERNATE_FUNCTION_5);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
        ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_13, PINMUX_ALTERNATE_FUNCTION_5);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
    }

    /* Compare mode is configured on UTIMER channel 5, Configure P1_14 as utimer driver_A output */
    if (mode == ARM_UTIMER_MODE_COMPARING) {
        ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_14, PINMUX_ALTERNATE_FUNCTION_4);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
        ret = PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_14, PAD_FUNCTION_READ_ENABLE);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINPAD.\r\n");
            return -1;
        }
    }

    /* Dead Time mode is configured on UTIMER channel 6, Configure P3_8 as utimer driver_A output and P3_9 as utimer driver_B output */
    if (mode == ARM_UTIMER_MODE_DEAD_TIME) {
        ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_8, PINMUX_ALTERNATE_FUNCTION_5);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
        ret = PINPAD_Config (PORT_NUMBER_3, PIN_NUMBER_8, PAD_FUNCTION_READ_ENABLE);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINPAD.\r\n");
            return -1;
        }

        ret = PINMUX_Config (PORT_NUMBER_3, PIN_NUMBER_9, PINMUX_ALTERNATE_FUNCTION_6);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
        ret = PINPAD_Config (PORT_NUMBER_3, PIN_NUMBER_9, PAD_FUNCTION_READ_ENABLE);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINPAD.\r\n");
            return -1;
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @function    int gpio_init(mode)
 * @brief       GPIO initialization using gpio driver
 * @note        none
 * @param       mode
 * @retval      execution status
 */
static int32_t gpio_init(UTIMER_MODE mode)
{
    int32_t ret = ARM_DRIVER_OK;

    if(mode == ARM_UTIMER_MODE_TRIGGERING) {
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
    }

    if(mode == ARM_UTIMER_MODE_CAPTURING) {
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

        /* init P1_3 as GPIO */
        ret = PINMUX_Config (PORT_NUMBER_1, PIN_NUMBER_3, PINMUX_ALTERNATE_FUNCTION_0);
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }
        ret = PINPAD_Config (PORT_NUMBER_1, PIN_NUMBER_3, (PAD_FUNCTION_READ_ENABLE|PAD_FUNCTION_OUTPUT_DRIVE_STRENGTH_04_MILI_AMPS));
        if(ret != ARM_DRIVER_OK) {
            printf("\r\n Error in PINMUX.\r\n");
            return -1;
        }

        ret = ptrDrv->Initialize(GPIO1_PIN3, NULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to initialize GPIO1_PIN3 as GPIO\n");
            return -1;
        }

        ret = ptrDrv->PowerControl(GPIO1_PIN3, ARM_POWER_FULL);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to Powered the GPIO1_PIN3\n");
            return -1;
        }

        ret = ptrDrv->SetDirection(GPIO1_PIN3, GPIO_PIN_DIRECTION_OUTPUT);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set direction for GPIO1_PIN3\n");
            return -1;
        }

        ret = ptrDrv->SetValue(GPIO1_PIN3, GPIO_PIN_OUTPUT_STATE_LOW);
        if (ret != ARM_DRIVER_OK) {
            printf("ERROR: Failed to set value for GPIO1_PIN3\n");
            return -1;
        }
    }

    return ARM_DRIVER_OK;
}

/**
 * @function    void utimer_basic_mode_cb_func(event)
 * @brief       utimer basic mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_basic_mode_cb_func (uint32_t event)
{
    if (event & ARM_UTIMER_OVER_FLOW_EVENT) {
        cb_basic_status++;
    }
}

/**
 * @function    void utimer_basic_mode_app(void)
 * @brief       utimer basic mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_basic_mode_app()
{
    int32_t ret;
    uint8_t channel = 0;
    uint32_t count_array[ARM_UTIMER_MODE_BASIC_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_BASIC,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_BASIC_COUNT_NUMBER
    };

    /* utimer channel 0 is configured for utimer basic mode (config counter ptr reg for 500ms) */

    printf("*** utimer demo application for basic mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6)) = 20000000
     *
     * DEC = 20000000
     * HEX = 0xBEBC200
     *
     */
    count_array[0] = 0x00000000;   /*< initial counter value >*/
    count_array[1] = 0xBEBC200;    /*< over flow count value >*/

    ret = ptrUTIMER->Initialize (channel, utimer_basic_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_basic_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_basic_mode_poweroff;
    }

    printf("utimer channel '%d'configured on basic mode for 500 ms\r\n", channel);

    ret = ptrUTIMER->Start (channel);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_basic_mode_poweroff;
    } else {
        printf("utimer channel '%d': timer started\n", channel);
    }

    PMU_delay_loop_us (500000);
    if (cb_basic_status) {
        cb_basic_status = 0;
        printf("utimer channel %d :500ms timer expired \n", channel);
    } else {
        printf("Error :utimer basic mode timeout \n");
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_basic_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_basic_mode_uninstall:

    ret = ptrUTIMER->Uninitialize(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: basic mode completed *** \r\n\n");
}

/**
 * @function    void utimer_buffering_mode_cb_func(event)
 * @brief       utimer buffer mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_buffering_mode_cb_func (uint32_t event)
{
    if (event & ARM_UTIMER_OVER_FLOW_EVENT) {
        cb_buffer_status ++;
    }
}

/**
 * @function    void utimer_buffering_mode_app(void)
 * @brief       utimer buffer mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_buffering_mode_app ()
{
    int32_t ret;
    uint8_t channel = 1;
    uint8_t index;
    uint32_t count_array[ARM_UTIMER_MODE_DOUBLE_BUFFERING_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_BUFFERING,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_DOUBLE_BUFFERING_COUNT_NUMBER
    };

    /* utimer channel 1 is configured for utimer buffer mode (selected double buffering)
     * configuring counter ptr, buf1, buf2 reg's as 500ms, 1 sec, 1.5 sec respectively */

    printf("*** utimer demo application for buffering mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6))
     * DEC = 200000000
     * HEX = 0xBEBC200
     *
     * So count for 1000ms = (1000*(10^-3))/(0.0025*(10^-6))
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 1500ms = (1500*(10^-3))/(0.0025*(10^-6))
     * DEC = 60000000
     * HEX = 0x23C34600
     *
     */

    count_array[0] = 0x00000000;     /*< Initial counter value>*/
    count_array[1] = 0xBEBC200;      /*< Over flow count value for First Iteration>*/
    count_array[2] = 0x17D78400;     /*< Over flow count value for Second Iteration>*/
    count_array[3] = 0x23C34600;     /*< Over flow count value for Third Iteration>*/

    ret = ptrUTIMER->Initialize (channel, utimer_buffering_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_buffering_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_buffering_mode_poweroff;
    }

    printf("channel '%d'configured on buffering mode for 500, 1000, and 1500 ms \r\n", channel);

    ret = ptrUTIMER->Start(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_buffering_mode_poweroff;
    } else {
        printf("utimer channel %d :timer started\n", channel);
    }

    for (index=1; index<=3; index++)
    {
        PMU_delay_loop_us (500000 * index);
        if (cb_buffer_status) {
            cb_buffer_status = 0;
            printf("utimer channel %d: %d ms timer expired\n", channel, (500*index));
        } else {
            printf("Error :Utimer buffer mode timeout \n");
        }
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d: timer stopped\n", channel);
    }

error_buffering_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("uTIMER channel %d failed power off \n", channel);
    }

error_buffering_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: buffering mode completed *** \r\n\n");
}

/**
 * @function    void utimer_trigger_mode_cb_func(event)
 * @brief       utimer trigger mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_trigger_mode_cb_func (uint32_t event)
{
    if (event & ARM_UTIMER_OVER_FLOW_EVENT) {
        cb_trigger_status = 1;
    }
}

/**
 * @function    void utimer_trigger_mode_app(void)
 * @brief       utimer trigger mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_trigger_mode_app()
{
    int32_t ret;
    uint32_t buf = 0;
    uint8_t channel = 3;
    uint32_t count_array[ARM_UTIMER_MODE_TRIGGERING_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_TRIGGERING,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_TRIGGERING_COUNT_NUMBER
    };

    UTIMER_GET_OPERATION_CONFIG get_info = {
        .operation_type = ARM_UTIMER_GET_COUNT_OF_CURRENT_RUNNING_TIMER,
        .count = &buf
    };

    /*
     * utimer channel 3 is configured for utimer trigger mode.
     * chan_event_a_rising_b_0 event from pinmux is used for triggering counter start.
     * H/W connection : short P1_0 and P1_10, short p1_1 and P1_11.
     **/

    printf("*** utimer demo application for trigger mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 500ms = (500*(10^-3))/(0.0025*(10^-6)) = 200000000
     *
     * DEC = 200000000
     * HEX = 0xBEBC200
     */

    count_array[0] = 0;            /*< initial counter value >*/
    count_array[1] = 0xBEBC200;    /*< over flow count value >*/

    ret = pinmux_config(config_info.mode);
    if (ret != ARM_DRIVER_OK) {
        printf("pinmux failed\n");
    }

    ret = gpio_init(config_info.mode);
    if (ret != ARM_DRIVER_OK) {
        printf("gpio init failed\n");
    }

    ret = ptrUTIMER->Initialize (channel, utimer_trigger_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_trigger_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_trigger_mode_poweroff;
    }

    /* Config Trigger for counter start using chan_event_a_rising_b_0 */
    ret = ptrUTIMER->Configure_Trigger (channel, ARM_UTIMER_TRIGGER_FOR_START, ARM_UTIMER_DRIVE_A_RISING_B_0);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_trigger_mode_poweroff;
    } else {
        printf("utimer channel %d triggered for counter start \n", channel);
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_trigger_mode_poweroff;
    } else {
        printf("counter value before triggering : %d\n",buf);
    }

    ret = ptrDrv->SetValue(GPIO1_PIN0, GPIO_PIN_OUTPUT_STATE_HIGH);
    if ((ret != ARM_DRIVER_OK)) {
        printf("ERROR: Failed to configure\n");
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_trigger_mode_poweroff;
    } else {
        printf("counter value just after triggering : %d\n",buf);
    }

    while(1)
    {
        if(cb_trigger_status) {
            cb_trigger_status = 0;
            printf("overflow interrupt is generated\n");
            break;
        }
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped\n", channel);
    }

error_trigger_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_trigger_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: trigger mode completed *** \r\n\n");
}

/**
 * @function    void utimer_capture_mode_cb_funcc(event)
 * @brief       utimer capture mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_capture_mode_cb_func(uint32_t event)
{
    if(event == ARM_UTIMER_CAPTURE_A_EVENT) {
        cb_capture_status++;
    }
    if(event == ARM_UTIMER_CAPTURE_B_EVENT) {
        cb_capture_status++;
    }
}

/**
 * @function    void utimer_capture_mode_app(void)
 * @brief       utimer capture mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_capture_mode_app()
{
    int32_t ret;
    uint8_t channel = 4;
    uint32_t buf = 0;
    uint32_t count_array[ARM_UTIMER_MODE_CAPTURING_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_CAPTURING,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_CAPTURING_COUNT_NUMBER
    };

    UTIMER_GET_OPERATION_CONFIG get_capt_a = {
        .operation_type = ARM_UTIMER_GET_COUNT_OF_DRIVE_A_CAPTURE_VALUE,
        .count = &buf
    };

    UTIMER_GET_OPERATION_CONFIG get_capt_a_buf1 = {
        .operation_type = ARM_UTIMER_GET_COUNT_OF_DRIVE_A_CAPTURE1_VALUE,
        .count = &buf
    };

    UTIMER_GET_OPERATION_CONFIG get_capt_a_buf2 = {
        .operation_type = ARM_UTIMER_GET_COUNT_OF_DRIVE_A_CAPTURE2_VALUE,
        .count = &buf
    };

    /*
     * utimer channel 4 is configured for utimer input capture mode (selected driver A, double buffer).
     * chan_event_a_rising_b_0 event from pinmux is used to trigger input capture counter value.
     * H/W connection : short P1_2 and P1_12, short p1_3 and P1_13.
     */

    printf("*** utimer demo application for capture mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 1 sec = 1/(0.0025*(10^-6)) = 400000000
     *
     * DEC = 400000000
     * HEX = 0x17D78400
     */
    count_array[0] = 0;             /*< initial counter value >*/
    count_array[1] = 0x17D78400;    /*< over flow count value >*/

    /* capture mode pin confg */
    ret = pinmux_config(config_info.mode);
    if (ret != ARM_DRIVER_OK) {
        printf("pinmux failed\n");
    }

    /* GPIO pin confg */
    ret = gpio_init(config_info.mode);
    if (ret != ARM_DRIVER_OK) {
        printf("gpio init failed\n");
    }

    ret = ptrUTIMER->Initialize (channel, utimer_capture_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_capture_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_capture_mode_poweroff;
    }

    /* Trigger for input capture using chan_event_a_rising_b_0 */
    ret = ptrUTIMER->Configure_Trigger (channel, ARM_UTIMER_TRIGGER_FOR_CAPTURE_A, ARM_UTIMER_DRIVE_A_RISING_B_0);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d trigger configuration failed \n", channel);
        goto error_capture_mode_poweroff;
    } else {
        printf("utimer channel %d triggered for input capture \n", channel);
    }

    ret = ptrUTIMER->Start (channel);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_capture_mode_poweroff;
    } else {
        printf("utimer channel '%d': timer started\n", channel);
    }

    for(int index=0; index<3; index++)
    {
        /* Delay of 100 ms */
        PMU_delay_loop_us (100000);
        ret = ptrDrv->SetValue(GPIO1_PIN2, GPIO_PIN_OUTPUT_STATE_HIGH);
        if ((ret != ARM_DRIVER_OK)) {
            printf("ERROR: Failed to configure\n");
        }

        while(1)
        {
            if(cb_capture_status) {
                printf("current counter value is captured\n");
                cb_capture_status = 0;
                ret = ptrDrv->SetValue(GPIO1_PIN2, GPIO_PIN_OUTPUT_STATE_LOW);
                if ((ret != ARM_DRIVER_OK)) {
                    printf("ERROR: Failed to configure\n");
                }
                break;
            }
        }
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d :timer stopped \n", channel);
    }

    /* Read capture registers : capture_a, capture_a_buf1, capture_a_buf2 */
    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_capt_a);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
    } else {
        printf("counter value at capture a : 0x%x \n",buf);
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_capt_a_buf1);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
    } else {
        printf("counter value at capture a buf1 : 0x%x \n",buf);
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_GET_COUNT, &get_capt_a_buf2);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
    } else {
        printf("counter value at capture a buf2 : 0x%x \n",buf);
    }

error_capture_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_capture_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: capture mode completed *** \r\n\n");
}

/**
 * @function    void utimer_compare_mode_cb_func(event)
 * @brief       utimer compare mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_compare_mode_cb_func(uint32_t event)
{
    if (event == ARM_UTIMER_CAPTURE_A_EVENT) {
        cb_compare_a_status++;
    }
    if (event == ARM_UTIMER_CAPTURE_C_EVENT) {
        cb_compare_a_buf1_status++;
    }
    if (event == ARM_UTIMER_CAPTURE_D_EVENT) {
        cb_compare_a_buf2_status++;
    }
    if (event == ARM_UTIMER_CAPTURE_B_EVENT) {
        cb_compare_b_status++;
    }
    if (event == ARM_UTIMER_CAPTURE_E_EVENT) {
        cb_compare_b_buf1_status++;
    }
    if (event == ARM_UTIMER_CAPTURE_F_EVENT) {
        cb_compare_b_buf2_status++;
    }
    if (event == ARM_UTIMER_OVER_FLOW_EVENT) {
        cb_basic_status++;
    }
}

/**
 * @function    void utimer_compare_mode_app(void)
 * @brief       utimer compare mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_compare_mode_app()
{
    int32_t ret;
    uint8_t channel = 5;
    uint32_t count_array[ARM_UTIMER_MODE_COMPARING_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_COMPARING,
        .direction = ARM_UTIMER_COUNT_DIRECTION_UP,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_COMPARING_COUNT_NUMBER
    };

    /*
     * utimer channel 5 is configured for utimer compare mode (driver A, double buffer is enabled).
     * observe driver A output signal from P1_14.
     */
    printf("*** utimer demo application for compare mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 1 sec = 1/(0.0025*(10^-6)) = 400000000
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 250ms = (250*(10^-6)/(0.0025*(10^-6)) = 100000000
     * DEC = 100000000
     * HEX = 0x5F5E100
     *
     * So count for 500ms = (500*(10^-6)/(0.0025*(10^-6)) = 200000000
     * DEC = 200000000
     * HEX = 0xBEBC200
     *
     * So count for 750ms = (750*(10^-6)/(0.0025*(10^-6)) = 300000000
     * DEC = 300000000
     * HEX = 0x11E1A300
     */
    count_array[0] =  0x000000000;       /*< initial counter value >*/
    count_array[1] =  0x17D78400;        /*< over flow count value >*/
    count_array[2] =  0x5F5E100;         /*< compare a/b value>*/
    count_array[3] =  0xBEBC200;         /*< compare a/b buf1 value>*/
    count_array[4] =  0x11E1A300;        /*< compare a/b buf2 value>*/

    /* compare mode pin confg */
    ret = pinmux_config(config_info.mode);
    if (ret != ARM_DRIVER_OK) {
        printf("pinmux failed\n");
    }

    ret = ptrUTIMER->Initialize (channel, utimer_compare_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_compare_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_compare_mode_poweroff;
    }

    ret = ptrUTIMER->Start(channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_compare_mode_poweroff;
    } else {
        printf("utimer channel %d :timer started\n", channel);
    }

    for (int index=0; index<=11; index++)
    {
        while(1)
        {
            if (cb_compare_a_status) {
                cb_compare_a_status = 0;
                printf("compare_a reg value is matched to counter value\n");
                break;
            }
            if (cb_compare_a_buf1_status) {
                cb_compare_a_buf1_status = 0;
                printf("compare_a_buf1 reg value is matched to counter value\n");
                break;
            }
            if (cb_compare_a_buf2_status) {
                cb_compare_a_buf2_status =0;
                printf("compare_a_buf2 reg value is matched to counter value\n");
                break;
            }
            if (cb_compare_b_status) {
                cb_compare_b_status = 0;
                printf("compare_b reg value is matched to counter value\n");
                break;
            }
            if (cb_compare_b_buf1_status) {
                cb_compare_b_buf1_status = 0;
                printf("compare_b_buf1 reg value is matched to counter value\n");
                break;
            }
            if (cb_compare_b_buf2_status) {
                cb_compare_b_buf2_status = 0;
                printf("compare_b_buf2 reg value is matched to counter value\n");
                break;
            }
            if (cb_basic_status) {
                cb_basic_status = 0;
                printf("Interrupt: Overflow occurred\n");
                break;
            }
        }
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d: timer stopped\n", channel);
    }

error_compare_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_compare_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: compare mode completed *** \r\n\n");
}

/**
 * @function    void utimer_dt_mode_cb_func(event)
 * @brief       utimer basic mode callback function
 * @note        none
 * @param       event
 * @retval      none
 */
static void utimer_dt_mode_cb_func(uint32_t event)
{
    if (event == ARM_UTIMER_UNDER_FLOW_EVENT) {
        cb_dt_status = 1;
    }
}

/**
 * @function    void utimer_dt_mode_app(void)
 * @brief       utimer DeadTime mode application
 * @note        none
 * @param       none
 * @retval      none
 */
static void utimer_dt_mode_app()
{
    int32_t ret;
    uint8_t channel = 6;
    uint32_t count_array[ARM_UTIMER_MODE_DEAD_TIME_COUNT_NUMBER];

    UTIMER_MODE_CONFIG config_info = {
        .mode = ARM_UTIMER_MODE_DEAD_TIME,
        .direction = ARM_UTIMER_COUNT_DIRECTION_TRIANGLE,
        .count_array = count_array,
        .count_number = ARM_UTIMER_MODE_DEAD_TIME_COUNT_NUMBER
    };

    /*
     * utimer channel 6 is configured for utimer dt mode. Using triangular counter for this mode.
     * Initially Compare B is assigned value greater than overflow/underflow value.
     * And it is auto assigned at next overflow/underflow event.
     * observe driver A output signal from P3_8 and driver B output signal form P3_9.
     */

    printf("*** utimer demo application for dead time mode started ***\n");
    /*
     * System CLOCK frequency (F)= 400Mhz
     *
     * Time for 1 count T = 1/F = 1/(400*10^6) = 0.0025 * 10^-6
     *
     * To Increment or Decrement Timer by 1 count, takes 0.0025 micro sec
     *
     * So count for 1 sec = 1/(0.0025*(10^-6)) = 400000000
     * DEC = 400000000
     * HEX = 0x17D78400
     *
     * So count for 250ms = (250*(10^-3)/(0.0025*(10^-6)) = 100000000
     * DEC = 100000000
     * HEX = 0x5F5E100
     *
     * So count for 500ms = (500*(10^-3)/(0.0025*(10^-6)) = 200000000
     * DEC = 200000000
     * HEX = 0xBEBC200
     *
     * So count for 350ms = (350*(10^-3)/(0.0025*(10^-6)) = 140000000
     * DEC = 140000000
     * HEX = 0x8583B00
     *
     */

    count_array[0] = 0x00000000;         /*< initial counter value >*/
    count_array[1] = 0x17D78400;         /*< overflow/underflow count value >*/
    count_array[2] = 0x5F5E100;          /*< dt up reg value>*/
    count_array[3] = 0x5F5E100;          /*< dt down reg value>*/
    count_array[4] = 0x8583B00;          /*< dt up buf value>*/
    count_array[5] = 0x8583B00;          /*< dt down buf value>*/
    count_array[6] = 0xBEBC200;          /*< compare a value>*/
    count_array[7] = 0x17D78401;         /*< compare b value>*/

    /* dt mode pin confg */
    ret = pinmux_config(config_info.mode);
    if (ret != ARM_DRIVER_OK) {
        printf("pinmux failed\n");
    }

    ret = ptrUTIMER->Initialize (channel, utimer_dt_mode_cb_func);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed initialize \n", channel);
        return;
    }

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power up \n", channel);
        goto error_deadtime_mode_uninstall;
    }

    ret = ptrUTIMER->Control (channel, ARM_UTIMER_MODE_CONFIG, &config_info);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d mode configuration failed \n", channel);
        goto error_deadtime_mode_poweroff;
    }

    ret = ptrUTIMER->Start (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to start \n", channel);
        goto error_deadtime_mode_poweroff;
    } else {
        printf("utimer channel %d :timer started\n", channel);
    }

    /* Testing purpose: checking for 2 underflow i.e, 2 cycles of triangular counter */
    for (int index=0; index<=1; index++)
    {
        while(1)
        {
            if (cb_dt_status) {
                cb_dt_status = 0;
                printf("Interrupt: underflow occurred\n");
                break;
            }
        }
    }

    ret = ptrUTIMER->Stop (channel, UTIMER_COUNTER_CLEAR);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to stop \n", channel);
    } else {
        printf("utimer channel %d: timer stopped\n", channel);
    }

error_deadtime_mode_poweroff:

    ret = ptrUTIMER->PowerControl (channel, ARM_POWER_OFF);
    if (ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed power off \n", channel);
    }

error_deadtime_mode_uninstall:

    ret = ptrUTIMER->Uninitialize (channel);
    if(ret != ARM_DRIVER_OK) {
        printf("utimer channel %d failed to un-initialize \n", channel);
    }

    printf("*** demo application: dead time mode completed *** \r\n\n");
}

int main()
{
    utimer_basic_mode_app();
    utimer_buffering_mode_app();
    utimer_trigger_mode_app();
    utimer_capture_mode_app();
    utimer_compare_mode_app();
    utimer_dt_mode_app();
}
