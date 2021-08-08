#pragma once
#include <Arduino.h>

#define CONFIG_DATAL_PORT PORTL
#define CONFIG_DATAL_PIN PINL
#define CONFIG_DATAL_DDR DDRL

#define CONFIG_DATAH_PORT PORTK
#define CONFIG_DATAH_PIN PINK
#define CONFIG_DATAH_DDR DDRK

#define CONFIG_IOW 36
#define CONFIG_IOR 35

#define CONFIG_DA2 5 
#define CONFIG_DA1 6
#define CONFIG_DA0 7

#define CONFIG_CS1 8
#define CONFIG_CS0 9

#define CONFIG_RESET 31

#define CONFIG_HD44780_RS 14
#define CONFIG_HD44780_RW 15
#define CONFIG_HD44780_E 16
#define CONFIG_HD44780_D0 A0
#define CONFIG_HD44780_D1 A1
#define CONFIG_HD44780_D2 A2
#define CONFIG_HD44780_D3 A3
#define CONFIG_HD44780_D4 A4
#define CONFIG_HD44780_D5 A5
#define CONFIG_HD44780_D6 A6
#define CONFIG_HD44780_D7 A7