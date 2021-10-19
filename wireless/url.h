#pragma once
#include <regex>
#include <string>
#include <type_traits>
class urlStr
{
public:
    urlStr(){};
    template <typename T, class = typename std::enable_if<std::is_same<T, std::string>::value || std::is_same<T, char *>::value>::type>
    urlStr(T &&u) : urls(std::forward<T>(u)){};
    template <class T>
    urlStr &setURL(T &&u)
    {
        urls = u;
        return *this;
    };
    template <class T, class U>
    urlStr &addParam(T &&name, U &&var)
    {
        param += "&" + replaceSymbol(name) + "=" + replaceSymbol(var);
        return *this;
    }
    std::string url()
    {
        if (param[0] == '&') param.erase(param.begin());
        return urls + "?" + param;
    }
    void clear() noexcept
    {
        urls.clear();
        param.clear();
    }

private:
    std::string urls;
    std::string param;
    template <class T>
    static std::string replaceSymbol(T &&s)
    {
        std::string str(s);
        str = std::regex_replace(str, std::regex("[+]"), "%2B");
        str = std::regex_replace(str, std::regex("[ ]"), "%20");
        str = std::regex_replace(str, std::regex("[/]"), "%2F");
        str = std::regex_replace(str, std::regex("[?]"), "%3F");
        str = std::regex_replace(str, std::regex("[%]"), "%25");
        str = std::regex_replace(str, std::regex("[#]"), "%23");
        str = std::regex_replace(str, std::regex("[&]"), "%26");
        str = std::regex_replace(str, std::regex("[=]"), "%3D");
        return str;
    }
};