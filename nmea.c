/*
 * nmea.c
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
 * $Id: nmea.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "nmea.h"

nmeaINFO    nmea_info;
nmeaSATINFO nmea_satinfo;

extern APRSConfig config;
extern APRSPosit position;

/** Parse the checksum field and get the 8-bit value. 
 * @param str: pointer to a two byte ASCII string.
 * @param checksum: container pointer.
 * @return Zero on success or an error.
 */
int8_t nmeaParseChecksum (char *str, uint8_t *checksum) {
    int8_t i;
    char c;
    //
    for (i = 0; i < 2; ++i) {
        *checksum <<= 4;
        c = *(str + i);
        if ((c >= '0') && (c <= '9')) {
            *checksum |= c - '0';
        }
        else if ((c >= 'A') && (c <= 'F')) {
            *checksum |= c + 10 - 'A';
        }
        else {
            return NMEA_PARSERR_CRCR;
        }
    }
    return 0;
}

/** Parse the NMEA sentence and populate argv.
 * @param str: pointer to NMEA sentence.
 * @param len: length of sentence.
 * @param argv: pointer table.
 * @return The number of parsed args NOT including the sentence type, or an error.
 */
int8_t nmeaParseSentence (char *str, uint8_t len, char **argv) {
    uint8_t crcc, crcr, flag, i;
    int8_t argc;
    //
    argc = 0;
    crcc = 0;
    crcr = 0;
    flag = FALSE;
    // Sentences should be within defined a minimum and maximum.
    if ((len < NMEA_STR_MIN) || (len > NMEA_STR_MAX)) return NMEA_PARSERR_SIZE;
    // All sentences must begin with '$'.
    if (*str != '$') return NMEA_PARSERR_START;
    // Does the sentence have a CRC field?
    if (*(str + len - 3) == '*') {
        len -= 3;
        // Set '*' delimiter to NULL.
        *(str + len) = 0;
        // Read Checksum.
        i = nmeaParseChecksum(str + len + 1, &crcr);
        if (i < 0) return i;
        // Calculate Checksum.
        for (i = 1; i < len; ++i) crcc ^= *(str + i);
        // Compare Checksums.
        if (crcc != crcr) return NMEA_PARSERR_CRCC;
    }
    // Grab the start of each field and set ',' delimiters to NULL.
    for (i = 1; i < len; ++i) {
        // Get field pointer.
        if (!flag) {
            flag = TRUE;
            argv[argc] = str + i;
        }
        if (argc == NMEA_STR_FIELDS) return NMEA_PARSERR_FIELDS;
        // Is this the end of string?
        if (*(str + i) == 0) break;
        // Is this a ',' delimiter?
        if (*(str + i) == ',') {
            flag = FALSE;
            *(str + i) = 0;
            ++argc;
        }
    }
    return argc;
}

