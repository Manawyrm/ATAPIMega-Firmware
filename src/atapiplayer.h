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
 * \defgroup ATAPIPLAYER ATAPIPlayer
 * \brief Audio Player
 *
 * Implements an Audio CD Player using the ATAPI-Interface.
 *
 * @{
 **/


#ifndef __ATAPIPLAYER_H__INCLUDED__
#define __ATAPIPLAYER_H__INCLUDED__


#include "atapiplayer_config.h"
#include "atapi.h"

#include <stdint.h>
#include <stdbool.h>


#define ATAPIPLAYER_STATUS_TRAYCLOSED	_BV(0)
#define ATAPIPLAYER_STATUS_CDINSERTED	_BV(1)
#define ATAPIPLAYER_STATUS_PLAYING	_BV(2)
#define ATAPIPLAYER_STATUS_PAUSED	_BV(3)


/// Audio player status.
/**
 * Any bit-combination of:
 * - \e ATAPIPLAYER_STATUS_TRAYCLOSED
 * - \e ATAPIPLAYER_STATUS_CDINSERTED
 * - \e ATAPIPLAYER_STATUS_PLAYING
 * - \e ATAPIPLAYER_STATUS_PAUSED
 **/
typedef uint8_t atapiplayer_status_t;


/// Audio player data struct.
typedef struct
{
#ifdef ATAPI_USE_MALLOC
	atapi_track_MSF_t * tracks;	///< Pointer to track array.
#else
	atapi_track_MSF_t tracks[ATAPI_MAX_TRACKS];	///< Track array.
#endif
	int8_t numTracks;		///< Number of tracks in track array.
	int8_t firstTrackNr;		///< The track number of the first track in track array.
	int8_t firstAudioTrack;		///< Index of the first track containing audio data.
	int8_t lastAudioTrack;		///< Index of the last track containing audio data.
	atapi_readSubChannel_currentPosition_MSF_t currentSubChannel;	///< Buffered sub-channel data. Updated on atapiplayer_update( atapiplayer_t * ).
	atapi_readSubChannel_currentPosition_MSF_t lastSubChannel;	///< Last sub-channel data.
	atapiplayer_status_t status;	///< Audio player status.
} atapiplayer_t;


/// Initializes audio player on current device.
bool atapiplayer_init( atapiplayer_t * player );
/// Poll drive.
bool atapiplayer_update( atapiplayer_t * player );
/// On pause, this will resume to playback audio. If the player is stopped, the playback will begin at the first audio track.
bool atapiplayer_play( atapiplayer_t * player );
/// The playback will begin at the chosen MSF address.
bool atapiplayer_playMSF( atapiplayer_t * player, const atapi_msf_t * address );
/// The playback will begin at the chosen audio track.
bool atapiplayer_playTrack( atapiplayer_t * player, int8_t trackIndex );
/// The device will be stopped.
bool atapiplayer_stop( atapiplayer_t * player );
/// Pauses or resumes playback.
bool atapiplayer_pause( atapiplayer_t * player );
/// Jumps to the next audio track - if available.
bool atapiplayer_next( atapiplayer_t * player );
/// Jumps to the previous audio track - if available.
bool atapiplayer_previous( atapiplayer_t * player );
/// Fast forward.
bool atapiplayer_forward( atapiplayer_t * player );
/// Rewind.
bool atapiplayer_rewind( atapiplayer_t * player );
/// Load/Eject.
bool atapiplayer_loadEject( atapiplayer_t * player );


#endif


/** @} **/
