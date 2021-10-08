/* File: main.c
 * ------------
 * 校园网802.1X客户端命令行
 */
#include "auth.h"
#include "clipp.h"
#include "config.h"
#include "tracelog.h"
#include <iostream>
#include <signal.h>
using namespace clipp;
struct sigaction sa_term;

void handle_term(int signal)
{
    LogWrite(ALL, INF, "Exiting...");
    auth_8021x_Logoff();
    exit(0);
}
config conf;

int main(int argc, char *argv[])
{
    LogWrite(ALL, INF, "scutclient built at: " __DATE__ " " __TIME__);
    LogWrite(ALL, INF, "Authored by Scutclient Project");
    LogWrite(ALL, INF, "Source code available at https://github.com/scutclient/scutclient");
    LogWrite(ALL, INF, "Contact us with QQ group 262939451");
    LogWrite(ALL, INF, "#######################################");

    auto cli = with_prefixes_short_long(
        "-", "--",
        (option("h", "help").call([]() { conf.client = config::HELPMODE; }))
                % "Show this message."
            | (
                required("u", "username") & value("username", conf.UserName),
                required("p", "password") & value("password", conf.Password),
                (option("i", "interface") & value("ifname", conf.DeviceName))
                    % "Interface to perform authentication.",
                (option("n", "dns")
                 & value("dns")
                       >> [](const std::string &arg) { inet_aton(arg.c_str(), &conf.dns_ipaddr); })
                    % "DNS server address to be sent to UDP server.",
                (option("H", "hostname")
                 & value("hostname", conf.HostName))
                    % " ",
                (option("s", "udp-server")
                 & value("server")
                       >> [](const std::string &arg) { inet_aton(arg.c_str(), &conf.udpserver_ipaddr); })
                    % " ",
                option("c", "cli-version")
                    & value("client version")
                          >> [](const std::string &arg) { conf.Version_len = hexStrToByte(arg.c_str(), conf.Version, sizeof(conf.Version)); },
                (option("T", "net-time")
                 & value("time")
                       >> [](const std::string &arg) {if (sscanf(arg.c_str(), "%hhu:%hhu", &conf.accessTime.a_hour, &conf.accessTime.a_minute) != 2
                                                            || (conf.accessTime.a_hour >= 24) || (conf.accessTime.a_minute >= 60))
                                                        LogWrite(INIT, ERROR, "Time invalid!"), exit(-1); })
                    % "The time you are allowed to access internet. e.g. 6:10",
                (option("h", "hash") & value("hash", conf.Hash))
                    % "DrAuthSvr.dll hash value.",
                (option("E", "online-hook") & value("command", conf.OnlineHookCmd))
                    % "Command to be execute after EAP authentication success.",
                (option("Q", "offline-hook") & value("command", conf.OfflineHookCmd))
                    % " Command to be execute when you are forced offline at night.",
                (option("D", "debug").call([]() { cloglev = DEBUG; })
                 & value("level")
                       >> [](const std::string &arg) { cloglev = (LOGLEVEL)std::stoi(arg); })
                    % "Debug log level")
            | option("o", "logoff")
                      .call([]() { conf.client = config::LOGOFF; })
                  % " ");
    auto fmt = doc_formatting{}
                   .first_column(2)  //left border column for text body
                   .doc_column(28)   //column where parameter docstring starts
                   .last_column(100) //right border column for text body
        ;
    auto res = clipp::parse(argc, argv, cli);
    if (!res || conf.client == config::HELPMODE)
    {
        std::cout << make_man_page(cli, argv[0], fmt);
        if (!res.missing().empty())
        {
            LogWrite(INIT, ERROR, "Please specify username and password!");
            exit(-1);
        }
        return 0;
    }

    /* 配置退出登录的signal handler */
    sa_term.sa_handler = &handle_term;
    sa_term.sa_flags   = SA_RESETHAND;
    sigfillset(&sa_term.sa_mask);
    sigaction(SIGTERM, &sa_term, NULL);
    sigaction(SIGINT, &sa_term, NULL);

    /* 调用子函数完成802.1X认证 */
    while (1)
    {
        int ret                 = Authentication(conf.client);
        unsigned int retry_time = 1;

        if (ret == 1)
        {
            retry_time = 1;
            LogWrite(ALL, INF, "Restart authentication.");
        }
        else if (ret == -ENETUNREACH)
        {
            LogWrite(ALL, INF, "Retry in %d secs.", retry_time);
            sleep(retry_time);
            if (retry_time <= 256)
                retry_time *= 2;
        }
        else if (timeNotAllowed)
        {
            timeNotAllowed = 0;
            auto ctime     = time(NULL);
            auto cltime    = localtime(&ctime);
            if (((int)conf.accessTime.a_hour * 60 + conf.accessTime.a_minute) > ((int)(cltime->tm_hour) * 60 + cltime->tm_min))
            {
                LogWrite(ALL, INF, "Waiting till %02hhd:%02hhd. Have a good sleep...", conf.accessTime.a_hour, conf.accessTime.a_minute);
                if (conf.OfflineHookCmd.empty())
                {
                    system(conf.OfflineHookCmd.c_str());
                }
                sleep((((int)conf.accessTime.a_hour * 60 + conf.accessTime.a_minute) - ((int)(cltime->tm_hour) * 60 + cltime->tm_min)) * 60 - cltime->tm_sec);
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    LogWrite(ALL, ERROR, "Exit.");
    return 0;
}
