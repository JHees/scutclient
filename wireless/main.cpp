#include <clipp.h>
#include <cstddef>
#include <iostream>
#include <string>

#include "logger.h"
#include "url.h"
#include "wireless.h"

using namespace clipp;
//TODO
/* 
wlan0 connect wifi: netplan or iwlist, not sure
route switch to wlan0:
    1. ifconfig eth0 down
    2. add redirect url to route
        get url ip
            popen host
        add ip and gateway to route table
            how to get gateway?
            how to add as root?

login
logout
*/
struct config
{
    std::string username;
    std::string password;
    std::string ifname      = "wlan0";
    std::string redirectUrl = "msftconnecttest.com";
} conf;
int main(int argc, char **argv)
{
    enum class mode
    {
        help,
        login,
        logout,
        debug
    };
    mode selected = mode::login;
    auto cli      = with_prefixes_short_long(
        "-", "--",
        (option("help", "h").set(selected, mode::help)) % "show this message."
            | ((required("u", "username") & value("username", conf.username)) % " ",
               (required("p", "password") & value("password", conf.password)) % " ",
               (option("i", "interface") & value("ifname", conf.ifname))
                   % ("interface name. (default: \"" + conf.ifname + "\")"),
               (option("redirectUrl") & value("url", conf.redirectUrl))
                   % ("url or address to test connection and get authentication url. (default: \" " + conf.redirectUrl + "\")"),
               option("d").set(selected, mode::debug))
            | (option("logout", "o").set(selected, mode::logout)) % "send logout to the server.");
    auto fmt = doc_formatting{}
                   .first_column(2)  //left border column for text body
                   .doc_column(28)   //column where parameter docstring starts
                   .last_column(100) //right border column for text body
        ;
    if (!parse(argc, argv, cli))
    {
        std::cout << "Error prarm.\n"
                  << make_man_page(cli, argv[0], fmt);
        return 0;
    }
    switch (selected)
    {
    case mode::login:
        wireless.setInterface(conf.ifname);
        wireless.redirect(conf.redirectUrl);
        wireless.loadconfig();
        wireless.chkstatus();
        wireless.login(conf.username, conf.password);
        break;
    case mode::logout:
        wireless.logout();
        break;
    default:
        // std::cout << make_man_page(cli, argv[0], fmt);
        break;
    }
    wireless.addRouteRable(conf.redirectUrl);
    std::cout << '\n'
              << conf.username;
}