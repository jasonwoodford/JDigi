/*
 * command.c
 * 
 * Copyright (c) 2017, Jason Woodford, VO1JWW. All rights reserved.
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
 * $Id: command.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "command.h"

static char const pstrBadParams[]   PROGMEM = "?Bad parameters";
static char const pstrCmdList[]     PROGMEM = \
"Available commands:\n\
APSTAT  CLRBCN  DSTINI  ENUMOW  LCDINI  LSTBCN  LSTENV  LSTOW\n\
HELP    SETBCN  SETCAL  SETCFL  SETCLK  SETCOM  SETDST  SETICO\n\
SETINB  SETINP  SETINW  SETPOS  SETVC1  SETVC2  WREROM";

char const pstrCmdPROMPT[]          PROGMEM = "\nJDigi> ";
char const pstrCmdAPRSStats[]       PROGMEM = "APSTAT";
char const pstrCmdBeaconClear[]     PROGMEM = "CLRBCN";
char const pstrCmdDS18B20Init[]     PROGMEM = "DSTINI";
char const pstrCmdOnewireEnum[]     PROGMEM = "ENUMOW";
char const pstrCmdLcdInit[]         PROGMEM = "LCDINI";
char const pstrCmdBeaconList[]      PROGMEM = "LSTBCN";
char const pstrCmdEnviroList[]      PROGMEM = "LSTENV";
char const pstrCmdOnewireList[]     PROGMEM = "LSTOW";
char const pstrCmdHelp[]            PROGMEM = "HELP";
char const pstrCmdBeaconSet[]       PROGMEM = "SETBCN";
char const pstrCmdSetCall[]         PROGMEM = "SETCAL";
char const pstrCmdSetChainflags[]   PROGMEM = "SETCFL";
char const pstrCmdSetClock[]        PROGMEM = "SETCLK";
char const pstrCmdSetComment[]      PROGMEM = "SETCOM";
char const pstrCmdSetDest[]         PROGMEM = "SETDST";
char const pstrCmdSetIcon[]         PROGMEM = "SETICO";
char const pstrCmdSetBeaconint[]    PROGMEM = "SETINB";
char const pstrCmdSetPosint[]       PROGMEM = "SETINP";
char const pstrCmdSetWXint[]        PROGMEM = "SETINW";
char const pstrCmdSetPos[]          PROGMEM = "SETPOS";
char const pstrCmdSetVia1[]         PROGMEM = "SETVC1";
char const pstrCmdSetVia2[]         PROGMEM = "SETVC2";
char const pstrCmdWriteEeprom[]     PROGMEM = "WREROM";

extern APRSConfig config;
extern AX25Frame frame[];
extern Beacon beacon[];
extern DS18B20 ds18b20[];
extern Stats statTable[];
extern ONEWIRE onewire;
extern ONEWIREDev onewireDevice[];

/** Parse command line.
 */
uint8_t cmdParseLine (char *buf, uint8_t size, char **argv) {
    uint8_t argc, i;
    char    delim;
    //
    argc = 0;
    i = 0;
    while (argc < 8) {
        delim = ' ';
        // skip leading spaces and set additional arg values.
        while ((i < size) && (*(buf + i) == delim)) ++i;
        if (i == size) break;
        // if this arg begins with a '"' set it as the delimiter.
        if (*(buf + i) == '"') {
            delim = *(buf + i);
            ++i;
        }
        if (i == size) break;
        argv[argc] = buf + i;
        ++argc;
        // find the next delimiter and set it to zero.
        while ((i < size) && (*(buf + i) != delim)) ++i;
        *(buf + i) = 0;
        if (i == size) break;
        ++i;
    }
    return argc;
}

/**
 */
