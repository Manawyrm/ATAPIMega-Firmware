/*
 * Copyright (c) 2011 Tobias Himmer <provisorisch@online.de>
 *
 * This file is part of ATAPIMega-Firmware and was modified.
 * This file used to be part of IDETrol (https://github.com/zwostein/idetrol).
 *
 * ATAPIMega-Firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATAPIMega-Firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ATAPIMega-Firmware.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \defgroup ATAPIPLAYER_CONFIG ATAPIPlayer Configuration
 * \brief ATAPI configuration.
 *
 * Configuration options for the ATAPI Player.
 *
 * @{
 **/


#ifndef __ATAPIPLAYER_CONFIG_H__INCLUDED__
#define __ATAPIPLAYER_CONFIG_H__INCLUDED__


#ifndef ATAPIPLAYER_SOFTSCAN
/// Use software scan instead of the \e ATAPI scan command.
/**
 * The support for the \e ATAPI scan command is optional! It is safe to enable softscan as this will work on every drive.
 * Use \e ATAPIPLAYER_SOFTFORWARD_DISTANCE and \e ATAPIPLAYER_SOFTREWIND_DISTANCE to adjust the jump distance on every fast-forward/rewind function call.
 * If softscan is disabled, the \e ATAPI scan command is used and the scan continues until the play button is pressed.
 **/
#define ATAPIPLAYER_SOFTSCAN		1
#endif

#ifndef ATAPIPLAYER_SOFTFORWARD_DISTANCE
/// How many seconds to jump forward when using software scan.
#define ATAPIPLAYER_SOFTFORWARD_DISTANCE	5
#endif

#ifndef ATAPIPLAYER_SOFTREWIND_DISTANCE
/// How many seconds to jump back when using software scan.
#define ATAPIPLAYER_SOFTREWIND_DISTANCE		5
#endif

#ifndef ATAPIPLAYER_SLIMMODE
/// If enabled, the player is compiled in a more slim-ATAPI-drive friendly mode.
/**
 * In this mode, the player won't use the audio stop and pause commands. It seems these don't work reliable on slim-drives.
 * Instead, the complete drive is stopped. You should also enable LBA support for the ATAPI interface:
 * Before the drive is stopped, the head is moved to the beginning of the disk. This causes (some?) drives to spin down.
 * Also, the audio status on slim-drives is ignored as some drives don't seem to report play operation correctly.
 **/
#define ATAPIPLAYER_SLIMMODE	0
#endif


#endif


/** @} **/
