#pragma once
#include <chrono>
#include <string>

class ProfilingTimer
{
    typedef std::chrono::steady_clock clock;
    // typedef std::chrono::system_clock clock;
    typedef std::chrono::time_point<clock> time_point;

public:
    
    explicit  ProfilingTimer(std::string name_) :
        start_tp(clock::now()),
        name(std::move(name_))
    {}

    ProfilingTimer(const ProfilingTimer & _) = delete;
    ProfilingTimer(ProfilingTimer && _) = delete;
    ProfilingTimer& operator = (const ProfilingTimer &_) = delete;
    ProfilingTimer& operator = (const ProfilingTimer &&_) = delete;
    
    ~ProfilingTimer();
    
    static void print_stats();
private:
    
    const time_point start_tp;    
    const std::string name;
    
};

std::string inline clean_func_name(const std::string & func_name)
{
    size_t pos = func_name.find("@");
    if (pos == std::string::npos)
    {
        return func_name;
    }

    return func_name.substr(0, pos);
}

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)
#define VARNAME_LINE_ID(Var) CAT(Var, __LINE__)

// about 1.5 times faster than PROFILE() 
#define PROFILE_NAMED(NAME) ProfilingTimer VARNAME_LINE_ID(timer)(std::string(NAME) + ":" + std::to_string(__LINE__))

#define PROFILE() PROFILE_NAMED(__FUNCDNAME__)



// #define PROFILE() ProfilingTimer VARNAME_LINE_ID(timer)(std::string(__FUNCDNAME__) + ":" + std::to_string(__LINE__))
// #define PROFILE(STR) ProfilingTimer VARNAME_LINE_ID(timer) ((std::string(STR)))