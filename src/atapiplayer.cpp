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


#include "atapiplayer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void atapiplayer_reset( atapiplayer_t * player )
{
#if ATAPI_USE_MALLOC
	free( player->tracks );
#endif
	memset( player, 0, sizeof(atapiplayer_t) );
	player->firstAudioTrack = -1;
	player->lastAudioTrack = -1;
}


bool atapiplayer_init( atapiplayer_t * player )
{
	atapiplayer_reset( player );
	return true;
}


bool atapiplayer_update( atapiplayer_t * player )
{
	if( atapi_testUnitReady() )
	{
		if( !( player->status & ATAPIPLAYER_STATUS_CDINSERTED ) )
		{
			player->status |= ATAPIPLAYER_STATUS_CDINSERTED | ATAPIPLAYER_STATUS_TRAYCLOSED;
			printf( "\nReading TOC:\n" );
			player->numTracks = ATAPI_MAX_TRACKS;
#if ATAPI_USE_MALLOC
			free( player->tracks );
			player->tracks = atapi_readTOC_MSF_malloc( &player->numTracks, &player->firstTrackNr );
			if( !player->tracks )
#else
			if( !atapi_readTOC_MSF( player->tracks, &player->numTracks, &player->firstTrackNr ) )
#endif
			{
				printf( "Couldn't read\n" );
				atapiplayer_reset( player );
				return false;
			}
			if( player->numTracks <= 1 )	// at least one track and lead out track needed
			{
				printf( "No content found\n" );
				return false;
			}
			for( int8_t i=0; i<player->numTracks-1; i++ )
			{
				if( !( player->tracks[i].qADRControl & ATAPI_QFIELDCONTROL_DATATRACK ) )
				{
					player->firstAudioTrack = i;
					break;
				}
			}
			for( int8_t i=player->numTracks-2; i>=0; i-- )	// from the track before lead-out track backwards
			{
				if( !( player->tracks[i].qADRControl & ATAPI_QFIELDCONTROL_DATATRACK ) )
				{
					player->lastAudioTrack = i;
					break;
				}
			}
			for( int8_t i=0; i<player->numTracks; i++ )
			{
				printf( " %c Track %02d  qADRControl 0x%02X  Start %02d:%02d:%02d\n",
					( i>=player->firstAudioTrack && i<=player->lastAudioTrack )?'#':'*',
					i+player->firstTrackNr, player->tracks[i].qADRControl,
					player->tracks[i].address.minutes, player->tracks[i].address.seconds, player->tracks[i].address.frames );
			}
			if( player->firstAudioTrack < 0 || player->lastAudioTrack < 0 )
			{
				printf( "No Audio Tracks found\n" );
				return false;
			}
			atapiplayer_play( player );
			atapiplayer_stop( player );
		}

#if !ATAPIPLAYER_SLIMMODE
		// relying on the audio status information seems unreliable on some slim atapi drives
		player->lastSubChannel = player->currentSubChannel;
		atapi_readSubChannel_currentPosition_MSF( &(player->currentSubChannel) );
		switch( player->lastSubChannel.audioStatus )
		{
			case ATAPI_READSUBCHANNEL_AUDIOSTATUS_PAUSED:
				player->status |= ATAPIPLAYER_STATUS_PAUSED | ATAPIPLAYER_STATUS_PLAYING;
				break;
			case ATAPI_READSUBCHANNEL_AUDIOSTATUS_PLAYING:
				player->status |= ATAPIPLAYER_STATUS_PLAYING;
				player->status &= ~( ATAPIPLAYER_STATUS_PAUSED );
				break;
			default:
				player->status &= ~( ATAPIPLAYER_STATUS_PAUSED | ATAPIPLAYER_STATUS_PLAYING );
				break;
		}
#endif

		if( player->status & ATAPIPLAYER_STATUS_PLAYING && !(player->status & ATAPIPLAYER_STATUS_PAUSED) )
		{
#if ATAPIPLAYER_SLIMMODE
			player->lastSubChannel = player->currentSubChannel;
			atapi_readSubChannel_currentPosition_MSF( &(player->currentSubChannel) );
#endif
			if( player->lastSubChannel.relative.seconds!=player->currentSubChannel.relative.seconds
				|| player->lastSubChannel.audioStatus != player->currentSubChannel.audioStatus )
			{
				printf( "Track %02d  Index %02d  Absolute %02d:%02d:%02d  Relative %02d:%02d:%02d  State 0x%02X  qADRControl 0x%02X\r",
					player->currentSubChannel.track, player->currentSubChannel.index,
					player->currentSubChannel.absolute.minutes, player->currentSubChannel.absolute.seconds, player->currentSubChannel.absolute.frames,
					player->currentSubChannel.relative.minutes, player->currentSubChannel.relative.seconds, player->currentSubChannel.relative.frames,
					player->currentSubChannel.audioStatus, player->currentSubChannel.qADRControl );
			}
		}
	}
	else
	{
		atapiplayer_reset( player );
	}
	return true;
}