char* cmdAPRSHeardDirect (char *buf, uint8_t argc, char **argv) {
    uint8_t i, j;
    //
    buf += sprintf_P(buf, PSTR("Directs="));
    for (i = 0; i < STATS_SLOTS; ++i) {
        if (!(statTable[i].flags & STATS_FLAG_E)) continue;
        for (j = 0; j < STATS_HOURS; ++j) {
            if (statTable[i].hourlyDirect[j] > 0) {
                *buf++ = ' ';
                buf += ax25FmtAddr(buf, statTable[i].callsign);
                break;
            }
        }
    }
    *buf = 0;
    return buf;
}

/**
 */
char* cmdAPRSHeardAll (char *buf, uint8_t argc, char **argv) {
    uint8_t i;
    //
    buf += sprintf_P(buf, PSTR("Heard="));
    for (i = 0; i < STATS_SLOTS; ++i) {
        if (!(statTable[i].flags & STATS_FLAG_E)) continue;
        *buf++ = ' ';
        buf += ax25FmtAddr(buf, statTable[i].callsign);
    }
    *buf = 0;
    return buf;
}

/**
 */
char* cmdAPRSHeardStatsDirect (char *buf, uint8_t argc, char **argv) {
    uint8_t addr[AX25_ADDR_SIZE];
    uint8_t i, j;
    //
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        aprsMakeAddr(argv[1], addr);
        for (j = 0; j < STATS_SLOTS; ++j) {
            if (!(statTable[j].flags & STATS_FLAG_E)) continue;
            if (memcmp(statTable[j].callsign, addr, AX25_ADDR_SIZE) != 0) continue;
            buf += ax25FmtAddr(buf, addr);
            buf += sprintf_P(buf, PSTR(" Heard Direct:"));
            for (i = 0; i < STATS_HOURS; ++i) {
                buf += sprintf_P(buf, PSTR(" %d"), statTable[j].hourlyDirect[i]);
            }
        }
    }
    return buf;
}

/**
 */
char* cmdAPRSHeardStatsAll (char *buf, uint8_t argc, char **argv) {
    uint8_t addr[AX25_ADDR_SIZE];
    uint8_t i, j;
    //
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        aprsMakeAddr(argv[1], addr);
        for (j = 0; j < STATS_SLOTS; ++j) {
            if (!(statTable[j].flags & STATS_FLAG_E)) continue;
            if (memcmp(statTable[j].callsign, addr, AX25_ADDR_SIZE) != 0) continue;
            buf += ax25FmtAddr(buf, addr);
            buf += sprintf_P(buf, PSTR(" Heard:"));
            for (i = 0; i < STATS_HOURS; ++i) {
                buf += sprintf_P(buf, PSTR(" %d"), statTable[j].hourlyHeard[i]);
            }
        }
    }
    return buf;
}

/**
 */
char* cmdAPRSRouteTrace (char *buf, uint8_t slot) {
    uint8_t i;
    //
    buf += sprintf_P(buf, PSTR("%s"), config.srcCall);
    if (config.srcSSID > 0) buf += sprintf_P(buf, PSTR("-%d"), config.srcSSID);
    buf += sprintf_P(buf, PSTR(">APRS"));
    for (i = 1; i <= frame[slot].hops; ++i) {
        *buf++ = ',';
        buf += ax25FmtAddr(buf, frame[slot].addr[i+1]);
    }
    *buf++ = ':';
    *buf = 0;
    return buf;
}

/**
 */
char* cmdBeaconList (char *buf) {
    uint8_t i;
    char    *p;
    time_t  timer;
    //
    timer = time(NULL);
    buf += sprintf_P(buf, PSTR("BEACONS:%d"), BEACON_TABLE_SIZE);
    for (i = 0; i < BEACON_TABLE_SIZE; ++i) {
        if (beacon[i].flags & BEACON_FLAG_E) {
            switch (beacon[i].type) {
                case BEACON_TYPE_BEACON:
                    p = (char*) PSTR("BEACON");
                    break;
                case BEACON_TYPE_POSIT:
                    p = (char*) PSTR("POSIT");
                    break;
                case BEACON_TYPE_WX:
                    p = (char*) PSTR("WX");
                    break;
                default:
                    p = (char*) PSTR("UNKNOWN");
            }
            buf += sprintf_P(buf, PSTR(";%d,%S,%d,%d"), i, p, beacon[i].interval, beacon[i].timeout - timer);
        }
    }
    return buf;
}

