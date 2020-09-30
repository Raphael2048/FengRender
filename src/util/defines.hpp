
#pragma once

#include <stdexcept>
#include <string>
#include <system_error>
// #include <format>
namespace feng
{
    class Uncopyable
    {
    public:
        Uncopyable(){};
    private:
        Uncopyable(const Uncopyable &) = delete;
        Uncopyable &operator=(const Uncopyable &) = delete;
    };
#define FMSG(msg)                      \
    {                                  \
        throw std::runtime_error(msg); \
    }

} // namespace feng