int8_t atapiplayer_getNextAudioTrack( atapiplayer_t * player )
{
	if( player->numTracks<=0 )
		return -1;
	int8_t current = player->currentSubChannel.track - player->firstTrackNr;
	for( int8_t i=current+1; i<player->numTracks-1; i++ )
	{
		if( !( player->tracks[i].qADRControl & ATAPI_QFIELDCONTROL_DATATRACK ) )
		{
			return i;
		}
	}
	return -1;
}


int8_t atapiplayer_getPreviousAudioTrack( atapiplayer_t * player )
{
	if( player->numTracks<=0 )
		return -1;
	int8_t current = player->currentSubChannel.track - player->firstTrackNr;
	for( int8_t i=current-1; i>=0; i-- )
	{
		if( !( player->tracks[i].qADRControl & ATAPI_QFIELDCONTROL_DATATRACK ) )
		{
			return i;
		}
	}
	return -1;
}


bool atapiplayer_play( atapiplayer_t * player )
{
	if( !( player->status & ATAPIPLAYER_STATUS_TRAYCLOSED ) )	// tray not closed? close it
		return atapi_startStopUnit( ATAPI_STARTSTOPUNIT_LOADEJECT | ATAPI_STARTSTOPUNIT_START );

	if( !( player->status & ATAPIPLAYER_STATUS_CDINSERTED ) )	// tray closed but no cd inserted? abort
		return false;

	if( player->firstAudioTrack<0 || player->lastAudioTrack<0 )
		return false;

	if( player->status & ATAPIPLAYER_STATUS_PLAYING )
	{
		if( player->status & ATAPIPLAYER_STATUS_PAUSED )
			return atapiplayer_pause( player );
		else
#if !ATAPIPLAYER_SOFTSCAN
			return atapi_playAudio_MSF( &(player->currentSubChannel.absolute), &(player->tracks[player->lastAudioTrack+1].address) );	// revert to normal play, in case a scan command is executing
#else
			return true;
#endif
	}
#if ATAPIPLAYER_SLIMMODE
	else
	{
		atapi_startStopUnit( ATAPI_STARTSTOPUNIT_START );
	}

	if( !atapi_playAudio_MSF( &(player->tracks[player->firstAudioTrack].address), &(player->tracks[player->lastAudioTrack+1].address) ) )
		return false;

	player->status |= ATAPIPLAYER_STATUS_PLAYING;
	return true;

#else

	return atapi_playAudio_MSF( &(player->tracks[player->firstAudioTrack].address), &(player->tracks[player->lastAudioTrack+1].address) );
#endif
}


bool atapiplayer_playMSF( atapiplayer_t * player, const atapi_msf_t * address )
{
	if( !( player->status & ATAPIPLAYER_STATUS_TRAYCLOSED ) )	// tray not closed? close it
		return atapi_startStopUnit( ATAPI_STARTSTOPUNIT_LOADEJECT | ATAPI_STARTSTOPUNIT_START );

	if( !( player->status & ATAPIPLAYER_STATUS_CDINSERTED ) )	// tray closed but no cd inserted? abort
		return false;

	if( player->firstAudioTrack<0 || player->lastAudioTrack<0 )
		return false;

#if ATAPIPLAYER_SLIMMODE
	if( !(player->status & ATAPIPLAYER_STATUS_PLAYING) )
		atapi_startStopUnit( ATAPI_STARTSTOPUNIT_START );

	if( !atapi_playAudio_MSF( address, &(player->tracks[player->lastAudioTrack+1].address) ) )
		return false;

	player->status |= ATAPIPLAYER_STATUS_PLAYING;
	return true;

#else

	return atapi_playAudio_MSF( address, &(player->tracks[player->lastAudioTrack+1].address) );
#endif
}


bool atapiplayer_playTrack( atapiplayer_t * player, int8_t trackIndex )
{
	if( player->firstAudioTrack<0 || player->lastAudioTrack<0 )
		return false;
	if( trackIndex < player->firstAudioTrack || trackIndex > player->lastAudioTrack )
		return false;
	return atapiplayer_playMSF( player, &(player->tracks[trackIndex].address) );
}


bool atapiplayer_stop( atapiplayer_t * player )
{
#if ATAPIPLAYER_SLIMMODE
#if ATAPI_USE_LBA
	atapi_seek( 0 );	// allows some slim drives to spin down after stopping
#endif
	if( !atapi_startStopUnit( 0 ) )
		return false;

	player->status &= ~( ATAPIPLAYER_STATUS_PAUSED | ATAPIPLAYER_STATUS_PLAYING );
	return true;

#else

	return atapi_stop();
#endif
}