/** Parse a string for time.
 * @param str: string to be parsed. Format is HHMMSS[.d[dd]]
 * @param utc: pointer to a nmeaTIME structure.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseTime (char *str, nmeaTIME *utc) {
    uint8_t i, l;
    //
    l = strlen(str);
    switch (l) {
        case 0:
            return 0;
            break;
        case 6:
            // HHMMSS
            i = sscanf_P(str, PSTR("%2d%2d%2d"), &(utc->hour), &(utc->min), &(utc->sec));
            if (i == 3) return NMEA_VALID_TIME;
            break;
        case 8:
        case 9:
        case 10:
            // HHMMSS.d[dd]
            i = sscanf_P(str, PSTR("%2d%2d%2d.%d"), &(utc->hour), &(utc->min), &(utc->sec), &(utc->hsec));
            if (i == 4) return NMEA_VALID_TIME;
            break;
    }
    return NMEA_PARSERR_TIME;
}

/** Parse argv for a GPGGA sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPGGA (uint8_t argc, char **argv, nmeaGPGGA *gpgga) {
    int8_t i, k;
    //
    if (argc != NMEA_ARGC_GPGGA) return NMEA_PARSERR_GPGGA;
    memset(gpgga, 0, sizeof(nmeaGPGGA));
    k = 1;
    i = nmeaParseTime(argv[k++], &(gpgga->utc));
    if (i < 0) return i;
    if (*argv[k] == 0) return i;
    gpgga->lat = atof(argv[k++]);
    if (*argv[k] == 0) return i;
    gpgga->ns = *argv[k++];
    if (*argv[k] == 0) return i;
    gpgga->lon = atof(argv[k++]);
    if (*argv[k] == 0) return i;
    gpgga->ew = *argv[k++];
    gpgga->sig = atoi(argv[k++]);
    gpgga->satinuse = atoi(argv[k++]);
    gpgga->HDOP = atof(argv[k++]);
    gpgga->elv = atof(argv[k++]);
    gpgga->elv_units = *argv[k++];
    gpgga->diff = atof(argv[k++]);
    gpgga->diff_units = *argv[k++];
    gpgga->dgps_age = atof(argv[k++]);
    gpgga->dgps_sid = atoi(argv[k]);
    return (i | NMEA_VALID_POS);
}

/** Parse argv for a GPGSA sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPGSA (uint8_t argc, char **argv, nmeaGPGSA *gpgsa) {
    int8_t i, k;
    //
    if (argc != NMEA_ARGC_GPGSA) return NMEA_PARSERR_GPGSA;
    memset(gpgsa, 0, sizeof(nmeaGPGSA));
    k = 1;
    gpgsa->fix_mode = *argv[k++];
    gpgsa->fix_type = atoi(argv[k++]);
    for (i = 0; i < NMEA_MAXSAT; ++i) {
        gpgsa->sat_prn[i] = atoi(argv[k++]);
    }
    gpgsa->PDOP = atof(argv[k++]);
    gpgsa->HDOP = atof(argv[k++]);
    gpgsa->VDOP = atof(argv[k]);
    return 0;
}

/** Parse argv for a GPGSV sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPGSV (uint8_t argc, char **argv, nmeaGPGSV *gpgsv) {
    int8_t i, j, k;
    //
    if ((argc < NMEA_ARGC_GPGSV_MIN) || (argc > NMEA_ARGC_GPGSV_MAX)) return NMEA_PARSERR_GPGSV;
    memset(gpgsv, 0, sizeof(nmeaGPGSV));
    k = 1;
    gpgsv->pack_count = atoi(argv[k++]);
    gpgsv->pack_index = atoi(argv[k++]);
    gpgsv->sat_count = atoi(argv[k++]);
    j = gpgsv->sat_count - ((gpgsv->pack_index - 1) * NMEA_SATINPACK);
    if (j > NMEA_SATINPACK) j = NMEA_SATINPACK;
    for (i = 0; i < j; ++i) {
        gpgsv->sat_data[i].id = atoi(argv[k++]);
        gpgsv->sat_data[i].elv = atoi(argv[k++]);
        gpgsv->sat_data[i].azimuth = atoi(argv[k++]);
        gpgsv->sat_data[i].sig = atoi(argv[k++]);
    }
    return 0;
}

/** Parse argv for a GPGLL sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPGLL (uint8_t argc, char **argv, nmeaGPGLL *gpgll) {
    int8_t i, k;
    //
    if ((argc < NMEA_ARGC_GPGLL_MIN) && (argc > NMEA_ARGC_GPGLL_MAX)) return NMEA_PARSERR_GPGLL;
    memset(gpgll, 0, sizeof(nmeaGPGLL));
    k = 1;
    if (*argv[k] == 0) return 0;
    gpgll->lat = atof(argv[k++]);
    if (*argv[k] == 0) return 0;
    gpgll->ns = *argv[k++];
    if (*argv[k] == 0) return 0;
    gpgll->lon = atof(argv[k++]);
    if (*argv[k] == 0) return 0;
    gpgll->ew = *argv[k++];
    i = nmeaParseTime(argv[k++], &(gpgll->utc));
    if (i < 0) return i;
    gpgll->valid = *argv[k];
    return (i | NMEA_VALID_POS);
}

/** Parse argv for a GPRMC sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPRMC (uint8_t argc, char **argv, nmeaGPRMC *gprmc) {
    int8_t i, k;
    //
    if (argc != NMEA_ARGC_GPRMC) return NMEA_PARSERR_GPRMC;
    memset(gprmc, 0, sizeof(nmeaGPRMC));
    k = 1;
    i = nmeaParseTime(argv[k++], &(gprmc->utc));
    if (i < 0) return i;
    gprmc->status = *argv[k++];
    if (*argv[k] == 0) return i;
    gprmc->lat = atof(argv[k++]);
    if (*argv[k] == 0) return i;
    gprmc->ns = *argv[k++];
    if (*argv[k] == 0) return i;
    gprmc->lon = atof(argv[k++]);
    if (*argv[k] == 0) return i;
    gprmc->ew = *argv[k++];
    gprmc->speed = atof(argv[k++]);
    gprmc->direction = atof(argv[k++]);
    gprmc->declination = atof(argv[k++]);
    gprmc->declin_ew = *argv[k++];
    gprmc->mode = *argv[k];
    return (i | NMEA_VALID_POS);
}

/** Parse argv for a GPVTG sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPVTG (uint8_t argc, char **argv, nmeaGPVTG *gpvtg) {
    int8_t k;
    //
    if (argc != NMEA_ARGC_GPVTG) return NMEA_PARSERR_GPVTG;
    memset(gpvtg, 0, sizeof(nmeaGPVTG));
    k = 1;
    gpvtg->dir = atof(argv[k++]);
    gpvtg->dir_t = *argv[k++];
    gpvtg->dec = atof(argv[k++]);
    gpvtg->dec_m = *argv[k++];
    gpvtg->spn = atof(argv[k++]);
    gpvtg->spn_n = *argv[k++];
    gpvtg->spk = atof(argv[k++]);
    gpvtg->spk_k = *argv[k];
    return 0;
}

/** Parse argv for a GPZDA sentence.
 * @param argc: arg count, NOT including the sentence type.
 * @param argv: pointer table.
 * @return Flags on success (7-bit) or an error (negative).
 */
