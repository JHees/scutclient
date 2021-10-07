#pragma once

#include "clipp.h"
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
#define Print(x) LogWrite(INIT, INF, #x ": %s", x.c_str())
#define LOGOFF   0
using namespace clipp;

class config
{
public:
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
    int client       = 1;
    clipp::group cli;

private:
    config()
    {
        auto username  = required("u", "username") & value("username", UserName);
        auto password  = required("p", "password") & value("password", Password);
        auto interface = (option("i", "interface") & value("ifname", DeviceName))
                             .doc("Interface to perform authentication.");

        auto dns = (option("n", "dns") & value("dns") >> [&](const std::string &arg) { inet_aton(arg.c_str(), &dns_ipaddr); })
                       .doc("DNS server address to be sent to UDP server.");
        
        auto hostname   = (option("H", "hostname") & value("hostname", HostName));
        auto udpServer  = option("s", "udp-server") & value("server") >> [&](const std::string &arg) { inet_aton(arg.c_str(), &udpserver_ipaddr); };
        auto cliVersion = option("c", "cli-version") & value("client version") >> [&](const std::string &arg) { Version_len = hexStrToByte(optarg, Version, sizeof(Version)); };
        auto netTime    = (option("T", "net-time") & value("time") >> [&](const std::string &arg) {
                           if (sscanf(arg.c_str(), "%hhu:%hhu", &accessTime.a_hour, &accessTime.a_minute) != 2
                               || (accessTime.a_hour >= 24) || (accessTime.a_minute >= 60))
                               LogWrite(INIT, ERROR, "Time invalid!"); })
                           .doc("The time you are allowed to access internet. e.g. 6:10");
        auto hash = (option("h", "hash") & value("hash", Hash))
                        .doc("DrAuthSvr.dll hash value.");
        auto onlineHook = (option("E", "online-hook") & value("command", OnlineHookCmd))
                              .doc("Command to be execute after EAP authentication success.");
        auto offlineHood = (option("Q", "offline-hook") & value("command", OfflineHookCmd))
                               .doc(" Command to be execute when you are forced offline at night.");
        auto loglevel = (option("D", "debug").call([&]() { cloglev = DEBUG; }) & value("level") >> [&](const std::string &arg) { cloglev = (LOGLEVEL)std::stoi(arg); })
                            .doc("Debug log level");
        auto logoff = option("o", "logoff").set(client, LOGOFF);
        this->cli   = with_prefixes_short_long(
            "-", "--",
            username, password,
            interface, dns, hostname,
            udpServer, cliVersion,
            hash,
            onlineHook, offlineHood, netTime,
            loglevel, logoff);
    };
    ~config()              = default;
    config(const config &) = delete;

public:
    static config &getConfig()
    {
        static config instance;
        return instance;
    }

private:
    int hexStrToByte(const char *source, unsigned char *dest, int bufLen)
    {
        int i;
        unsigned char highByte, lowByte;

        for (i = 0; source[i * 2] && source[i * 2 + 1] && (i < bufLen); i++)
        {
            highByte = toupper(source[i * 2]);
            lowByte  = toupper(source[i * 2 + 1]);

            if (highByte > 0x39)
            {
                highByte -= 0x37;
            }
            else
            {
                highByte -= 0x30;
            }

            if (lowByte > 0x39)
            {
                lowByte -= 0x37;
            }
            else
            {
                lowByte -= 0x30;
            }

            dest[i] = (highByte << 4) | lowByte;
        }
        return i;
    }
};
#define conf config::getConfig()