/* File: main.c
 * ------------
 * 校园网802.1X客户端命令行
 */
#include "auth.h"
#include "clipp.h"
#include "info.h"
#include "parsers.h"
#include "tracelog.h"
#include <getopt.h>
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

int main(int argc, char *argv[])
{
    LogWrite(ALL, INF, "scutclient built at: " __DATE__ " " __TIME__);
    LogWrite(ALL, INF, "Authored by Scutclient Project");
    LogWrite(ALL, INF, "Source code available at https://github.com/scutclient/scutclient");
    LogWrite(ALL, INF, "Contact us with QQ group 262939451");
    LogWrite(ALL, INF, "#######################################");

    int ret;
    unsigned int retry_time = 1;
    time_t ctime;
    struct tm *cltime;
    if (!clipp::parse(argc, argv, cli))
        std::cout << make_man_page(cli, argv[0]);

    if (conf.HostName[0] == 0)
    {
        char host[32] = {0};
        gethostname(host, sizeof(host));
        conf.HostName = host;
    }

    if ((conf.client != LOGOFF) && !(conf.UserName.empty() && conf.Password.empty()))
    {
        LogWrite(INIT, ERROR, "Please specify username and password!");
        exit(-1);
    }
    if (conf.udpserver_ipaddr.s_addr == 0)
        inet_aton(SERVER_ADDR, &conf.udpserver_ipaddr);
    if (conf.dns_ipaddr.s_addr == 0)
        inet_aton(DNS_ADDR, &conf.dns_ipaddr);

    /* 配置退出登录的signal handler */
    sa_term.sa_handler = &handle_term;
    sa_term.sa_flags = SA_RESETHAND;
    sigfillset(&sa_term.sa_mask);
    sigaction(SIGTERM, &sa_term, NULL);
    sigaction(SIGINT, &sa_term, NULL);

    /* 调用子函数完成802.1X认证 */
    while (1)
    {
        ret = Authentication(conf.client);
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
        else if (timeNotAllowed && (conf.accessTime.a_minute < 60))
        {
            timeNotAllowed = 0;
            ctime = time(NULL);
            cltime = localtime(&ctime);
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
