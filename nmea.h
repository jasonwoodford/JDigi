/*
 * nmea.h
 * 
 * Copyright (c) 2018, Jason Woodford, VO1JWW. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. No personal names or organizations' names associated with the
 *    Atomthreads project may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE JDIGI PROJECT AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: nmea.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup nmea <nmea.h>: NMEA
 * \code #include <nmea.h> \endcode
 * <h3>Introduction to the NMEA Module</h3>
 */

#ifndef __NMEA_H__
#define __NMEA_H__

#include "atom.h"
#include "aprs.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** \ingroup nmea */
/* @{ */
#define NMEA_STR_MIN    4   ///< Minimum NMEA sentence length w/o CRLF.
#define NMEA_STR_MAX    80  ///< Maximum NMEA sentence length w/o CRLF.
#define NMEA_STR_FIELDS 32  ///< Maximum fields per NMEA sentence.

#define NMEA_SIG_BAD    0
#define NMEA_SIG_LOW    1
#define NMEA_SIG_MID    2
#define NMEA_SIG_HIGH   3

#define NMEA_FIX_BAD    1
#define NMEA_FIX_2D     2
#define NMEA_FIX_3D     3

#define NMEA_MAXSAT     12
#define NMEA_SATINPACK  4
#define NMEA_NSATPACKS  (NMEA_MAXSAT / NMEA_SATINPACK)

#define NMEA_CONV_YARD  1.0936  ///< Yard per metre.
#define NMEA_CONV_KNOT  1.852   ///< Knot per kilometre.
#define NMEA_CONV_MILE  1.609   ///< Mile per kilometre.

#define NMEA_POS_FREQ   15  ///< seconds.
#define NMEA_TIME_FREQ  30  ///< seconds.
#define NMEA_TIME_DRIFT 10  ///< seconds.

#define NMEA_RXFLAG_CR      1
#define NMEA_RXFLAG_CRLF    2
#define NMEA_RXFLAG_RESET   3

#define NMEA_PARSERR_SIZE   (-1)
#define NMEA_PARSERR_START  (-2)
#define NMEA_PARSERR_CRCR   (-3)
#define NMEA_PARSERR_CRCC   (-4)
#define NMEA_PARSERR_FIELDS (-5)
#define NMEA_PARSERR_TIME   (-6)

#define NMEA_PARSERR_GPGGA  (-8)
#define NMEA_PARSERR_GPGSA  (-9)
#define NMEA_PARSERR_GPGSV  (-10)
#define NMEA_PARSERR_GPGLL  (-11)
#define NMEA_PARSERR_GPRMC  (-12)
#define NMEA_PARSERR_GPVTG  (-13)
#define NMEA_PARSERR_GPZDA  (-14)

#define NMEA_UNKNOWN_TYPE   (-64)

#define NMEA_VALID_TIME     (1)
#define NMEA_VALID_DATE     (1 << 1)
#define NMEA_VALID_POS      (1 << 2)

#define NMEA_ARGC_GPGGA     14
#define NMEA_ARGC_GPGSA     17
#define NMEA_ARGC_GPGSV_MIN 3
#define NMEA_ARGC_GPGSV_MAX 19
#define NMEA_ARGC_GPGLL_MIN 6
#define NMEA_ARGC_GPGLL_MAX 7
#define NMEA_ARGC_GPRMC     12
#define NMEA_ARGC_GPVTG     9
#define NMEA_ARGC_GPZDA     7

/** Date and time data
 */
typedef struct nmeaTIME {
    uint8_t year;       ///< Years since 1900
    uint8_t mon;        ///< Months since January - [0,11]
    uint8_t day;        ///< Day of the month - [1,31]
    uint8_t hour;       ///< Hours since midnight - [0,23]
    uint8_t min;        ///< Minutes after the hour - [0,59]
    uint8_t sec;        ///< Seconds after the minute - [0,59]
    uint8_t hsec;       ///< Hundredth part of second - [0,99]
    uint8_t flags;
} nmeaTIME;

/** Position data in fractional degrees or radians
 */