/**
 */
char* cmdBeaconClear (char *buf, uint8_t argc, char **argv) {
    uint8_t i;
    //
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        i = atoi(argv[1]);
        if (i >= BEACON_TABLE_SIZE) {
            buf += sprintf_P(buf, PSTR("?Invalid index"));
        } else {
            beacon[i].flags = 0;
            buf += sprintf_P(buf, PSTR("Cleared beacon #%d"), i);
        }
    }
    return buf;
}

/**
 */
char* cmdBeaconSet (char *buf, uint8_t argc, char **argv) {
    uint8_t  type;
    uint16_t period, offset;
    int8_t   slot;
    //
    if (argc != 4) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        type = argv[1][0];
        period = atoi(argv[2]);
        offset = atoi(argv[3]);
        switch (type) {
            case 'P':
                type = BEACON_TYPE_POSIT;
                break;
            case 'W':
                type = BEACON_TYPE_WX;
                break;
            case 'B':
            default:
                type = BEACON_TYPE_BEACON;
        }
        slot = beaconSet(period, offset, type, config.infoComment);
        if (slot < 0) {
            buf += sprintf_P(buf, PSTR("Beacon could not be set"));
        } else {
            buf += sprintf_P(buf, PSTR("Beacon #%d set"), slot);
        }
    }
    return buf;
}

/**
 */
char* cmdClockSet (char *buf, uint8_t argc, char **argv) {
    uint16_t    tu[3];
    time_t      timer;
    struct tm   clk;
    //
    if (argc != 3) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        // Always init the time structure.
        timer = time(NULL);
        localtime_r(&timer, &clk);
        // MM/DD/YY
        sscanf_P(argv[1], PSTR("%u/%u/%u"), &tu[0], &tu[1], &tu[2]);
        clk.tm_mon = tu[0] - 1;
        clk.tm_mday = tu[1];
        clk.tm_year = tu[2] - 1900;
        // HH:MM:SS
        sscanf_P(argv[2], PSTR("%u:%u:%u"), &tu[0], &tu[1], &tu[2]);
        clk.tm_hour = tu[0];
        clk.tm_min = tu[1];
        clk.tm_sec = tu[2];
        // Set time.
        timer = mktime(&clk);
        localtime_r(&timer, &clk);
        timer = mktime(&clk);
        clockSet(timer);
        buf += sprintf_P(buf, PSTR("Clock set to "));
        ctime_r(&timer, buf);
        buf += strlen(buf);
    }
    return buf;
}

/**
 */
char* cmdConfigSetcall (char *buf, uint8_t argc, char **argv) {
    uint8_t ssid;
    int8_t i;
    //
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        i = aprsParseAddr(argv[1], &ssid);
        if (i > 3) {
            strcpy(config.srcCall, argv[1]);
            config.srcSSID = ssid;
            fprintf_P(stderr, PSTR("\nSetCall: %s-%u"), argv[1], ssid);
        } else {
            buf += sprintf_P(buf, PSTR("?Invalid callsign"));
        }
    }
    return buf;
}

/**
 */
char* cmdConfigSetdest (char *buf, uint8_t argc, char **argv) {
    uint8_t ssid;
    int8_t  i;
    //
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        i = aprsParseAddr(argv[1], &ssid);
        if (i > 3) {
            strcpy(config.destCall, argv[1]);
            config.destSSID = ssid;
            buf += sprintf_P(buf, PSTR("SetDest: %s-%u"), argv[1], ssid);
        } else {
            buf += sprintf_P(buf, PSTR("?Invalid callsign"));
        }
    }
    return buf;
}

