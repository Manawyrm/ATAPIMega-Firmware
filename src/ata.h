#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "config.h"

void ata_set_register(uint8_t reg);
void ata_write_register(uint8_t reg, uint16_t data);
uint16_t ata_read_register(uint8_t reg);
void ata_init();
bool ata_waitStatus(uint8_t *status);

uint16_t ata_read16(uint8_t reg);
uint8_t ata_read8( uint8_t reg );
void ata_write16( uint8_t reg, uint16_t data );
void ata_write8( uint8_t reg, uint8_t data );
