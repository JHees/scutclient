#pragma once

#include "tracelog.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <ctype.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

// #define LOGOFF 0
#define SERVER_ADDR "202.38.210.131"
#define SERVER_PORT 61440
#define DNS_ADDR    "222.201.130.30"
struct config
{
    struct in_addr udpserver_ipaddr;
    struct in_addr dns_ipaddr;
    std::string UserName;
    std::string Password;
    std::string OnlineHookCmd;
    std::string OfflineHookCmd;
    std::string DeviceName = "eth0";
    std::string HostName;
    std::string Hash          = "2ec15ad258aee9604b18f2f8114da38db16efd00";
    unsigned char Version[64] = {0x44, 0x72, 0x43, 0x4f, 0x4d, 0x00, 0x96, 0x02, 0x2a};
    int Version_len           = 9;
    struct time
    {
        uint8_t a_hour;
        uint8_t a_minute;
    } accessTime;
    LOGLEVEL cloglev = INF; // debug log level
    enum
    {
        LOGOFF   = 0,
        LOGIN    = 1,
        HELPMODE = 2
    } client
        = LOGIN;
    config()
    {
        inet_aton(DNS_ADDR, &dns_ipaddr);
        inet_aton(SERVER_ADDR, &udpserver_ipaddr);
        char host[32] = {0};
        gethostname(host, sizeof(host));
        HostName = host;
    };
};

inline int hexStrToByte(const char *source, unsigned char *dest, int bufLen)
{
    int i;
    unsigned char highByte, lowByte;

    for (i = 0; source[i * 2] && source[i * 2 + 1] && (i < bufLen); i++)
    {
        highByte = toupper(source[i * 2]);
        lowByte  = toupper(source[i * 2 + 1]);

        highByte -= highByte > 0x39 ? 0x37 : 0x30;
        lowByte -= lowByte > 0x39 ? 0x37 : 0x30;
        dest[i] = (highByte << 4) | lowByte;
    }
    return i;
}