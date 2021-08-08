/*
 * Copyright (c) 2021 Tobias Mädel <t.maedel@alfeld.de>
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
 * \defgroup ATAPI ATAPI-Interface
 * \brief ATAPI-Interface based on the ATA-Interface.
 *
 * Implements an ATAPI-Driver.
 *
 * @{
 **/


#ifndef __ATAPI_H__INCLUDED__
#define __ATAPI_H__INCLUDED__


#include "atapi_config.h"
#include "ata.h"
#include "config.h"

#if ATAPI_USE_MALLOC == 0 && ATAPI_USE_NON_MALLOC == 0
#error Neither ATAPI_USE_MALLOC nor ATAPI_USE_NON_MALLOC is used!
#endif

#define _BV( bit ) ( 1<<(bit) ) 

#define ATA_COMMAND_NOP				0x00
#define ATA_COMMAND_DEVICERESET			0x08
#define ATA_COMMAND_DOORLOCK			0xDE
#define ATA_COMMAND_DOORUNLOCK			0xDF
#define ATA_COMMAND_STANDBYIMMEDIATE		0xE0
#define ATA_COMMAND_MEDIAEJECT			0xED
#define ATA_COMMAND_IDENTIFYDRIVE		0xEC
#define ATA_COMMAND_PACKET			0xA0
#define ATA_COMMAND_IDENTIFYPACKETDEVICE	0xA1
#define ATA_IDENTIFYPACKETDEVICE_SERIALNUMBER_LEN	20
#define ATA_IDENTIFYPACKETDEVICE_FIRMWAREREVISION_LEN	8
#define ATA_IDENTIFYPACKETDEVICE_MODELNUMBER_LEN	40


// register bit definitions
#define ATA_STATUS_BUSY			_BV(7)
#define ATA_STATUS_DEVICEREADY		_BV(6)
#define ATA_STATUS_DATAREQUEST		_BV(3)
#define ATA_STATUS_ERROR		_BV(0)
#define ATA_ERROR_ABORT			_BV(2)
#define ATA_DEVICEHEAD_DEVICE		_BV(4)
#define ATA_DEVICECONTROL_IEN		_BV(1)
#define ATA_DEVICECONTROL_RESET		_BV(2)

////////////////////////////////////////////////////////////////
// register address definitions based upon the ATA/ATAPI-4 Working Draft

#define ATAPI_ALTERNATESTATUS_REG	0
#define ATAPI_DEVICECONTROL_REG		0
#define ATAPI_STATUS_REG			7
#define ATAPI_ERROR_REG				1
#define ATAPI_FEATURES_REG			1
#define ATAPI_DRIVESELECT			6
#define ATAPI_BYTECOUNTHIGH_REG		5
#define ATAPI_BYTECOUNTLOW_REG		4
#define ATAPI_INTERRUPTREASON_REG	2

#define ATA_DATA_REG		0

//	0	1	1	1	0	R
#define ATA_ALTERNATESTATUS_REG	0
//						W
#define ATA_DEVICECONTROL_REG	0

//	1	1	1	0	1	W
#define ATA_COMMAND_REG		7
//						R
#define ATA_STATUS_REG		7

//	1	0	0	0	1	R
#define ATA_ERROR_REG		1
//						W
#define ATA_FEATURES_REG	ATA_ERROR_REG

//	0	1	1	0	1	RW
#define ATA_DEVICEHEAD_REG	6

//	1	0	1	0	1	RW
#define ATA_CYLINDERHIGH_REG	5

//	0	0	1	0	1	RW
#define ATA_CYLINDERLOW_REG	4

//	0	1	0	0	1	RW
#define ATA_SECTORCOUNT_REG	2

//	1	1	0	0	1	RW
#define ATA_SECTORNUMBER_REG	3



////////////////////////////////////////////////////////////////
// register bit definitions

/// Initial cylindercount value on atapi devices.
#define ATAPI_MAGICNUMBER		0xEB14

#define ATAPI_STATUS_BUSY		ATA_STATUS_BUSY
#define ATAPI_STATUS_DEVICEREADY	ATA_STATUS_DEVICEREADY
#define ATAPI_STATUS_DATAREQUEST	ATA_STATUS_DATAREQUEST
#define ATAPI_STATUS_CORRECTABLE	_BV(2)
#define ATAPI_STATUS_ERROR		ATA_STATUS_ERROR

#define ATAPI_ERROR_LENGTH		_BV(0)
#define ATAPI_ERROR_EOM			_BV(1)
#define ATAPI_ERROR_ABORT		ATA_ERROR_ABORT
#define ATAPI_ERROR_MEDIACHANGEREQ	_BV(3)

