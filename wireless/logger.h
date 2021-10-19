#pragma once
#include <string>
#include <type_traits>
template <class T>
struct basic_type
{
    using type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
};

class loggerMessage
{
public:
    loggerMessage() : error_num(0), message(){};
    loggerMessage(int err) : error_num(err), message(){};
    template <typename T, class = typename std::enable_if<!std::is_constructible<int, T>::value>::type>
    loggerMessage(T &&meg) : error_num(0), message(std::forward<T>(meg)){};
    template <typename T>
    loggerMessage(int err, T &&meg) : error_num(err), message(std::forward<T>(meg)){};
    // ~loggerMessage(){};
    std::string what() noexcept
    {
        return message;
    }
    operator int()
    {
        return error_num;
    }

private:
    std::string message;
    int error_num;
};
