#include <Arduino.h>
#include "ata.h"
#include "SHEX.h"

void setup()
{
	Serial.begin(115200);
	ata_init();
}


void cf_dump_registers()
{
	for (int i = 0; i < 8; ++i)
	{
		printf("Register %u: %02x\n", i, ata_read_register(i));
	}
}

uint16_t timeout = 0;
int cf_wait_for_not_busy_and_data_ready()
{
	uint8_t state;

	state = 0xFF;
	while (bitRead(state, 7) || !bitRead(state, 3)) // wait for !busy 0x80 and dry 0x08
	{
		state = ata_read_register(CF_STATUS);
		Serial.print("Status: 0x");
		Serial.println(state, HEX);

		timeout++;
		delayMicroseconds(10000);
		if (timeout == 5000)
		{
			timeout = 0;
			Serial.print("cf_wait_for_not_busy_and_data_ready() !busy/dry timeout\n");
			//cf_dump_registers();
			//abort();
		}
	}
	timeout = 0;
}

int cf_identify(uint8_t *data)
{
	/*
		https://wiki.osdev.org/ATA_PIO_Mode:
		To use the IDENTIFY command, select a target drive by sending 0xA0 for the master drive, or 0xB0 for the slave, 
		to the "drive select" IO port. On the Primary bus, this would be port 0x1F6. Then set the Sectorcount, LBAlo, LBAmid, 
		and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5). Then send the IDENTIFY command (0xEC) to the Command IO port (0x1F7). 
		Then read the Status port (0x1F7) again. If the value read is 0, the drive does not exist. For any other value: 
		poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears. Because of some ATAPI drives that do not follow spec, 
		at this point you need to check the LBAmid and LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero. 
		If so, the drive is not ATA, and you should stop polling. 
		Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
		At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0). Read 256 16-bit values, and store them.
	*/

	int i;

	ata_write_register(CF_ADDR3, 0xA0);

	ata_write_register(CF_NUMSECT, 0x00);
	ata_write_register(CF_ADDR0, 0x00);
	ata_write_register(CF_ADDR1, 0x00);
	ata_write_register(CF_ADDR2, 0x00);

	ata_write_register(CF_COMMAND, 0xEC);

	if (ata_read_register(CF_STATUS) != 0x00)
	{
		cf_wait_for_not_busy_and_data_ready();

		for (i = 0; i < SECTOR_SIZE; i += 2)
		{
			uint16_t cur = ata_read_register(CF_DATA);
			*data++ = (cur >> 8) & 0xFF;
			*data++ = cur & 0xFF;
		}

		return true;
	}
	else
	{
		printf("No device detected!\n");
		return false;
	}
}

uint8_t buf[512];
void loop()
{
	cf_identify(buf);

	SHEX shex(&Serial, 16);
	for (int i = 0; i < 512; i++)
	{
		shex.print(buf[i]);
	}

	delay(10000);
}