#define ATAPI_DEVICEHEAD_DRIVE		ATA_DEVICEHEAD_DEVICE


////////////////////////////////////////////////////////////////
// ATAPI Commands

#define ATAPI_COMMAND_STARTSTOPUNIT	0x1B
#define ATAPI_STARTSTOPUNIT_START	_BV(0)
#define ATAPI_STARTSTOPUNIT_LOADEJECT	_BV(1)

#define ATAPI_COMMAND_REQUESTSENSE	0x03
#define ATAPI_SENSE_NONE		0x00
#define ATAPI_SENSE_RECOVEREDERROR	0x01
#define ATAPI_SENSE_NOTREADY		0x02
#define ATAPI_SENSE_MEDIUMERROR		0x03
#define ATAPI_SENSE_HARDWAREERROR	0x04
#define ATAPI_SENSE_ILLEGALREQUEST	0x05
#define ATAPI_SENSE_UNITATTENTION	0x06
#define ATAPI_SENSE_DATAPROTECT		0x07
#define ATAPI_SENSE_ABORTEDCOMMAND	0x0B
#define ATAPI_SENSE_MISCOMPARE		0x0E
#define ATAPI_ASC_LOGICALDRIVENOTREADY	0x04
#define ATAPI_ASC_NOREFERENCEPOSITION	0x06
#define ATAPI_ASC_INVALIDCOMMAND	0x20
#define ATAPI_ASC_INVALIDFIELD		0x24
#define ATAPI_ASC_CANNOTREADMEDIUM	0x30
#define ATAPI_ASC_MEDIUMNOTPRESENT	0x3A
#define ATAPI_ASC_PLAYOPERATIONABORTED	0xB9
#define ATAPI_ASC_LOSSOFSTREAMING	0xBF

#define ATAPI_COMMAND_READSUBCHANNEL	0x42
#define ATAPI_READSUBCHANNEL_DATAFORMAT_CURRENTPOSITION	0x01
#define ATAPI_READSUBCHANNEL_DATAFORMAT_UPC		0x02
#define ATAPI_READSUBCHANNEL_DATAFORMAT_ISRC		0x03
#define ATAPI_READSUBCHANNEL_AUDIOSTATUS_NOTSUPPORTED	0x00
#define ATAPI_READSUBCHANNEL_AUDIOSTATUS_PLAYING	0x11
#define ATAPI_READSUBCHANNEL_AUDIOSTATUS_PAUSED		0x12
#define ATAPI_READSUBCHANNEL_AUDIOSTATUS_COMPLETED	0x13
#define ATAPI_READSUBCHANNEL_AUDIOSTATUS_ERROR		0x14
#define ATAPI_READSUBCHANNEL_AUDIOSTATUS_NOTPLAYING	0x15
#define ATAPI_QFIELDADR_NOTSUPPLIED		0x00
#define ATAPI_QFIELDADR_CURRENTPOSITION		0x10
#define ATAPI_QFIELDADR_UPC			0x20
#define ATAPI_QFIELDADR_ISRC			0x30
#define ATAPI_QFIELDCONTROL_PREEMPHASIS		_BV(0)
#define ATAPI_QFIELDCONTROL_COPYPERMITTED	_BV(1)
#define ATAPI_QFIELDCONTROL_DATATRACK		_BV(2)
#define ATAPI_QFIELDCONTROL_FOURCHANNELAUDIO	_BV(3)

#define ATAPI_COMMAND_READTOC			0x43
#define ATAPI_COMMAND_PLAYAUDIO			0x45
#define ATAPI_COMMAND_PLAYAUDIOMSF		0x47
#define ATAPI_COMMAND_PAUSERESUME		0x4B
#define ATAPI_COMMAND_STOP			0x4E
#define ATAPI_COMMAND_SCAN			0xBA
#define ATAPI_COMMAND_SEEK			0x2B
#define ATAPI_COMMAND_TESTUNITREADY		0x00


////////////////////////////////////////////////////////////////
// other stuff

/// ATAPI spec. defines a maximum of 99 tracks + 1 lead-out.
#define ATAPI_MAX_TRACKS			100

#define ATAPI_WAITDATAREQUEST_DEFAULT_TIMEOUT	128


////////////////////////////////////////////////////////////////
// data types

#if ATAPI_USE_LBA
typedef uint32_t atapi_lba_t;	///< Logical Block Address Type.
#endif