/**
 */
char* cmdConfigSetvia1 (char *buf, uint8_t argc, char **argv) {
    uint8_t ssid;
    int8_t  i;
    //
    if (argc > 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else if (argc == 1) {
        config.viaCall1[0] = 0;
        config.viaSSID1 = 0;
        buf += sprintf_P(buf, PSTR("ClearVia1"));
    } else {
        i = aprsParseAddr(argv[1], &ssid);
        if (i > 3) {
            strcpy(config.viaCall1, argv[1]);
            config.viaSSID1 = ssid;
            buf += sprintf_P(buf, PSTR("SetVia1: %s-%u"), argv[1], ssid);
        } else {
            buf += sprintf_P(buf, PSTR("?Invalid callsign"));
        }
    }
    return buf;
}

/**
 */
char* cmdConfigSetvia2 (char *buf, uint8_t argc, char **argv) {
    uint8_t ssid;
    int8_t  i;
    //
    if (argc > 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else if (argc == 1) {
        config.viaCall2[0] = 0;
        config.viaSSID2 = 0;
        buf += sprintf_P(buf, PSTR("ClearVia2"));
    } else {
        i = aprsParseAddr(argv[1], &ssid);
        if (i > 3) {
            strcpy(config.viaCall2, argv[1]);
            config.viaSSID2 = ssid;
            buf += sprintf_P(buf, PSTR("SetVia2: %s-%u"), argv[1], ssid);
        } else {
            buf += sprintf_P(buf, PSTR("?Invalid callsign"));
        }
    }
    return buf;
}

/**
 */
char* cmdConfigSetpos (char *buf, uint8_t argc, char **argv) {
    uint16_t    tu[2];
    char        tc;
    //
    if (argc != 3) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        sscanf_P(argv[1], PSTR("%u.%u%c"), &tu[0], &tu[1], &tc);
        if ((tu[0] <= 9000) && (tu[1] <= 99) && ((tc == 'N') || (tc == 'S'))) {
            config.posLatD = tu[0] / 100;
            if (tc == 'S') config.posLatD *= -1;
            config.posLatM = (float) (tu[0] % 100);
            config.posLatM = (float) (tu[1] / 100.0f);
        }
        sscanf_P(argv[2], PSTR("%u.%u%c"), &tu[0], &tu[1], &tc);
        if ((tu[0] <= 18000) && (tu[1] <= 99) && ((tc == 'E') || (tc == 'W'))) {
            config.posLongD = tu[0] / 100;
            if (tc == 'W') config.posLongD *= -1;
            config.posLongM = (float) (tu[0] % 100);
            config.posLongM = (float) (tu[1] / 100.0f);
        }
        //
        buf += sprintf_P(buf, PSTR("SetPos: "));
        buf += aprsPrintLat(buf, config.posLatD, config.posLatM, APRS_PRINTFMT_DEGMIN);
        buf += aprsPrintLong(buf, config.posLongD, config.posLongM, APRS_PRINTFMT_DEGMIN);
    }
    return buf;
}

/**
 */
char* cmdConfigSeticon (char *buf, uint8_t argc, char **argv) {
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        config.icon1 = argv[1][0];
        config.icon2 = argv[1][1];
        buf += sprintf_P(buf, PSTR("SetIcon: %c%c"), config.icon1, config.icon2);
    }
    return buf;
}

/**
 */
char* cmdConfigSetcomment (char *buf, uint8_t argc, char **argv) {
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        strncpy(config.infoComment, argv[1], APRS_CONFIG_COMMENT_SIZE);
        buf += sprintf_P(buf, PSTR("SetComment: %s"), config.infoComment);
    }
    return buf;
}

/**
 */
char* cmdConfigSetposint (char *buf, uint8_t argc, char **argv) {
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        config.txIntPos = atoi(argv[1]);
        buf += sprintf_P(buf, PSTR("SetPosInt: %d"), config.txIntPos);
    }
    return buf;
}

