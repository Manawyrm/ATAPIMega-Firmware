ATAPIMega Firmware
=======

[ATAPIMega](https://github.com/Manawyrm/ATAPIMega) is a hardware project which allows regular (P)ATA CD/DVD drives to be used as a standalone Audio-CD player.  
This repository contains the (Arduino-based) firmware for the ATmega2560 microcontroller.  

### Feature/Bug Wishlist
- Show TOC info after disc insertion (total length, tracks)
- Play hidden tracks properly (Track 0, etc.)
- Playing/Skipping over the end of a CD will cause the display of erroneous info
- Eject/Play should close the tray if opened (this works on some drives, but not all!)

![Photo of HD44780 LCD showing track info](https://screenshot.tbspace.de/gvkdehsaoix.jpg)

Project is ideally built with PlatformIO, but should work in the Arduino IDE as well.  
Pin configurations are set in the `config.h` file, look into the schematic for electrical details.  

ATAPIMega does not handle any audio data itself, the CD drive is playing the CDs standalone and only gets SCSI/ATAPI commands like Play, Pause, Next, etc. from the microcontroller. 

ATAPI and CD player implementation is based on [IDETrol](https://github.com/zwostein/idetrol) by [Tobias Himmer](https://github.com/zwostein), thanks!  
Thanks to [@faultierkatze](https://github.com/faultierkatze) for the inspiration for this project, UI layout and general feedback!

Licensed under GPLv3!  
Copyright (c) 2021 Tobias Mädel \<t.maedel@alfeld.de\>  
Copyright (c) 2011 Tobias Himmer \<provisorisch@online.de\>  
  
[https://tbspace.de](https://tbspace.de)
