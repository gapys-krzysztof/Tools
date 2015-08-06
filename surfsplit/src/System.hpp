#ifndef __DARKRL__SYSTEM_HPP__
#define __DARKRL__SYSTEM_HPP__

class System
{
public:
    System() = delete;

    static int CPUCores();
    static void SetThreadName( const char* name );
};

#endif
