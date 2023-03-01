#include "global_map.h"
#include <stdint.h>

#define HW_REG32(u,v) *((volatile uint32_t*)(u + v))

#if 1
/*  Tessolve Development Board
 *  P2_22 -> SDA
 *  P2_23 -> SCL
 */
#define BITBANG_BASE	GPIO2_BASE

/* GPIO Port and Pin Number for SDA/SCL. */
#define PINMUX_SDA_PORT_NUMBER         PORT_NUMBER_2
#define PINMUX_SDA_PIN_NUMBER          PIN_NUMBER_22

#define PINMUX_SCL_PORT_NUMBER         PORT_NUMBER_2
#define PINMUX_SCL_PIN_NUMBER          PIN_NUMBER_23
#else
/*  Alif Semi Development Board
 *  P3_8 -> SDA
 *  P3_9 -> SCL
 */
#define BITBANG_BASE	GPIO3_BASE

/* GPIO Port and Pin Number for SDA/SCL. */
#define PINMUX_SDA_PORT_NUMBER         PORT_NUMBER_3
#define PINMUX_SDA_PIN_NUMBER          PIN_NUMBER_8

#define PINMUX_SCL_PORT_NUMBER         PORT_NUMBER_3
#define PINMUX_SCL_PIN_NUMBER          PIN_NUMBER_9
#endif

#define SDA_MASK            (1U << PINMUX_SDA_PIN_NUMBER)
#define SCL_MASK            (1U << PINMUX_SCL_PIN_NUMBER)

#define i2c_SDA_High()      HW_REG32(BITBANG_BASE,0) |= SDA_MASK
#define i2c_SDA_Low()       HW_REG32(BITBANG_BASE,0) &= ~SDA_MASK

#define i2c_SCL_High()      HW_REG32(BITBANG_BASE,0) |= SCL_MASK
#define i2c_SCL_Low()       HW_REG32(BITBANG_BASE,0) &= ~SCL_MASK

#define i2c_SDA_Input()     HW_REG32(BITBANG_BASE,0x4) &= ~SDA_MASK
#define i2c_SDA_Status()    (HW_REG32(BITBANG_BASE,0x50) & SDA_MASK)

#define i2c_SCL_Input()     HW_REG32(BITBANG_BASE,0x4) &= ~SCL_MASK
#define i2c_SCL_Status()    (HW_REG32(BITBANG_BASE,0x50) & SCL_MASK)

void i2c_init();
int i2c_master_WriteData (uint8_t addr, const uint8_t *data, uint16_t len);
int i2c_master_ReadData  (uint8_t addr, uint8_t *data, uint16_t len);