int8_t nmeaParseGPZDA (uint8_t argc, char **argv, nmeaTIME *t) {
    int8_t i, k;
    //
    if (argc != NMEA_ARGC_GPZDA) return NMEA_PARSERR_GPZDA;
    memset(t, 0, sizeof(nmeaTIME));
    k = 1;
    i = nmeaParseTime(argv[k++], t);
    if (i < 0) return i;
    t->day = atoi(argv[k++]);
    t->mon = atoi(argv[k++]);
    t->year = atoi(argv[k]);
    if ((t->day > 0) && (t->mon > 0) && (t->year >= 1900)) {
        t->year -= 1900;
        return (i | NMEA_VALID_DATE);
    }
    return i;
}

/**********************************************************/

/** Initialize NMEA structures to all zeroes.
 */
void nmeaInitInfo (void) {
    memset(&nmea_info, 0, sizeof(nmeaINFO));
    memset(&nmea_satinfo, 0, sizeof(nmeaSATINFO));
}

/** Copy NMEA UTC data from temporary to global structure.
 *  Also used to copy GPZDA sentence info.
 * @param p: pointer to nmeaTIME structure.
 */
void nmeaTimeInfo (nmeaTIME *p, int8_t flags) {
    if (flags & NMEA_VALID_DATE) {
        nmea_info.utc.day = p->day;
        nmea_info.utc.mon = p->mon;
        nmea_info.utc.year = p->year;
    }
    if (flags & NMEA_VALID_TIME) {
        nmea_info.utc.hsec = p->hsec;
        nmea_info.utc.sec = p->sec;
        nmea_info.utc.min = p->min;
        nmea_info.utc.hour = p->hour;
    }
}

/** Copy GPGGA data from temporary to global structure.
 * @param p: pointer to GPGGA structure.
 */
void nmeaGPGGAInfo (nmeaGPGGA *p, int8_t flags) {
    nmeaTimeInfo(&p->utc, flags);
    nmea_info.sig = p->sig;
    nmea_info.HDOP = p->HDOP;
    nmea_info.elv = p->elv;
    nmea_info.position.lat = ((p->ns == 'N') ? p->lat : -(p->lat));
    nmea_info.position.lon = ((p->ew == 'E') ? p->lon : -(p->lon));
}

