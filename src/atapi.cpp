/*
 * Copyright (C) 2011 Tobias Himmer <provisorisch@online.de>
 *
 * This file is part of IDETrol.
 *
 * IDETrol is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IDETrol is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with IDETrol.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "atapi.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "display.h"

bool atapi_isDataRequest( void )
{
	uint8_t status;
	if( !ata_waitStatus( &status ) )
		return false;
	return status & ATAPI_STATUS_DATAREQUEST;
}


bool atapi_waitDataRequestTimeout( uint8_t timeout )
{
	if( !atapi_isDataRequest() )
	{
		for( uint8_t i=0; i<timeout; i++ )
		{
			if( atapi_isDataRequest() )
			{
				return true;
			}
			delayMicroseconds( i * 50 );
		}
		printf( "atapi_waitDataRequestTimeout\n" );
		return false;
	}
	return true;
//	return cf_wait_for_not_busy_and_data_ready();
}


bool atapi_waitDataRequest( void )
{
	return atapi_waitDataRequestTimeout( ATAPI_WAITDATAREQUEST_DEFAULT_TIMEOUT );
}


bool atapi_waitNoDataRequest( void )
{
	uint8_t status;
	if( !ata_waitStatus( &status ) )
		return false;
	if( status & ATAPI_STATUS_DATAREQUEST )
	{
		printf( "Drive still expecting data\n" );
		return false;
	}
	return true;
}


bool atapi_writePacketWord( uint16_t source )
{
	if( atapi_waitDataRequest() )
	{
		ata_write16( ATA_DATA_REG, source );
		return true;
	}
	return false;
}


bool atapi_readPacketWord( uint16_t * destination )
{
	if( atapi_waitDataRequest() )
	{
		*((uint16_t*)destination) = ata_read16( ATA_DATA_REG );
		return true;
	}
	return false;
}


bool atapi_readPacketWordSkip( void )
{
	if( atapi_waitDataRequest() )
	{
		ata_read16( ATA_DATA_REG );
		return true;
	}
	return false;
}


bool atapi_writePacket( const uint8_t * source, uint16_t byteCount )
{
	for( uint16_t i = 0; i<(byteCount>>1); i++ )
	{
		if( atapi_waitDataRequest() )
		{
			ata_write16( ATA_DATA_REG, ((uint16_t*)source)[i] );
		}
		else
		{
			return false;
		}
	}
	return true;
}


bool atapi_readPacket( uint8_t * destination, uint16_t byteCount )
{
	for( uint16_t i = 0; i<(byteCount>>1); i++ )
	{
		if( atapi_waitDataRequest() )
		{
			((uint16_t*)destination)[i] = ata_read16( ATA_DATA_REG );
		}
		else
		{
			return false;
		}
	}
	return true;
}


bool atapi_readPacketSkip( uint16_t byteCount )
{
	for( uint16_t i = 0; i<(byteCount>>1); i++ )
	{
		if( atapi_waitDataRequest() )
		{
			ata_read16( ATA_DATA_REG );
		}
		else
		{
			return false;
		}
	}
	return true;
}


bool atapi_readPacketString( char * destination, uint8_t charCount )
{
	for( uint16_t i = 0; i<(charCount>>1); i++ )
	{
		if( atapi_waitDataRequest() )
		{
			uint16_t data = ata_read16( ATA_DATA_REG );
			destination[i<<1] = data>>8;
			destination[(i<<1)+1] = data;
		}
		else
		{
			return false;
		}
	}
	destination[charCount] = 0;
	for( uint8_t i=0; i<=charCount; i++ )
	{
		if( destination[charCount-i] <= 0x20 || destination[charCount-i] >= 0x7F )
			destination[charCount-i] = 0;
		else
			break;
	}
	return true;
}


bool atapi_writeCommandPacket( const uint8_t command[12], uint16_t ioLength )
{
	ioLength += 12;
	if( !ata_waitNotBusy() )
		return false;

	ata_write8( ATA_FEATURES_REG, 0 );
	ata_write8( ATAPI_BYTECOUNTLOW_REG, ioLength );
	ata_write8( ATAPI_BYTECOUNTHIGH_REG, ioLength>>8 );
	ata_write8( ATA_COMMAND_REG, ATA_COMMAND_PACKET );

	atapi_writePacket( command, 12 );
	return true;
}


bool atapi_requestSense( atapi_requestSense_t * sense )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_REQUESTSENSE,
		0,0,0,		// reserved
		14,	// allocarion length
		0,0,0,0,0,0,0	// reserved
	};
	if( !atapi_writeCommandPacket( packet, 14 ) )
		return false;

	uint8_t data[14];
	if( !atapi_readPacket( data, 14 ) )
		return false;

	sense->errorCode = data[0] & 0x7F;
	sense->senseKey = data[2] & 0x0F;
	sense->additionalSenseCode = data[12];
	sense->additionalSenseCodeQualifier = data[13];
	return true;
}


////////////////////////////////////////////////////////////////
// atapi_readTOC_*

bool atapi_readTOC_initiate( int8_t * numTracks, int8_t * firstTrack, bool msf )
{
	uint16_t ioLength = ATAPI_MAX_TRACKS * 8 + 4;	// 4Bytes Header + TrackDescriptor each 8Bytes
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_READTOC,
		msf?0x02:0x00,	// Bit 1: MSF, others reserved
		0,		// Bits 0 - 2: format, others reserved
		0,	// reserved
		0,	// reserved
		0,	// reserved
		0,	// starting track / session number
		ioLength>>8,	// allocation length: MSB
		ioLength,	// allocation length: LSB
		0,	// Bits 6 & 7: Format, others reserved
		0,	// reserved
		0	// reserved
	};
	if( !atapi_writeCommandPacket( packet, ioLength ) )
		return false;

	if( !atapi_readPacketWordSkip() )	// TOC data length
		return false;

	uint16_t data;
	if( !atapi_readPacketWord( &data ) )	// first and last track number
		return false;
	(*firstTrack) = data;			// fist track in low-byte ( 1 to 99 )
	int8_t lastTrack = data >> 8;		// last track in high-byte ( excluding lead-out )
	lastTrack++;				// last track including lead-out

	(*numTracks) = lastTrack - (*firstTrack) + 1;

	return true;
}

bool atapi_readTOC_MSF_receive( atapi_track_MSF_t * tracks, int8_t numTracks )
{
	uint16_t data;
	for( int8_t i=0; i<numTracks; i++ )
	{
		if( !atapi_readPacketWord( &data ) )	// low-byte reserved, high-byte ADRControl
			return false;
		tracks[i].qADRControl = data >> 8;

		if( !atapi_readPacketWordSkip() )	// skip track number
			return false;

		if( !atapi_readPacketWord( &data ) )	// low-byte reserved, high-byte minutes
			return false;
		tracks[i].address.minutes = data >> 8;

		if( !atapi_readPacketWord( &data ) )
			return false;
		tracks[i].address.seconds = data;
		tracks[i].address.frames = data >> 8;
	}
	return true;
}

#if ATAPI_USE_LBA
bool atapi_readTOC_LBA_receive( atapi_track_LBA_t * tracks, int8_t numTracks )
{
	uint16_t data;
	for( int8_t i=0; i<numTracks; i++ )
	{
		if( !atapi_readPacketWord( &data ) )	// low-byte reserved, high-byte ADRControl
			return false;
		tracks[i].qADRControl = data >> 8;

		if( !atapi_readPacketWordSkip() )	// skip track number
			return false;

		if( !atapi_readPacketWord( &data ) )	// LBA MSB
			return false;
		tracks[i].address = ((uint32_t)data) << 16;

		if( !atapi_readPacketWord( &data ) )	// LBA LSB
			return false;
		tracks[i].address |= data;
	}
	return true;
}
#endif

#if ATAPI_USE_MALLOC
atapi_track_MSF_t * atapi_readTOC_MSF_malloc( int8_t * numTracks, int8_t * firstTrack )
{
	if( !atapi_readTOC_initiate( numTracks, firstTrack, true ) )
		return NULL;

	atapi_track_MSF_t * tracks = malloc( (*numTracks) * sizeof(atapi_track_MSF_t) );
	if( !tracks )
		return NULL;

	if( !atapi_readTOC_MSF_receive( tracks, *numTracks ) )
	{
		free( tracks );
		return NULL;
	}

	return tracks;
}
#if ATAPI_USE_LBA
atapi_track_LBA_t * atapi_readTOC_LBA_malloc( int8_t * numTracks, int8_t * firstTrack )
{
	if( !atapi_readTOC_initiate( numTracks, firstTrack, false ) )
		return NULL;

	atapi_track_LBA_t * tracks = malloc( (*numTracks) * sizeof(atapi_track_LBA_t) );
	if( !tracks )
		return NULL;

	if( !atapi_readTOC_LBA_receive( tracks, *numTracks ) )
	{
		free( tracks );
		return NULL;
	}

	return tracks;
}
#endif // ATAPI_USE_LBA
#endif // ATAPI_USE_MALLOC

#if ATAPI_USE_NON_MALLOC
bool atapi_readTOC_MSF( atapi_track_MSF_t * tracks, int8_t * numTracks, int8_t * firstTrack )
{
	int8_t maxTracks = (*numTracks);

	if( !atapi_readTOC_initiate( numTracks, firstTrack, true ) )
		return false;

	if( (*numTracks) > maxTracks )	// if not all tracks can be stored, abort
		return false;

	return atapi_readTOC_MSF_receive( tracks, *numTracks );
}
#if ATAPI_USE_LBA
bool atapi_readTOC_LBA( atapi_track_LBA_t * tracks, int8_t * numTracks, int8_t * firstTrack )
{
	int8_t maxTracks = (*numTracks);

	if( !atapi_readTOC_initiate( numTracks, firstTrack, false ) )
		return false;

	if( (*numTracks) > maxTracks )	// if not all tracks can be stored, abort
		return false;

	return atapi_readTOC_LBA_receive( tracks, *numTracks );
}
#endif // ATAPI_USE_LBA
#endif // ATAPI_USE_MALLOC

////////////////////////////////////////////////////////////////


bool atapi_startStopUnit( uint8_t operation )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_STARTSTOPUNIT,
		0x00,	// Bit 0: immediate, others reserved
		0,	// reserved
		0,	// reserved
		operation & 0x03,	// Bit 0: start, Bit 1: load/eject, others reserved
		0,0,0,0,0,0,0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}


////////////////////////////////////////////////////////////////
// atapi_readSubChannel_*

bool atapi_readSubChannel_currentPosition_MSF( atapi_readSubChannel_currentPosition_MSF_t * current )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_READSUBCHANNEL,
		0x02,	// Bit 1: MSF, others reserved
		0x40,	// Bit 6: SubQ, others reserved
		0x01,	// subchannel data format (current position)
		0,	// reserved
		0,	// reserved
		0,	// track number
		0,	// allocation length: MSB
		16,	// allocation length: LSB
		0,0,0	// reserved
	};
	if( !atapi_writeCommandPacket( packet, 16 ) )
		return false;

	uint8_t data[16];
	if( !atapi_readPacket( data, 16 ) )
		return false;

	current->audioStatus = data[1];
	current->qADRControl = data[5];
	current->track = data[6];
	current->index = data[7];
	current->absolute.minutes = data[9];
	current->absolute.seconds = data[10];
	current->absolute.frames = data[11];
	current->relative.minutes = data[13];
	current->relative.seconds = data[14];
	current->relative.frames = data[15];
	return true;
}

#if ATAPI_USE_LBA
bool atapi_readSubChannel_currentPosition_LBA( atapi_readSubChannel_currentPosition_LBA_t * current )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_READSUBCHANNEL,
		0x00,	// Bit 1: MSF, others reserved
		0x40,	// Bit 6: SubQ, others reserved
		0x01,	// subchannel data format (current position)
		0,	// reserved
		0,	// reserved
		0,	// track number
		0,	// allocation length: MSB
		16,	// allocation length: LSB
		0,0,0	// reserved
	};
	if( !atapi_writeCommandPacket( packet, 16 ) )
		return false;

	uint8_t data[16];
	if( !atapi_readPacket( data, 16 ) )
		return false;

	current->audioStatus = data[1];
	current->qADRControl = data[5];
	current->track = data[6];
	current->index = data[7];
	current->absolute = ((uint32_t*)data)[2];
	current->relative = ((uint32_t*)data)[3];
	return true;
}
#endif // ATAPI_USE_LBA

////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
// atapi_playAudio_*

bool atapi_playAudio_MSF( const atapi_msf_t * start, const atapi_msf_t * end )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_PLAYAUDIOMSF,
		0,	// reserved
		0,	// reserved
		start->minutes,	// MSF starting address
		start->seconds,	// MSF starting address
		start->frames,	// MSF starting address
		end->minutes,	// MSF ending address
		end->seconds,	// MSF ending address
		end->frames,	// MSF ending address
		0,0,0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}

#if ATAPI_USE_LBA
bool atapi_playAudio_LBA( const atapi_lba_t * start, uint16_t length )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_PLAYAUDIO,
		0,	// reserved
		(*start)>>24,	// LBA address
		(*start)>>16,	// LBA address
		(*start)>>8,	// LBA address
		(*start),	// LBA address
		0,	// reserved
		length>>8,	// transfer length
		length,		// transfer length
		0,0,0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}
#endif // ATAPI_USE_LBA

////////////////////////////////////////////////////////////////


bool atapi_stop( void )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_STOP,
		0,0,0,0,0,0,0,0,0,0,0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}


bool atapi_pauseResume( bool resume )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_PAUSERESUME,
		0,0,0,0,0,0,0,	// reserved
		resume?0x01:0x00,	// bit 0: resume, others reserved
		0,0,0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}

bool atapi_testUnitReady(void)
{
	uint8_t packet[12] =
		{
			ATAPI_COMMAND_TESTUNITREADY,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // reserved
		};
	if (!atapi_writeCommandPacket(packet, 0))
		return false;

	ata_waitNotBusy();
	return !ata_read8(ATA_ERROR_REG);
}

////////////////////////////////////////////////////////////////
// atapi_scan_*

bool atapi_scan_track( uint8_t startTrack, bool reverse )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_SCAN,
		reverse?0x10:0x00,	// Bit 4: direction, others reserved
		0,		// TNO address: reserved
		0,		// TNO address: reserved
		0,		// TNO address: reserved
		startTrack,	// TNO address: track number
		0,	// reserved
		0,	// reserved
		0,	// reserved
		0x80,	// Bit 6 & 7: type (TNO)
		0,	// reserved
		0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}

bool atapi_scan_MSF( atapi_msf_t * start, bool reverse )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_SCAN,
		reverse?0x10:0x00,	// Bit 4: direction, others reserved
		0,		// MSF address: reserved
		start->minutes,	// MSF address
		start->seconds,	// MSF address
		start->frames,	// MSF address
		0,	// reserved
		0,	// reserved
		0,	// reserved
		0x40,	// Bit 6 & 7: type (MSF)
		0,	// reserved
		0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}

#if ATAPI_USE_LBA
bool atapi_scan_LBA( atapi_lba_t * start, bool reverse )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_SCAN,
		reverse?0x10:0x00,	// Bit 4: direction, others reserved
		(*start)>>24,	// LBA address
		(*start)>>16,	// LBA address
		(*start)>>8,	// LBA address
		(*start),	// LBA address
		0,	// reserved
		0,	// reserved
		0,	// reserved
		0x00,	// Bit 6 & 7: type (LBA)
		0,	// reserved
		0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}
#endif // ATAPI_USE_LBA

////////////////////////////////////////////////////////////////


#if ATAPI_USE_LBA
bool atapi_seek( atapi_lba_t * address )
{
	uint8_t packet[12] =
	{
		ATAPI_COMMAND_SEEK,
		0,	// reserved
		(*address)>>24,	// LBA address
		(*address)>>16,	// LBA address
		(*address)>>8,	// LBA address
		(*address),	// LBA address
		0,0,0,0,0,0	// reserved
	};
	return atapi_writeCommandPacket( packet, 0 );
}
#endif // ATAPI_USE_LBA


bool atapi_printError( void )
{
	static bool recursionStopper = false;
	if( !recursionStopper )
	{
		recursionStopper = true;
		atapi_requestSense_t sense;
		if( atapi_requestSense( &sense ) )
		{
			printf( "ErrorCode 0x%02X  SenseKey 0x%02X  ASC 0x%02X  ASCQ 0x%02X\n", sense.errorCode, sense.senseKey, sense.additionalSenseCode, sense.additionalSenseCodeQualifier );
		}
		recursionStopper = false;
		return true;
	}
	return false;
}


bool atapi_identifyPacketDevice( atapi_device_information_t * info )
{
	ata_write8( ATA_FEATURES_REG, 0 );
	ata_write8( ATAPI_BYTECOUNTLOW_REG, 0x00 );
	ata_write8( ATAPI_BYTECOUNTHIGH_REG, 0x01 );
	ata_write8( ATA_COMMAND_REG, ATA_COMMAND_IDENTIFYPACKETDEVICE );

	uint16_t data = 0;

	atapi_readPacketWord( &(info->generalConfig) );			// word 0: general configuration
	atapi_readPacketSkip( 18 );					// words 1 to 9: reserved
	atapi_readPacketString( info->serialNumber,
		ATA_IDENTIFYPACKETDEVICE_SERIALNUMBER_LEN );		// words 10 to 19: serial number
	atapi_readPacketSkip( 6 );					// words 20 to 22: reserved
	atapi_readPacketString( info->firmwareRev,
		ATA_IDENTIFYPACKETDEVICE_FIRMWAREREVISION_LEN );	// words 23 to 26: firmware revision
	atapi_readPacketString( info->modelNr,
		ATA_IDENTIFYPACKETDEVICE_MODELNUMBER_LEN );		// words 27 to 46: model number
	atapi_readPacketSkip( 4 );					// words 47 to 48: reserved
	atapi_readPacketWord( &data );					// word 49: capabilities
	info->capabilities=data>>8;
	atapi_readPacketWordSkip();					// word 50: reserved
	atapi_readPacketWord( &data );					// word 51: PIO mode + vendor specific
	info->pioModeNr=data>>8;
	atapi_readPacketWordSkip();					// word 52: reserved

	atapi_readPacketSkip( 406 );				// words 53 to 255: too lazy

	atapi_waitNoDataRequest();

	return true;
}


static bool atapi_isValidDevice( void )
{
	uint16_t cylinders = ata_read8( ATA_CYLINDERLOW_REG ) | ( ata_read8( ATA_CYLINDERHIGH_REG ) << 8 );
	return cylinders == ATAPI_MAGICNUMBER;
}


static bool atapi_initDevice( void )
{
	if( !atapi_isValidDevice() )
	{
		printf( "No device detected\n" );
		return false;
	}

	atapi_device_information_t info;
	if( !atapi_identifyPacketDevice( &info ) )
	{
		printf( "Could not identify device\n" );
		return false;
	}

	printf( "Device Information:\n" );
	printf( " * Configuration  0x%04X\n", info.generalConfig );
	printf( " * Serial Nr.    \"%s\"\n", info.serialNumber );
	printf( " * Firmware Rev. \"%s\"\n", info.firmwareRev );
	printf( " * Model Nr.     \"%s\"\n", info.modelNr );
	printf( " * Capabilities   0x%02X\n", info.capabilities );
	printf( " * PIO Mode Nr.   0x%02X\n", info.pioModeNr );

	display_log("Device info:", info.modelNr);
	return true;
}



bool atapi_init( void )
{
	bool foundMaster;
	//ata_selectDevice( 0 );

	ata_write8(ATAPI_DRIVESELECT, 0xE0);
	printf( "ATAPI Master: " );
	foundMaster = atapi_initDevice();

	/*ata_selectDevice( 1 );
	printf( "ATAPI Slave: " );
	if( atapi_initDevice() )
		return true;

	if( foundMaster )
		ata_selectDevice( 0 );
*/
	return foundMaster;
}
