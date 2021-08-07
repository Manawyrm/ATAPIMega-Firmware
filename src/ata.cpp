#include "ata.h"

#define _BV( bit ) ( 1<<(bit) ) 
#define ATA_STATUS_BUSY			_BV(7)

uint8_t ata_waitStatus( uint8_t * status )
{
	*status = ata_read_register( 7 );
	if( !( *status & ATA_STATUS_BUSY ) )
	{
		for( uint8_t i=0; i<10; i++ )
		{
			*status = ata_read_register( 7 );
			if( !( *status & ATA_STATUS_BUSY ) )
			{
				return true;
			}
			delayMicroseconds( i * 1000 );
		}
		Serial.print( "ata_waitStatusTimeout\n" );
		return 0;
	}
	return 1;
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

	Serial.print("register %02x write: %04x \n"/*, reg, data*/);
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

	Serial.print("register addi %02x write: %04x \n"/*, reg, data*/);
}


uint16_t ata_read_register(uint8_t reg)
{
	uint16_t ret = 0;

	// set data lines back to input
	CONFIG_DATAL_DDR = 0x00;
	CONFIG_DATAH_DDR = 0x00;
	// pull-up D7
	//CONFIG_DATAL_PORT = 0x80;

	ata_set_register(reg);

	// read strobe
	digitalWrite(CONFIG_IOR, 0);
	delayMicroseconds(1);

	ret |= CONFIG_DATAL_PIN;
	ret |= CONFIG_DATAH_PIN << 8;

	digitalWrite(CONFIG_IOR, 1);

	//Serial.print("register %02x read: %04x \n", reg, ret);
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
	delayMicroseconds(25);
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

