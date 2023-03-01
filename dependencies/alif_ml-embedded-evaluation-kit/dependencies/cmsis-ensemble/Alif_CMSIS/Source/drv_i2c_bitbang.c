#include "Driver_PINMUX_AND_PINPAD.h"
#include "drv_i2c_bitbang.h"
#include "system_utils.h"

/* Half of I2C Cycle Delay */
#define i2c_delay() PMU_delay_loop_us(2)
#define i2c_half_delay() PMU_delay_loop_us(1)

volatile int started = 0;;

void i2c_init () {

	/* Required for I2C Bit-Bang to work */
	HW_REG32(CFGMST0_BASE,0x0) |= (1 << 0) | (1 << 4);

	i2c_SDA_High(); /* Setting the IO level before PINMUX prevents glitches */
	i2c_SCL_High();
	i2c_SDA_Input();	/* PAD in open-drain mode uses GPIO input */
	i2c_SCL_Input();

	PINMUX_Config(PINMUX_SDA_PORT_NUMBER, PINMUX_SDA_PIN_NUMBER, 0);
	PINPAD_Config(PINMUX_SDA_PORT_NUMBER, PINMUX_SDA_PIN_NUMBER, \
						PAD_FUNCTION_READ_ENABLE |
						PAD_FUNCTION_DRIVER_OPEN_DRAIN);

	PINMUX_Config(PINMUX_SCL_PORT_NUMBER, PINMUX_SCL_PIN_NUMBER, 0);
	PINPAD_Config(PINMUX_SCL_PORT_NUMBER, PINMUX_SCL_PIN_NUMBER, \
						PAD_FUNCTION_READ_ENABLE |
						PAD_FUNCTION_DRIVER_OPEN_DRAIN);

	i2c_delay();
	i2c_delay();
	i2c_delay();
	i2c_delay();
}

int i2c_start () {

	if (started)  {
		/* pre-conditions */
		i2c_SDA_High();
		i2c_delay();
		i2c_SCL_High();

		while (1) {
			/* loop as long as SCL is Low */
			if (i2c_SCL_Status() != 0) break;
		}

		i2c_delay();
		i2c_delay();
	}

	/* start condition */
	i2c_SDA_Low();
	i2c_delay();
	i2c_SCL_Low();
	i2c_delay();

	started = 1;
	return 0;
}

int i2c_stop () {

	i2c_SDA_Low();
	i2c_delay();
	i2c_SCL_High();

	while (1) {
		/* loop as long as SCL is Low */
		if (i2c_SCL_Status() != 0) break;
	}

	/* stop condition */
	i2c_delay();
	i2c_SDA_High();
	i2c_delay();

	/* extra delay */
	i2c_delay();
	i2c_delay();

	started = 0;
	return 0;
}

int i2c_write_byte (uint8_t data_byte) {
	for(int i = 8; i; i--) {
		if ((data_byte & 0x80) != 0)
			i2c_SDA_High();
		else
			i2c_SDA_Low();

		data_byte <<= 1;
		i2c_half_delay();
		i2c_SCL_High();
		i2c_delay();
		i2c_SCL_Low();
		i2c_half_delay();
	}

	i2c_half_delay();
	i2c_SDA_High();
	i2c_SCL_High();
	i2c_half_delay();

	int ack = i2c_SDA_Status() == 0 ? 1 : 0;    // Acknowledge bit
	i2c_half_delay();

	i2c_SCL_Low();
	i2c_SDA_Low();
	i2c_delay();

	return ack;
}

uint8_t i2c_read_byte (int ack) {
	uint8_t rcv_data = 0;

	i2c_SDA_High();
	for (int i = 0; i < 8; i++) {
		rcv_data <<= 1;
		do {
			i2c_SCL_High();
		} while(i2c_SCL_Status() == 0);
		i2c_half_delay();

		if (i2c_SDA_Status() != 0)
			rcv_data |= 1;

		i2c_half_delay();
		i2c_SCL_Low();
		i2c_delay();
	}

	if (ack) {
		i2c_SDA_Low();
	}
	else {
		i2c_SDA_High();
	}
	i2c_SCL_High();
	i2c_delay();

	i2c_SCL_Low();
	i2c_SDA_High();
	i2c_delay();

	return rcv_data;
}

int i2c_master_WriteData (uint8_t addr, const uint8_t *data, uint16_t len) {

	if (i2c_start()) return -1;

	i2c_write_byte((addr << 1));
	i2c_delay();

	for (int i = 0; i < len; i++) {
		i2c_write_byte(data[i]);
		i2c_delay();
	}

	i2c_stop();

	return 0;
}

int i2c_master_ReadData (uint8_t addr, uint8_t *data, uint16_t len) {

	if (i2c_start()) return -1;

	i2c_write_byte((addr << 1) | 1);

	len--;
	for (int i = 0; i < len; i++) {
		data[i] = i2c_read_byte(1);
	}
	data[len] = i2c_read_byte(0);

	i2c_stop();

	return 0;
}

