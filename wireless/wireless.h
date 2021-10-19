#pragma once
#include "logger.h"
#include "url.h"
#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <regex>
#include <stdio.h>

namespace base64
{
    static std::string decode(const std::string &val)
    {
        using namespace boost::archive::iterators;
        using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
        return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
            return c == '\0';
        });
        
    }

    static std::string encode(const std::string &val)
    {
        using namespace boost::archive::iterators;
        using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
        auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
        return tmp.append((3 - val.size() % 3) % 3, '=');
    }
} // namespace base64
class wireless
{
private:
    wireless()
    {
        curl_global_init(CURL_GLOBAL_ALL);
        handle = curl_easy_init();
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, Callback);
    };
    ~wireless()
    {
        curl_easy_cleanup(handle);
    };

public:
    static void setInterface(const std::string &ifname)
    {
        curl_easy_setopt(instance().handle, CURLOPT_INTERFACE, ifname.c_str());
    }
    static wireless &instance() noexcept
    {
        static wireless instance;
        return instance;
    }
    std::string getCommand(const char *const cmd)
    {
        FILE *fp = popen(cmd, "r");
        std::string ret;
        if (fp != nullptr)
        {
            char buf[1024];
            while (fgets(buf, sizeof(buf), fp) != NULL)
            {
                ret.append(buf);
            }
        }
        return ret;
    }
    template <class T>
    loggerMessage addRouteRable(T &&inputUrl)
    {
        std::regex ipRegex("^\\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$");
        std::string ip;
        if (std::regex_match(inputUrl, ipRegex))
            ip = inputUrl;
        else
        {
            std::string cmd("host ");
            cmd += inputUrl;
            std::string returnstr = getCommand(cmd.c_str());
            ipRegex               = std::regex("\\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}");
            std::smatch cm;
            if (std::regex_search(returnstr, cm, ipRegex))
                ip = cm[0];
            else
                return loggerMessage("url not found.");
        }
        std::cout << ip;
    }

    loggerMessage redirect(const std::string &redirect)
    {
        curl_easy_setopt(handle, CURLOPT_URL, redirect.c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &redirectUrl);

        auto success = curl_easy_perform(handle);
        if (success != CURLE_OK)
            return loggerMessage(success);
        std::smatch cm;
        if (std::regex_search(redirectUrl, cm, std::regex("<NextURL>(.*)</NextURL>")))
        {
            redirectUrl = cm[1];
            std::regex_search(redirectUrl, cm, std::regex("source-address=([^&]*)"));
            wlan_user_ip = cm[1];
            std::regex_search(redirectUrl, cm, std::regex("wlanacname=([^&]*)"));
            wlan_ac_name = cm[1];
            std::regex_search(redirectUrl, cm, std::regex("wlanacip=([^&]*)"));
            wlan_ac_ip = cm[1];
            std::regex_search(redirectUrl, cm, std::regex("source-mac=([^&]*)"));
            wlan_user_mac = cm[1];
            std::regex_search(redirectUrl, cm, std::regex("ssid=([^&]*)"));
            ssid = cm[1];
        }

        std::cout << "redirect url: " << redirectUrl << '\n'
                  << "wlan user ip: " << wlan_user_ip << '\n'
                  << "wlan user mac: " << wlan_user_mac << '\n'
                  << "wlan ac name: " << wlan_ac_name << '\n'
                  << "wlan ac ip: " << wlan_ac_ip << '\n';

        // return loggerMessage(wlan_ac_ip.empty() && wlan_user_ip.empty());
    }
    loggerMessage loadconfig()
    {
        url1.setURL("https://s.scut.edu.cn:802/eportal/portal/page/loadConfig")
            .addParam("callback", "dr1001")
            .addParam("program_index", "ZPDqaE1622579936")
            .addParam("wlan_vlan_id", "0")
            .addParam("wlan_user_ip", base64::encode(wlan_user_ip))
            .addParam("wlan_user_ipv6", "")
            .addParam("wlan_user_ssid", ssid)
            .addParam("wlan_user_areaid", "")
            .addParam("wlan_ac_ip", base64::encode(wlan_ac_ip))
            .addParam("wlan_ap_mac", "000000000000")
            .addParam("gw_ip", "000000000000")
            .addParam("jsVersion", "4.1.3");
        // url1.addParam("v", "7452");
        url1.addParam("lang", "zh");
        curl_easy_setopt(handle, CURLOPT_URL, url1.url().c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &load1);
        auto success = curl_easy_perform(handle);
        return loggerMessage(success);
    }
    loggerMessage chkstatus()
    {
        url2.setURL("https://s.scut.edu.cn/drcom/chkstatus")
            .addParam("callback", "dr1002")
            .addParam("jsVersion", "4.1.3")
            .addParam("lang", "zh");
        curl_easy_setopt(handle, CURLOPT_URL, url2.url().c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &load2);
        auto success = curl_easy_perform(handle);
        return loggerMessage(success);
    }
    template <class T, class U>
    loggerMessage login(T &&username, U &&password)
    {
        url3.setURL("https://s.scut.edu.cn:802/eportal/portal/login")
            .addParam("callback", "dr1003")
            .addParam("login_method", "1")
            .addParam("user_account", std::forward<T>(username))  // user account
            .addParam("user_password", std::forward<U>(password)) // user password
            .addParam("wlan_user_ip", wlan_user_ip)
            .addParam("wlan_user_ipv6", "")
            .addParam("wlan_ap_mac", "000000000000")
            .addParam("wlan_ac_ip", wlan_ac_ip)
            .addParam("wlan_ac_name", wlan_ac_name)
            .addParam("jsVersion", "4.1.3")
            .addParam("lang", "zh-cn")
            .addParam("lang", "zh");
        curl_easy_setopt(handle, CURLOPT_URL, url3.url().c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &load3);
        auto success = curl_easy_perform(handle);
        return loggerMessage(success);
    }
    loggerMessage logout()
    {
        url4.setURL("https://s.scut.edu.cn:802/eportal/portal/logout")
            .addParam("callback", "dr1004")
            .addParam("login_method", "1")
            .addParam("user_account", "drcom")
            .addParam("user_password", "123")
            .addParam("ac_logout", "1")
            .addParam("register_mode", "1")
            .addParam("wlan_user_ip", wlan_user_ip)
            .addParam("wlan_user_ipv6", "")
            .addParam("wlan_vlan_id", "0")
            .addParam("wlan_ap_mac", "000000000000")
            .addParam("wlan_ac_ip", wlan_ac_ip)
            .addParam("wlan_ac_name", wlan_ac_name)
            .addParam("jsVersion", "4.1.3")
            .addParam("lang", "zh");
        curl_easy_setopt(handle, CURLOPT_URL, url4.url().c_str());
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &load4);
        auto success = curl_easy_perform(handle);
        return loggerMessage(success);
    }
    static size_t Callback(void *data, size_t size, size_t nmemb, void *userp)
    {
        size_t realsize       = size * nmemb;
        *(std::string *)userp = std::string((char *)data, realsize);
        return realsize;
    }

private:
    CURL *handle;
    // std::string connecttestUrl;
    std::string redirectUrl; //redirect url
    std::string load1;
    std::string load2;
    std::string load3;
    std::string load4;
    std::string wlan_user_ip;
    std::string wlan_ac_name;
    std::string wlan_ac_ip;
    std::string wlan_user_mac;
    std::string ssid;
    urlStr url1;
    urlStr url2;
    urlStr url3;
    urlStr url4;
};
auto &wireless = wireless::instance();