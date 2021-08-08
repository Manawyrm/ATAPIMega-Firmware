#include <Arduino.h>
#include <stdio.h>
#include "ata.h"
#include "atapiplayer.h"
#include "display.h"

static FILE uartout = {0};
static int uart_putchar(char c, FILE *stream)
{
	Serial.write(c);
	return 0;
}

atapiplayer_t player;

void setup()
{
	Serial.begin(115200);

	// setup stdout, get printf working
	fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &uartout;

	display_init();

	ata_init();
	delay(1000);

	atapi_init();
	delay(1000);

	atapiplayer_init(&player);

	while (!atapi_testUnitReady())
	{
		delay(100);
	}

	atapiplayer_update(&player);
	atapiplayer_update(&player);
}

void loop()
{
	atapiplayer_update(&player);
	display_update(&player);

	delay(100);

	if (Serial.available())
	{
		switch (Serial.read())
		{
		/*
				p - Play
				Space - Pause
				a - Previous
				d - Next
				
			 */
		case 'p':
			atapiplayer_play(&player);
			break;
		case 's':
			atapiplayer_stop(&player);
			break;
		case ' ':
			atapiplayer_pause(&player);
			break;
		case 'd':
			atapiplayer_next(&player);
			break;
		case 'a': 
			atapiplayer_previous(&player);
			break;
		case 'e':
			atapiplayer_loadEject(&player);
			break;
		case 'f':
			atapiplayer_rewind(&player);
			break; 
		case 'g':
			atapiplayer_forward(&player);
			break;	
		default : break;
		}
	}
}
