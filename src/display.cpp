#include "display.h"
#include <stdarg.h>
byte display_custom_stop[] = {
    B00000,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000,
    B00000
};

byte display_custom_play[] = {
    B10000,
    B11000,
    B11100,
    B11110,
    B11100,
    B11000,
    B10000,
    B00000,
};

byte display_custom_pause[] = {
    B11011,
    B11011,
    B11011,
    B11011,
    B11011,
    B11011,
    B00000,
    B00000,

};

byte display_custom_pause_left[] = {
    B00000,
    B00000,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011
};

byte display_custom_pause_right[] = {
    B00000,
    B00000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000
};

LiquidCrystal lcd(
    CONFIG_HD44780_RS,
    CONFIG_HD44780_RW,
    CONFIG_HD44780_E,
    CONFIG_HD44780_D0,
    CONFIG_HD44780_D1,
    CONFIG_HD44780_D2,
    CONFIG_HD44780_D3,
    CONFIG_HD44780_D4,
    CONFIG_HD44780_D5,
    CONFIG_HD44780_D6,
    CONFIG_HD44780_D7
);

void display_init()
{
    lcd.createChar(0, display_custom_play);
    lcd.createChar(1, display_custom_stop);
    lcd.createChar(2, display_custom_pause);

    lcd.begin(16, 4);
    lcd.print("ATAPI CD-Player");
    lcd.setCursor(0,1);
    lcd.print("@faultierkatze");
}

void subtract_msf_length(atapi_msf_t *target, atapi_msf_t *next, atapi_msf_t *current)
{
    uint32_t current_frames = (((current->minutes * 60UL) + current->seconds) * 74UL) + current->frames;
    uint32_t next_frames = (((next->minutes * 60UL) + next->seconds) * 74UL) + next->frames;

    /*printf(" Cur: F: %lu Next: F: %lu \n", current_frames, next_frames);

    printf(" Cur M: %02d S: %02d\n", current->minutes, current->seconds);
    printf("Next M: %02d S: %02d\n", next->minutes, next->seconds);*/

    uint32_t length_frames = next_frames - current_frames;
    uint32_t length_seconds = length_frames / 74;

    target->minutes = length_seconds / 60;
    target->seconds = length_seconds % 60;

    //printf("Target M: %02d S: %02d\n", target->minutes, target->seconds);

    return;
}

char lcdbuffer[17];
atapiplayer_status_t last_status = 0;

void display_update(atapiplayer_t *player)
{
    if (last_status != player->status)
    {
        lcd.clear();
        last_status = player->status;
    }

    // CD inserted and tray closed
    if ((player->status & ATAPIPLAYER_STATUS_CDINSERTED) && (player->status & ATAPIPLAYER_STATUS_TRAYCLOSED))
    {
        // Display track info
        lcd.setCursor(2, 0);
        snprintf(lcdbuffer, sizeof(lcdbuffer), "Track %02d/%02d", player->currentSubChannel.track, player->lastAudioTrack + 1);
        lcd.print(lcdbuffer);

        // Calculate total track length by subtracting the (absolute) start 
        // of the next track from the (absolute) start of the current one
        atapi_msf_t track_length;
        subtract_msf_length(
            &track_length, 
            &((player->tracks[player->currentSubChannel.track]).address), 
            &((player->tracks[player->currentSubChannel.track - 1]).address)
        );

        lcd.setCursor(2, 1);
        snprintf(lcdbuffer, sizeof(lcdbuffer), "%02d:%02d / %02d:%02d", player->currentSubChannel.relative.minutes, player->currentSubChannel.relative.seconds, track_length.minutes, track_length.seconds);
        lcd.print(lcdbuffer);
        

        // Display status symbol
        if (player->status & ATAPIPLAYER_STATUS_PLAYING)
        {
            if (player->status & ATAPIPLAYER_STATUS_PAUSED)
            {
                lcd.setCursor(0, 0);
                lcd.write((byte)2);
            }
            else
            {
                lcd.setCursor(0, 0);
                lcd.write((byte)0);
            }
        }
        else
        {
            lcd.setCursor(0, 0);
            lcd.write((byte)1);
        }
    }
    else
    {
        if (!(player->status & ATAPIPLAYER_STATUS_TRAYCLOSED))
        {
            lcd.setCursor(0, 0);
            lcd.print("CD einlegen");
        }
        else
        {       
            lcd.setCursor(0, 0);
            lcd.print("Keine CD");
        }
    }
    

}

void display_log(const char* message, const char* message2)
{
    Serial.println(message);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);

    if (strlen(message2))
    {
        Serial.println(message2);
        lcd.setCursor(0, 1);
        lcd.print(message2);
    }
}