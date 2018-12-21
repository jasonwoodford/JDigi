/*
 * threadEnviro.c
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
 * $Id: threadEnviro.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "threads.h"

static uint8_t bufEnv[ENVIRO_STATIC_SIZE];

extern APRSConfig config;
extern APRSPosit position;
//extern BME280 enviroBME280;
extern nmeaINFO nmea_info;

/** Environmental update thread.
 */
void threadEnviro (uint32_t bufsz) {
    //uint8_t stat;
    int8_t  count, i;
    char    *p;
    //
    count = 0;
    while (TRUE) {
        p = (char*) bufEnv;
        // Environmental stuff is sampled ~ 5 sec.
        atomTimerDelay(5 * SYSTEM_TICKS_PER_SEC);
        i = ds18b20GetTemp(config.devTemp);
        if (i < 0) {
            *p = '\n';
            ds18b20PrintError(p+1, i);
            fputs(p, stderr);
        }
        if (++count == 2) {
            // Debug stuff goes here.
            count = 0;
/*            // TEST BME280.
            stat = bme280ReadEnviro(BME280_I2C_ADDRESS);
            if (stat > 0) fprintf_P(stderr, PSTR("\nBME280 Read Status: 0x%02X"), stat);
            // Always do a bme280CompTemp() before other functions.
            fprintf_P(stderr, PSTR("\nBME280:[T/P/H]:[%f / "), bme280CompTemp());
            fprintf_P(stderr, PSTR("%f / %f]"), bme280CompPres(), bme280CompHumd());
*/
/*            // TEST DHT11.
            i = dht11Read();
            fprintf_P(stderr, PSTR("\nDHT11[T/H]:%02d/%02d(%d)"), dht11.temp, dht11.humd, i);
*/
/*            // TEST Temperature devices.
            fprintf_P(stderr, PSTR("\nThermistor (Steinhart): %+03.1fC"), thermistorGetTempS(THERMISTOR_PIN));
            fprintf_P(stderr, PSTR("\nThermistor (Linear): %+03.1fC"), thermistorGetTempL(THERMISTOR_PIN));
            fprintf_P(stderr, PSTR("\nDS18B20: %+03.1fC"), ds18b20ConvTemp(devid, DS18B20_UNIT_C));
*/
/*            // TEST GPS.
            fputs_P(PSTR("\n***NMEAInfo***"), stderr);
            fprintf_P(stderr, PSTR("\nLat:%f Lon:%f"), nmea_info.position.lat, nmea_info.position.lon);
            fprintf_P(stderr, PSTR("\nDate: %02d/%02d/%04d"), nmea_info.utc.mon + 1, nmea_info.utc.day, nmea_info.utc.year + 1900);
            fprintf_P(stderr, PSTR("\nTime: %02d:%02d:%02d"), nmea_info.utc.hour, nmea_info.utc.min, nmea_info.utc.sec);
            fputs_P(PSTR("\nSignal: "), stderr);
            switch (nmea_info.sig) {
                case NMEA_SIG_BAD:
                    fputs_P(PSTR("Invalid"), stderr);
                    break;
                case NMEA_SIG_LOW:
                    fputs_P(PSTR("Fix"), stderr);
                    break;
                case NMEA_SIG_MID:
                    fputs_P(PSTR("Differential"), stderr);
                    break;
                case NMEA_SIG_HIGH:
                    fputs_P(PSTR("Sensitive"), stderr);
                    break;
            }
            fputs_P(PSTR("\nFix: "), stderr);
            switch (nmea_info.fix) {
                case NMEA_FIX_BAD:
                    fputs_P(PSTR("N/A"), stderr);
                    break;
                case NMEA_FIX_2D:
                    fputs_P(PSTR("2D"), stderr);
                    break;
                case NMEA_FIX_3D:
                    fputs_P(PSTR("3D"), stderr);
                    break;
            }
            fprintf_P(stderr, PSTR("\nPDOP: %f, HDOP: %f, VDOP: %f"),
                      nmea_info.PDOP, nmea_info.HDOP, nmea_info.VDOP);
            fprintf_P(stderr, PSTR("\nElev: %f, Speed: %f, Direction: %f, Declination: %f"),
                      nmea_info.elv, nmea_info.speed, nmea_info.direction, nmea_info.declination);
            fprintf_P(stderr, PSTR("\nSats: in view %02d, in use %02d"),
                      nmea_info.satinfo.inview, nmea_info.satinfo.inuse);
            for (i = 0; i < NMEA_MAXSAT; ++i) {
                if (nmea_info.satinfo.sat[i].in_use == 1) {
                    fprintf_P(stderr, PSTR("\n-> PRN: %d, Elev: %d, Azi: %d, Sig: %d"),
                              nmea_info.satinfo.sat[i].id,
                              nmea_info.satinfo.sat[i].elv,
                              nmea_info.satinfo.sat[i].azimuth,
                              nmea_info.satinfo.sat[i].sig);
                }
            }
*/
/*
            fprintf_P(stderr, PSTR("\nPosition: %02d %f %03d %f"),
                      position.latitudeD, position.latitudeM,
                      position.longitudeD, position.longitudeM);
*/
        }
    }
}