/** Copy GPGSA data from temporary to global structure.
 * @param p: pointer to GPGSA structure.
 */
void nmeaGPGSAInfo (nmeaGPGSA *p, int8_t flags) {
    int i, j, nuse;
    time_t timer0;
    //
    timer0 = time(NULL);
    nuse = 0;
    nmea_info.fix = p->fix_type;
    nmea_info.PDOP = p->PDOP;
    nmea_info.HDOP = p->HDOP;
    nmea_info.VDOP = p->VDOP;
    for (i = 0; i < NMEA_MAXSAT; ++i) {
        if (nmea_info.satinfo.sat[i].lastheard < (timer0 - 60)) {
            nmea_info.satinfo.sat[i].in_use = 0;
        }
        for (j = 0; j < nmea_info.satinfo.inview; ++j) {
            if (p->sat_prn[i] == nmea_info.satinfo.sat[j].id) {
                nmea_info.satinfo.sat[j].lastheard = timer0;
                nmea_info.satinfo.sat[j].in_use = 1;
                nuse++;
            }
        }
    }
    nmea_info.satinfo.inuse = nuse;
}

/** Copy GPGSV data from temporary to global structure.
 * @param p: pointer to GPGSV structure.
 */
void nmeaGPGSVInfo (nmeaGPGSV *p, int8_t flags) {
    int isat, isi, nsat;
    //
    if ((p->pack_index > p->pack_count) || ((p->pack_index * NMEA_SATINPACK) > NMEA_MAXSAT)) return;
    if (p->pack_index < 1) p->pack_index = 1;
    nmea_info.satinfo.inview = p->sat_count;
    nsat = (p->pack_index - 1) * NMEA_SATINPACK;
    nsat = (nsat + NMEA_SATINPACK > p->sat_count) ? p->sat_count - nsat : NMEA_SATINPACK;
    for (isat = 0; isat < nsat; ++isat) {
        isi = (p->pack_index - 1) * NMEA_SATINPACK + isat;
        nmea_info.satinfo.sat[isi].id = p->sat_data[isat].id;
        nmea_info.satinfo.sat[isi].elv = p->sat_data[isat].elv;
        nmea_info.satinfo.sat[isi].azimuth = p->sat_data[isat].azimuth;
        nmea_info.satinfo.sat[isi].sig = p->sat_data[isat].sig;
    }
}

/**
 */
void nmeaGPGLLInfo (nmeaGPGLL *p, int8_t flags) {
    if (p->valid == 'A') {
        nmea_info.position.lat = ((p->ns == 'N') ? p->lat : -(p->lat));
        nmea_info.position.lon = ((p->ew == 'E') ? p->lon : -(p->lon));
        nmeaTimeInfo(&p->utc, flags);
    }
}

/** Copy GPRMC data from temporary to global structure.
 * @param p: pointer to GPRMC structure.
 */
void nmeaGPRMCInfo (nmeaGPRMC *p, int8_t flags) {
    if (p->status == 'A') {
        if (nmea_info.sig == NMEA_SIG_BAD) nmea_info.sig = NMEA_SIG_MID;
        if (nmea_info.fix == NMEA_FIX_BAD) nmea_info.fix = NMEA_FIX_2D;
    }
    else if (p->status == 'V') {
        nmea_info.sig = NMEA_SIG_BAD;
        nmea_info.fix = NMEA_FIX_BAD;
    }
    nmeaTimeInfo(&p->utc, flags);
    nmea_info.position.lat = ((p->ns == 'N') ? p->lat : -(p->lat));
    nmea_info.position.lon = ((p->ew == 'E') ? p->lon : -(p->lon));
    nmea_info.speed = p->speed * NMEA_CONV_KNOT;
    nmea_info.direction = p->direction;
}

/** Copy GPVTG data from temporary to global structure.
 * @param p: pointer to GPVTG structure.
 */
void nmeaGPVTGInfo (nmeaGPVTG *p, int8_t flags) {
    nmea_info.direction = p->dir;
    nmea_info.declination = p->dec;
    nmea_info.speed = p->spk;
}

