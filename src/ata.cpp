#include "ata.h"

#define _BV( bit ) ( 1<<(bit) ) 
#define ATA_STATUS_BUSY			_BV(7)
#define ATA_WAITNOTBUSY_DEFAULT_TIMEOUT 160

bool ata_waitStatusTimeout(uint8_t timeout, uint8_t *status)
{
	*status = ata_read8(7);
	if (!(*status & ATA_STATUS_BUSY))
	{
		//printf( "Waiting for device ... " );
		for (uint8_t i = 0; i < timeout; i++)
		{
			*status = ata_read8(7);
			if (!(*status & ATA_STATUS_BUSY))
			{
				//printf( "proceeding\n" );
				return true;
			}
			delay(i);
		}
		//		printf_P( PSTR("timeout\n") );
		//printf("ata_waitStatusTimeout\n");
		return false;
	}
	return true;
}

bool ata_waitStatus(uint8_t *status)
{
	return ata_waitStatusTimeout(ATA_WAITNOTBUSY_DEFAULT_TIMEOUT, status);
}

bool ata_isNotBusy(void)
{
	return !(ata_read8(7) & ATA_STATUS_BUSY);
}

// waits for the selected device beeing ready - aborts on timeout
bool ata_waitNotBusyTimeout(uint8_t timeout)
{
	if (!ata_isNotBusy())
	{
		for (uint8_t i = 0; i < timeout; i++)
		{
			if (ata_isNotBusy())
			{
				return 1;
			}
			delay(i);
		}
		printf("ata_waitNotBusyTimeout\n");
		return false;
	}
	return true;
}

// ata_waitNotBusyTimeout with a default value
bool ata_waitNotBusy(void)
{
	return ata_waitNotBusyTimeout(ATA_WAITNOTBUSY_DEFAULT_TIMEOUT);
}

uint16_t ata_read16( uint8_t reg )
{
	return ata_read_register(reg);
}
uint8_t ata_read8( uint8_t reg )
{
	return ata_read_register(reg) & 0xFF;
}
void ata_write16( uint8_t reg, uint16_t data )
{
	ata_write_register(reg, data);
}
void ata_write8( uint8_t reg, uint8_t data )
{
	ata_write_register(reg, data & 0xFF);
}

void ata_set_register(uint8_t reg)
{
	digitalWrite(CONFIG_CS0, !1);
	digitalWrite(CONFIG_CS1, !0);
	digitalWrite(CONFIG_DA0, reg & 0b001);
	digitalWrite(CONFIG_DA1, reg & 0b010);
	digitalWrite(CONFIG_DA2, reg & 0b100);
}

void ata_set_register_additional(uint8_t reg)
{
	digitalWrite(CONFIG_CS0, !0);
	digitalWrite(CONFIG_CS1, !1);
	digitalWrite(CONFIG_DA0, reg & 0b001);
	digitalWrite(CONFIG_DA1, reg & 0b010);
	digitalWrite(CONFIG_DA2, reg & 0b100);
}


void ata_write_register(uint8_t reg, uint16_t data)
{
	ata_set_register(reg);

	CONFIG_DATAL_PORT = data & 0xFF;
	CONFIG_DATAH_PORT = (data >> 8) & 0xFF;
	CONFIG_DATAL_DDR  = 0xFF;
	CONFIG_DATAH_DDR  = 0xFF;

	// write strobe
	digitalWrite(CONFIG_IOW, 0);
	delayMicroseconds(1000);
	digitalWrite(CONFIG_IOW, 1);
	delayMicroseconds(1000);

	// set data lines back to input
	CONFIG_DATAL_DDR = 0x00;
	CONFIG_DATAH_DDR = 0x00;

	//printf("register %02x write: %04x \n", reg, data);
}

void ata_write_register_additional(uint8_t reg, uint16_t data)
{
	ata_set_register_additional(reg);

	CONFIG_DATAL_PORT = data & 0xFF;
	CONFIG_DATAH_PORT = (data >> 8) & 0xFF;
	CONFIG_DATAL_DDR = 0xFF;
	CONFIG_DATAH_DDR = 0xFF;

	// write strobe
	digitalWrite(CONFIG_IOW, 0);
	delayMicroseconds(1000);
	digitalWrite(CONFIG_IOW, 1);
	delayMicroseconds(1000);

	// set data lines back to input
	CONFIG_DATAL_DDR = 0x00;
	CONFIG_DATAH_DDR = 0x00;

	//printf("register addi %02x write: %04x \n", reg, data);
}


uint16_t ata_read_register(uint8_t reg)
{
	uint16_t ret = 0;

	// set data lines back to input
	CONFIG_DATAL_DDR = 0x00;
	CONFIG_DATAH_DDR = 0x00;

	ata_set_register(reg);

	// read strobe
	digitalWrite(CONFIG_IOR, 0);
	delayMicroseconds(1);

	ret |= CONFIG_DATAL_PIN;
	ret |= CONFIG_DATAH_PIN << 8;

	/*if (reg == 7)
	{
		printf("Status register: %04x \n", reg);
	}*/

	digitalWrite(CONFIG_IOR, 1);

	//printf("register %02x read: %04x \n", reg, ret);
	return ret;
}

void ata_init()
{
	// read and write strobe
	pinMode(CONFIG_IOW, OUTPUT);
	pinMode(CONFIG_IOR, OUTPUT);
	digitalWrite(CONFIG_IOW, 1);
	digitalWrite(CONFIG_IOR, 1);

	// register addressing
	pinMode(CONFIG_DA0, OUTPUT);
	pinMode(CONFIG_DA1, OUTPUT);
	pinMode(CONFIG_DA2, OUTPUT);

	// enable both cable select lines
	pinMode(CONFIG_CS0, OUTPUT);
	pinMode(CONFIG_CS1, OUTPUT);
	digitalWrite(CONFIG_CS0, 0);
	digitalWrite(CONFIG_CS1, 1);

	// set data lines back to input
	CONFIG_DATAL_DDR = 0x00;
	CONFIG_DATAH_DDR = 0x00;

	// reset the drives once
	pinMode(CONFIG_RESET, OUTPUT);
	digitalWrite(CONFIG_RESET, 0);
	delay(1000);
	digitalWrite(CONFIG_RESET, 1);

	delay(5);

	// reset drives
	/*	ata_write_register_additional(0, 0x04);
	delayMicroseconds(10000);
	ata_write_register_additional(0, 0x00);
	delayMicroseconds(10000);
	ata_write_register_additional(1, 0x02);
	*/
}