bool atapiplayer_pause( atapiplayer_t * player )
{
	if( !(player->status & ATAPIPLAYER_STATUS_PLAYING) )
		return false;

#if ATAPIPLAYER_SLIMMODE
	if( player->status & ATAPIPLAYER_STATUS_PAUSED )
	{
		atapi_startStopUnit( ATAPI_STARTSTOPUNIT_START );
		if( !atapi_playAudio_MSF( &(player->currentSubChannel.absolute), &(player->tracks[player->lastAudioTrack+1].address) ) )
			return false;
		player->status &= ~ATAPIPLAYER_STATUS_PAUSED;
	}
	else
	{
#if ATAPI_USE_LBA
		atapi_seek( 0 );	// allows some slim drives to spin down after stopping
#endif
		if( !atapi_startStopUnit( 0 ) )
			return false;
		player->status |= ATAPIPLAYER_STATUS_PAUSED;

	}
	return true;

#else

	return atapi_pauseResume( player->status & ATAPIPLAYER_STATUS_PAUSED );
#endif
}


bool atapiplayer_next( atapiplayer_t * player )
{
	if( !(player->status & ATAPIPLAYER_STATUS_PLAYING) )
		return false;

	int8_t next = atapiplayer_getNextAudioTrack( player );
	if( next<0 || player->lastAudioTrack<0 )
		return false;

	return atapi_playAudio_MSF( &(player->tracks[next].address), &(player->tracks[player->lastAudioTrack+1].address) );
}


bool atapiplayer_previous( atapiplayer_t * player )
{
	if( !(player->status & ATAPIPLAYER_STATUS_PLAYING) )
		return false;

	int8_t previous = atapiplayer_getPreviousAudioTrack( player );
	if( previous<0 || player->lastAudioTrack<0 )
		return false;

	return atapi_playAudio_MSF( &(player->tracks[previous].address), &(player->tracks[player->lastAudioTrack+1].address) );
}


bool atapiplayer_forward( atapiplayer_t * player )
{
	if( player->numTracks<=0 )
		return false;

	if( !(player->status & ATAPIPLAYER_STATUS_PLAYING) )
		return false;

	atapi_readSubChannel_currentPosition_MSF_t current = player->currentSubChannel;
#if ATAPIPLAYER_SOFTSCAN
	if( current.absolute.seconds >= 60-ATAPIPLAYER_SOFTFORWARD_DISTANCE )
	{
		current.absolute.seconds -= 60 - ATAPIPLAYER_SOFTFORWARD_DISTANCE;
		current.absolute.minutes++;
	}
	else
	{
		current.absolute.seconds += ATAPIPLAYER_SOFTFORWARD_DISTANCE;
	}

	return atapi_playAudio_MSF( &current.absolute, &(player->tracks[player->lastAudioTrack+1].address) );

#else

	return atapi_scan_MSF( &current.absolute, false );
#endif
}


bool atapiplayer_rewind( atapiplayer_t * player )
{
	if( player->numTracks<=0 )
		return false;

	if( !(player->status & ATAPIPLAYER_STATUS_PLAYING) )
		return false;

	atapi_readSubChannel_currentPosition_MSF_t current = player->currentSubChannel;
#if ATAPIPLAYER_SOFTSCAN
	if( current.absolute.seconds < ATAPIPLAYER_SOFTREWIND_DISTANCE )
	{
		if( current.absolute.minutes > 0 )
		{
			current.absolute.seconds += 60 - ATAPIPLAYER_SOFTREWIND_DISTANCE;
			current.absolute.minutes--;
		}
		else
		{
			current.absolute.seconds = 0;
		}
	}
	else
	{
		current.absolute.seconds -= ATAPIPLAYER_SOFTREWIND_DISTANCE;
	}

	return atapi_playAudio_MSF( &current.absolute, &(player->tracks[player->lastAudioTrack+1].address) );

#else

	return atapi_scan_MSF( &current.absolute, true );
#endif
}


bool atapiplayer_loadEject( atapiplayer_t * player )
{
	if( (player->status & ATAPIPLAYER_STATUS_TRAYCLOSED) )
	{
		atapi_startStopUnit( ATAPI_STARTSTOPUNIT_LOADEJECT );
		atapiplayer_reset( player );
	}
	else
	{
		atapi_startStopUnit( ATAPI_STARTSTOPUNIT_LOADEJECT | ATAPI_STARTSTOPUNIT_START );
		player->status |= ATAPIPLAYER_STATUS_TRAYCLOSED;
	}
	return true;
}