/**
 */
char* cmdConfigSetbeaconint (char *buf, uint8_t argc, char **argv) {
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        config.txIntBeacon = atoi(argv[1]);
        buf += sprintf_P(buf, PSTR("SetBeaconInt: %d"), config.txIntBeacon);
    }
    return buf;
}

/**
 */
char* cmdConfigSetwxint (char *buf, uint8_t argc, char **argv) {
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        config.txIntWX = atoi(argv[1]);
        buf += sprintf_P(buf, PSTR("SetWXInt: %d"), config.txIntWX);
    }
    return buf;
}

/**
 */
char* cmdConfigSetchainflags (char *buf, uint8_t argc, char **argv) {
    uint8_t flags;
    //
    if (argc != 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else {
        flags = atoi(argv[1]);
        if (flags <= 0xFF) config.chainFlags = flags;
        buf += sprintf_P(buf, PSTR("SetChainFlags: %d"), config.chainFlags);
    }
    return buf;
}

/**
 */
char* cmdConfigWriteEeprom (char *buf, uint8_t argc, char **argv) {
    config.lastTime = time(NULL);
    eepromWriteConfig();
    buf += sprintf_P(buf, PSTR("EEPROM configuration written"));
    return buf;
}

/**
 */
char* cmdDS18B20Init (char *buf, uint8_t argc, char **argv) {
    int8_t i, j;
    //
    i = ds18b20Init();
    if (i >= 0) {
        buf += sprintf_P(buf, PSTR("%d DS18B20 devices found."), i);
        while (i > 0) {
            --i;
            buf += sprintf_P(buf, PSTR("\n-> %s ROM"), onewireDevice[ds18b20[i].onewireIndex].id);
            j = 8;
            while (j > 0) {
                --j;
                buf += sprintf_P(buf, PSTR(":%02X"), onewireDevice[ds18b20[i].onewireIndex].rom[j]);
            }
        }
    }
    return buf;
}

/**
 */
char* cmdEnviroList (char *buf, uint8_t argc, char **argv) {
    char devid[11];
    //
    strcpy_P(devid, PSTR("18B20_00"));
    buf += sprintf_P(buf, PSTR("Temperature: %+4.1fC"), ds18b20ConvTemp(devid, DS18B20_UNIT_C));
    return buf;
}

/**
 */
char* cmdLcdInit(char *buf, uint8_t argc, char **argv) {
    lcdshieldInit();
    return buf;
}

/**
 */
char* cmdOnewireList (char *buf, uint8_t argc, char **argv) {
    int8_t i, j;
    //
    buf += sprintf_P(buf, PSTR("Onewire: "));
    if (onewire.flags & ONEWIRE_FLAG_VALID) {
        buf += sprintf_P(buf, PSTR("%d devices active."), onewire.devcount);
        j = 0;
        while (j < ONEWIRE_MAX_DEV) {
            if (onewireDevice[j].flags & ONEWIRE_FLAG_VALID) {
                buf += sprintf_P(buf, PSTR("\n-> %s ROM"), onewireDevice[j].id);
                i = 8;
                while (i > 0) {
                    --i;
                    buf += sprintf_P(buf, PSTR(":%02X"), onewireDevice[j].rom[i]);
                }
            }
            ++j;
        }
    } else {
        buf += sprintf_P(buf, PSTR("Bus not initialized."));
    }
    return buf;
}

/**
 */
char* cmdOnewireEnumerate (char *buf, uint8_t argc, char **argv) {
    int8_t i;
    //
    i = onewireEnumerate();
    if (i < 0) {
        onewire.flags &= ~ONEWIRE_FLAG_VALID;
        onewire.devcount = 0;
        buf = onewirePrintError(buf, i);
    } else {
        if (i > 0) onewire.flags |= ONEWIRE_FLAG_VALID;
        onewire.devcount = i;
        buf += sprintf_P(buf, PSTR("%d Onewire devices found."), i);
    }
    return buf;
}

/**
 */
char* cmdPrintHelp (char *buf, uint8_t argc, char **argv) {
    const char *p;
    //
    if (argc > 2) {
        buf += sprintf_P(buf, pstrBadParams);
    } else if (argc == 1) {
        buf += sprintf_P(buf, pstrCmdList);
    } else {
        strupr(argv[1]);
        if (strcmp_P(argv[1], pstrCmdBeaconList) == 0) {
            p = PSTR("%s\nLists all active beacons.");
        } else if (strcmp_P(argv[1], pstrCmdBeaconClear) == 0) {
            p = PSTR("%s N\nClears beacon at index N.");
        } else if (strcmp_P(argv[1], pstrCmdBeaconSet) == 0) {
            p = PSTR("%s T I N\nSets beacon.\nT: type (B,P,W); I: interval (secs); N: offset (secs)");
        } else if (strcmp_P(argv[1], pstrCmdSetClock) == 0) {
            p = PSTR("%s M/D/Y H:M:S\nSets system time.");
        } else if (strcmp_P(argv[1], pstrCmdSetCall) == 0) {
            p = PSTR("%s CALL-SSID\nSets APRS source callsign.");
        } else if (strcmp_P(argv[1], pstrCmdSetDest) == 0) {
            p = PSTR("%s CALL-SSID\nSets APRS destination callsign.");
        } else if (strcmp_P(argv[1], pstrCmdSetVia1) == 0) {
            p = PSTR("%s CALL-SSID\nSets APRS via1 path.");
        } else if (strcmp_P(argv[1], pstrCmdSetVia2) == 0) {
            p = PSTR("%s CALL-SSID\nSets APRS via2 path.");
        } else if (strcmp_P(argv[1], pstrCmdSetPos) == 0) {
            p = PSTR("%s lat.xxN long.yyE\nSets APRS position.");
        } else if (strcmp_P(argv[1], pstrCmdSetIcon) == 0) {
            p = PSTR("%s XY\nSets APRS icon.");
        } else if (strcmp_P(argv[1], pstrCmdSetComment) == 0) {
            p = PSTR("%s \"comment\"\nSets APRS comment.");
        } else if (strcmp_P(argv[1], pstrCmdSetPosint) == 0) {
            p = PSTR("%s I\nSets APRS Posit interval.\nI: interval (secs)");
        } else if (strcmp_P(argv[1], pstrCmdSetWXint) == 0) {
            p = PSTR("%s I\nSets APRS WX interval.\nI: interval (secs)");
        } else if (strcmp_P(argv[1], pstrCmdSetBeaconint) == 0) {
            p = PSTR("%s I\nSets APRS Beacon interval.\nI: interval (secs)");
        } else if (strcmp_P(argv[1], pstrCmdSetChainflags) == 0) {
            p = PSTR("%s X\nSets AX.25 chain flags.");
        } else if (strcmp_P(argv[1], pstrCmdEnviroList) == 0) {
            p = PSTR("%s\nList environmental conditions.");
        } else if (strcmp_P(argv[1], pstrCmdOnewireList) == 0) {
            p = PSTR("%s\nList Onewire devices.");
        } else if (strcmp_P(argv[1], pstrCmdOnewireEnum) == 0) {
            p = PSTR("%s\nEnumerate Onewire devices.");
        } else if (strcmp_P(argv[1], pstrCmdDS18B20Init) == 0) {
            p = PSTR("%s\nInitialize DS18B20 devices.");
        } else if (strcmp_P(argv[1], pstrCmdHelp) == 0) {
            p = PSTR("...I'm being oppressed!!!");
        } else {
            p = PSTR("%s is not a recognized command.");
        }
        buf += sprintf_P(buf, p, argv[1]);
    }
    return buf;
}