typedef struct nmeaPOS {
    double  lat;         ///< Latitude in NDEG - +/-[degree][min].[sec/60]
    double  lon;         ///< Longitude in NDEG - +/-[degree][min].[sec/60]
    uint8_t flags;
} nmeaPOS;

/** Information about satellite
 * @see nmeaSATINFO
 * @see nmeaGPGSV
 */
typedef struct nmeaSATELLITE {
    int     id;         ///< Satellite PRN number
    int     in_use;     ///< Used in position fix
    int     elv;        ///< Elevation in degrees, 90 maximum
    int     azimuth;    ///< Azimuth, degrees from true north, 000 to 359
    int     sig;        ///< Signal, 00-99 dB
    time_t  lastheard;  ///< System time last heard.
} nmeaSATELLITE;

/** Information about all satellites in view
 * @see nmeaINFO
 * @see nmeaGPGSV
 */
typedef struct nmeaSATINFO {
    nmeaSATELLITE sat[NMEA_MAXSAT]; ///< Satellites information
    int     inuse;      ///< Number of satellites in use (not those in view)
    int     inview;     ///< Total number of satellites in view
} nmeaSATINFO;

/** Summary GPS information from all parsed packets,
 * used also for generating NMEA stream
 * @see nmea_parse
 * @see nmea_GPGGAInfo, etc.
 */
typedef struct nmeaINFO {
    nmeaSATINFO satinfo;///< Satellites information
    nmeaTIME utc;       ///< UTC of position
    nmeaPOS  position;  ///< Position
    time_t  ctimer;     ///< Clock update timer
    time_t  ptimer;     ///< Position update timer
    int     sig;        ///< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive)
    int     fix;        ///< Operating mode, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D)
    double  PDOP;       ///< Position Dilution Of Precision
    double  HDOP;       ///< Horizontal Dilution Of Precision
    double  VDOP;       ///< Vertical Dilution Of Precision
    double  elv;        ///< Antenna altitude above/below mean sea level (geoid) in meters
    double  speed;      ///< Speed over the ground in kilometers/hour
    double  direction;  ///< Track angle in degrees True
    double  declination; ///< Magnetic variation degrees (Easterly var. subtracts from true course)
    uint8_t flags;
} nmeaINFO;

/** GGA packet information structure (Global Positioning System Fix Data)
 */
typedef struct nmeaGPGGA {
    nmeaTIME utc;       ///< UTC of position (just time)
	double  lat;        ///< Latitude in NDEG - [degree][min].[sec/60]
    char    ns;         ///< [N]orth or [S]outh
	double  lon;        ///< Longitude in NDEG - [degree][min].[sec/60]
    char    ew;         ///< [E]ast or [W]est
    int     sig;        ///< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive)
	int     satinuse;   ///< Number of satellites in use (not those in view)
    double  HDOP;       ///< Horizontal dilution of precision
    double  elv;        ///< Antenna altitude above/below mean sea level (geoid)
    char    elv_units;  ///< [M]eters (Antenna height unit)
    double  diff;       ///< Geoidal separation (Diff. between WGS-84 earth ellipsoid and mean sea level. '-' = geoid is below WGS-84 ellipsoid)
    char    diff_units; ///< [M]eters (Units of geoidal separation)
    double  dgps_age;   ///< Time in seconds since last DGPS update
    int     dgps_sid;   ///< DGPS station ID number
} nmeaGPGGA;

/** GSA packet information structure (Satellite status)
 */
typedef struct nmeaGPGSA {
    int     sat_prn[NMEA_MAXSAT]; ///< PRNs of satellites used in position fix (null for unused fields)
    char    fix_mode;   ///< Mode (M = Manual, forced to operate in 2D or 3D; A = Automatic, 3D/2D)
    int     fix_type;   ///< Type, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D)
    double  PDOP;       ///< Dilution of precision
    double  HDOP;       ///< Horizontal dilution of precision
    double  VDOP;       ///< Vertical dilution of precision
} nmeaGPGSA;

/** GSV packet information structure (Satellites in view)
 */