/************************************************/

/** Set the system position/date/time using NMEA data.
 */
void nmeaInfoPosTime (int8_t flags) {
    struct  tm stm;
    time_t  timer0, timer1;
    int32_t timed;
    //
    timer0 = time(NULL);
    // Update Position.
    if (flags & NMEA_VALID_POS) {
        timed = nmea_info.ptimer - timer0;
        if ((timed < 0) || (timed > NMEA_POS_FREQ)) {
            nmea_info.ptimer = timer0 + NMEA_POS_FREQ;
            position.latitudeD = (int8_t) (nmea_info.position.lat / 100.0);
            position.latitudeM = fmod(nmea_info.position.lat, 100.0);
            position.longitudeD = (int16_t) (nmea_info.position.lon / 100.0);
            position.longitudeM = fmod(nmea_info.position.lon, 100.0);
        }
    }
    // Update clock.
    if (flags & (NMEA_VALID_TIME | NMEA_VALID_DATE)) {
        timed = nmea_info.ctimer - timer0;
        if ((timed < 0) || (timed > NMEA_TIME_FREQ)) {
            nmea_info.ctimer = timer0 + NMEA_TIME_FREQ;
            gmtime_r(&timer0, &stm);
            if (flags & NMEA_VALID_TIME) {
                stm.tm_sec = nmea_info.utc.sec;
                stm.tm_min = nmea_info.utc.min;
                stm.tm_hour = nmea_info.utc.hour;
            }
            if (flags & NMEA_VALID_DATE) {
                stm.tm_mday = nmea_info.utc.day;
                stm.tm_mon = nmea_info.utc.mon;
                stm.tm_year = nmea_info.utc.year;
            }
            timer1 = mk_gmtime(&stm);
            localtime_r(&timer1, &stm);
            timer1 = mktime(&stm);
            timed = timer1 - timer0;
            // Update if local clock is more than NMEA_TIME_DRIFT seconds off.
            if ((timed > NMEA_TIME_DRIFT) || (timed < -NMEA_TIME_DRIFT)) {
                // If the difference is more than an hour BEHIND system time,
                // increment by one day.
                if (timed < -ONE_HOUR) timer1 += ONE_DAY;
                clockSet(timer1);
            }
        }
    }
}

/** Print an error message to stderr.
 * @param errno: error number.
 */
char* nmeaPrintError (char *buf, int8_t errno) {
    buf += sprintf_P(buf, PSTR("NMEA: "));
    switch (errno) {
        case NMEA_PARSERR_SIZE:
            buf += sprintf_P(buf, PSTR("Bad Size"));
            break;
        case NMEA_PARSERR_START:
            buf += sprintf_P(buf, PSTR("Bad Start"));
            break;
        case NMEA_PARSERR_CRCR:
            buf += sprintf_P(buf, PSTR("Bad RX CRC"));
            break;
        case NMEA_PARSERR_CRCC:
            buf += sprintf_P(buf, PSTR("Bad Calculated CRC"));
            break;
        case NMEA_PARSERR_FIELDS:
            buf += sprintf_P(buf, PSTR("Too Many Fields"));
            break;
        case NMEA_PARSERR_TIME:
            buf += sprintf_P(buf, PSTR("Bad Time"));
            break;
        case NMEA_PARSERR_GPGGA:
            buf += sprintf_P(buf, PSTR("Bad GPGGA"));
            break;
        case NMEA_PARSERR_GPGSA:
            buf += sprintf_P(buf, PSTR("Bad GPGSA"));
            break;
        case NMEA_PARSERR_GPGSV:
            buf += sprintf_P(buf, PSTR("Bad GPGSV"));
            break;
        case NMEA_PARSERR_GPRMC:
            buf += sprintf_P(buf, PSTR("Bad GPRMC"));
            break;
        case NMEA_PARSERR_GPVTG:
            buf += sprintf_P(buf, PSTR("Bad GPVTG"));
            break;
        case NMEA_UNKNOWN_TYPE:
            buf += sprintf_P(buf, PSTR("Unknown Sentence type"));
            break;
    }
    return buf;
}
