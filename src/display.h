#pragma once
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "config.h"
#include "atapiplayer.h"

void display_init();
void display_update(atapiplayer_t *player);
void display_log(const char *message, const char *message2);