typedef struct nmeaGPGSV {
    nmeaSATELLITE sat_data[NMEA_SATINPACK];
    int     pack_count; ///< Total number of messages of this type in this cycle
    int     pack_index; ///< Message number
    int     sat_count;  ///< Total number of satellites in view
} nmeaGPGSV;

/** GLL packet information
 */
typedef struct nmeaGPGLL {
    nmeaTIME utc;       ///< UTC of position
    double  lat;        ///< Latitude in NDEG - [degree][min].[sec/60]
    char    ns;         ///< [N]orth or [S]outh
	double  lon;        ///< Longitude in NDEG - [degree][min].[sec/60]
    char    ew;         ///< [E]ast or [W]est
    char    valid;
} nmeaGPGLL;

/** RMC packet information structure (Recommended Minimum sentence C)
 */
typedef struct nmeaGPRMC {
    nmeaTIME utc;       ///< UTC of position
    char    status;     ///< Status (A = active or V = void)
	double  lat;        ///< Latitude in NDEG - [degree][min].[sec/60]
    char    ns;         ///< [N]orth or [S]outh
	double  lon;        ///< Longitude in NDEG - [degree][min].[sec/60]
    char    ew;         ///< [E]ast or [W]est
    double  speed;      ///< Speed over the ground in knots
    double  direction;  ///< Track angle in degrees True
    double  declination; ///< Magnetic variation degrees (Easterly var. subtracts from true course)
    char    declin_ew;  ///< [E]ast or [W]est
    char    mode;       ///< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator)
} nmeaGPRMC;

/** VTG packet information structure (Track made good and ground speed)
 */
typedef struct nmeaGPVTG {
    double  dir;        ///< True track made good (degrees)
    char    dir_t;      ///< Fixed text 'T' indicates that track made good is relative to true north
    double  dec;        ///< Magnetic track made good
    char    dec_m;      ///< Fixed text 'M'
    double  spn;        ///< Ground speed, knots
    char    spn_n;      ///< Fixed text 'N' indicates that speed over ground is in knots
    double  spk;        ///< Ground speed, kilometers per hour
    char    spk_k;      ///< Fixed text 'K' indicates that speed over ground is in kilometers/hour
} nmeaGPVTG;

/**
 */
int8_t nmeaParseChecksum (char *str, uint8_t *checksum);
int8_t nmeaParseSentence (char *str, uint8_t len, char **argv);
int8_t nmeaParseTime (char *str, nmeaTIME *utc);
int8_t nmeaParseGPGGA (uint8_t argc, char **argv, nmeaGPGGA *gpgga);
int8_t nmeaParseGPGSA (uint8_t argc, char **argv, nmeaGPGSA *gpgsa);
int8_t nmeaParseGPGSV (uint8_t argc, char **argv, nmeaGPGSV *gpgsv);
int8_t nmeaParseGPGLL (uint8_t argc, char **argv, nmeaGPGLL *gpgll);
int8_t nmeaParseGPRMC (uint8_t argc, char **argv, nmeaGPRMC *gprmc);
int8_t nmeaParseGPVTG (uint8_t argc, char **argv, nmeaGPVTG *gpvtg);
int8_t nmeaParseGPZDA (uint8_t argc, char **argv, nmeaTIME *t);

void nmeaInitInfo (void);
void nmeaTimeInfo (nmeaTIME *p, int8_t flags);
void nmeaGPGGAInfo (nmeaGPGGA *p, int8_t flags);
void nmeaGPGSAInfo (nmeaGPGSA *p, int8_t flags);
void nmeaGPGSVInfo (nmeaGPGSV *p, int8_t flags);
void nmeaGPGLLInfo (nmeaGPGLL *p, int8_t flags);
void nmeaGPRMCInfo (nmeaGPRMC *p, int8_t flags);
void nmeaGPVTGInfo (nmeaGPVTG *p, int8_t flags);

void nmeaInfoPosTime (int8_t flags);

char* nmeaPrintError (char *buf, int8_t errno);

/* @} */
#endif /* __NMEA_H__ */