/// Status of a play operation.
/**
* This field has one of the following values:
* - \e ATAPI_READSUBCHANNEL_AUDIOSTATUS_NOTSUPPORTED
* - \e ATAPI_READSUBCHANNEL_AUDIOSTATUS_PLAYING
* - \e ATAPI_READSUBCHANNEL_AUDIOSTATUS_PAUSED
* - \e ATAPI_READSUBCHANNEL_AUDIOSTATUS_COMPLETED
* - \e ATAPI_READSUBCHANNEL_AUDIOSTATUS_ERROR
* - \e ATAPI_READSUBCHANNEL_AUDIOSTATUS_NOTPLAYING
* .
**/
typedef uint8_t atapi_audioStatus_t;

/// ADR and control information.
/**
* The control information is in the low-nibble (bits 0-3). This may be any bit-combination of:
* - \e ATAPI_QFIELDCONTROL_PREEMPHASIS
* - \e ATAPI_QFIELDCONTROL_COPYPERMITTED
* - \e ATAPI_QFIELDCONTROL_DATATRACK
* - \e ATAPI_QFIELDCONTROL_FOURCHANNELAUDIO
* .
* The ADR code is in the high-nibble (bits 4-7) and has one of the following values:
* - \e ATAPI_QFIELDADR_NOTSUPPLIED
* - \e ATAPI_QFIELDADR_CURRENTPOSITION
* - \e ATAPI_QFIELDADR_UPC
* - \e ATAPI_QFIELDADR_ISRC
* .
**/
typedef uint8_t atapi_qADRControl_t;

/// Address in minutes, seconds and frames.
typedef struct
{
	uint8_t minutes;	///< Minutes from 0 to 99.
	uint8_t seconds;	///< Seconds from 0 to 59.
	uint8_t frames;		///< Frames from 0 to 74.
} atapi_msf_t;

/// Detailed error information.
typedef struct
{
	uint8_t errorCode;	///< 0x70 (current), 0x71 (deferred), 0x7F (vendor).
	uint8_t senseKey;	///< 0x00 for a successful command, other values for indicate an error.
	uint8_t additionalSenseCode;	///< ASC - More detailed information.
	uint8_t additionalSenseCodeQualifier;	///< ASCQ Even more details.
} atapi_requestSense_t;

/// Subchannel data in MSF address format
typedef struct
{
	atapi_audioStatus_t	audioStatus;	///< Current audio status information.
	atapi_qADRControl_t	qADRControl;	///< Current ADR and control information.
	int8_t		track;		///< Current track number from 1 to 99.
	int8_t		index;		///< Index number in the current track.
	atapi_msf_t	absolute;	///< Absolute position on CD.
	atapi_msf_t	relative;	///< Relative position from beginning of the track.
} atapi_readSubChannel_currentPosition_MSF_t;

#if ATAPI_USE_LBA
/// Subchannel data in LBA address format
typedef struct
{
	atapi_audioStatus_t	audioStatus;	///< Current audio status information.
	atapi_qADRControl_t	qADRControl;	///< Current ADR and control information.
	int8_t		track;		///< Current track number from 1 to 99.
	int8_t		index;		///< Index number in the current track.
	atapi_lba_t	absolute;	///< Absolute position on CD.
	atapi_lba_t	relative;	///< Relative position from beginning of the track.
} atapi_readSubChannel_currentPosition_LBA_t;
#endif

/// Drive identification information
typedef struct
{
	uint16_t generalConfig;
	char serialNumber[ATA_IDENTIFYPACKETDEVICE_SERIALNUMBER_LEN+1];
	char firmwareRev[ATA_IDENTIFYPACKETDEVICE_FIRMWAREREVISION_LEN+1];
	char modelNr[ATA_IDENTIFYPACKETDEVICE_MODELNUMBER_LEN+1];
	uint8_t capabilities;
	uint8_t pioModeNr;
} atapi_device_information_t;

/// Track information in MSF format
typedef struct
{
	atapi_qADRControl_t qADRControl;	///< ADR and control information for this track.
	atapi_msf_t address;			///< Starting address of this track.
} atapi_track_MSF_t;

#if ATAPI_USE_LBA
/// Track information in LBA format
typedef struct
{
	atapi_qADRControl_t qADRControl;	///< ADR and control information for this track.
	atapi_lba_t address;			///< Starting address of this track.
} atapi_track_LBA_t;
#endif


////////////////////////////////////////////////////////////////
// functions

/// Tries to initialize any connected ATAPI device.
bool atapi_init( void );

/// Prints current error/sense information from device.
bool atapi_printError( void );

/// Reads information from device.
bool atapi_identifyPacketDevice( atapi_device_information_t * info );

/// Reads current error/sense information.
bool atapi_requestSense( atapi_requestSense_t * sense );

/// Returns \e true if current device has loaded and initialized the medium.
bool atapi_testUnitReady( void );

/// Executes a start/stop/eject/load operation on device.
bool atapi_startStopUnit( uint8_t operation );

/// Pause or resume a play operation.
bool atapi_pauseResume( bool resume );

/// Stop a play/scan operation.
bool atapi_stop( void );

/// Fast-forward or fast-reverse operation.
bool atapi_scan_track( uint8_t startTrack, bool reverse );

/// Fast-forward or fast-reverse operation.
bool atapi_scan_MSF( atapi_msf_t * start, bool reverse );

/// Extracts sub-channel information of current track.
bool atapi_readSubChannel_currentPosition_MSF( atapi_readSubChannel_currentPosition_MSF_t * current );

/// Starts audio play operation.
bool atapi_playAudio_MSF( const atapi_msf_t * start, const atapi_msf_t * end );

#if ATAPI_USE_LBA
/// Fast-forward or fast-reverse operation.
bool atapi_scan_LBA( atapi_lba_t * start, bool reverse );

/// Extracts sub-channel information of current track.
bool atapi_readSubChannel_currentPosition_LBA( atapi_readSubChannel_currentPosition_LBA_t * current );

/// Starts audio play operation.
bool atapi_playAudio_LBA( const atapi_lba_t * start, uint16_t length );

/// Change position to a logical block address.
bool atapi_seek( atapi_lba_t * address );
#endif // ATAPI_USE_LBA

bool ata_waitNotBusy( void );

#if ATAPI_USE_MALLOC
/// Reads the Table Of Contents from the current CD using MSF addresses. (\e malloc version)
/**
 * This function will dynamically allocate memory for all tracks on the CD.
 * If returned pointer is not \e NULL, \e numtracks and \e firstTrack contain valid data, and the returned pointer to the allocated memory must be freed manually.
 * \param numTracks Must point to a variable. If this function returns successful, \e numTracks is set to the actual number of tracks.
 * \param firstTrack Must point to a variable. On successful execution, this variable contains the tracknumber of the first track in \e tracks.
 * \returns The pointer to the allocated memory that contains the tracklist, or \e NULL.
 **/
atapi_track_MSF_t * atapi_readTOC_MSF_malloc( int8_t * numTracks, int8_t * firstTrack );
#if ATAPI_USE_LBA
/// Reads the Table Of Contents from the current CD using LBA addresses. (\e malloc version)
/**
 * This function will dynamically allocate memory for all tracks on the CD.
 * If returned pointer is not \e NULL, \e numtracks and \e firstTrack contain valid data, and the returned pointer to the allocated memory must be freed manually.
 * \param numTracks Must point to a variable. If this function returns successful, \e numTracks is set to the actual number of tracks.
 * \param firstTrack Must point to a variable. On successful execution, this variable contains the tracknumber of the first track in \e tracks.
 * \returns The pointer to the allocated memory that contains the tracklist, or \e NULL.
 **/
atapi_track_LBA_t * atapi_readTOC_LBA_malloc( int8_t * numTracks, int8_t * firstTrack );
#endif // ATAPI_USE_LBA
#endif // ATAPI_USE_NON_MALLOC

#if ATAPI_USE_NON_MALLOC
/// Reads the Table Of Contents from the current CD using MSF addresses. (non \e malloc version)
/**
 * \param tracks A pointer to an array of \e numTracks atapi_track_MSF_t elements.
 * \param numTracks Must point to a variable containing the number of elements in \e tracks.
 * If this function returns successful, \e numTracks will be set to the actual number of tracks.
 * \param firstTrack Must point to a variable. On successful execution, this variable contains the tracknumber of the first track in \e tracks.
 * \returns \e true on success.
 **/
bool atapi_readTOC_MSF( atapi_track_MSF_t * tracks, int8_t * numTracks, int8_t * firstTrack );
#if ATAPI_USE_LBA
/// Reads the Table Of Contents from the current CD using LBA addresses. (non \e malloc version)
/**
 * \param tracks A pointer to an array of \e numTracks atapi_track_MSF_t elements.
 * \param numTracks Must point to a variable containing the number of elements in \e tracks.
 * If this function returns successful, \e numTracks will be set to the actual number of tracks.
 * \param firstTrack Must point to a variable. On successful execution, this variable contains the tracknumber of the first track in \e tracks.
 * \returns \e true on success.
 **/
bool atapi_readTOC_LBA( atapi_track_LBA_t * tracks, int8_t * numTracks, int8_t * firstTrack );
#endif // ATAPI_USE_LBA
#endif // ATAPI_USE_NON_MALLOC


#endif


/** @} **/
