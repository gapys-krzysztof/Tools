#ifdef _WIN32
#  include <windows.h>
#else
#  include <pthread.h>
#  include <unistd.h>
#endif

#include "System.hpp"

int System::CPUCores()
{
    static int cores = 0;
    if( cores == 0 )
    {
#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo( &info );
        cores = (int)info.dwNumberOfProcessors;
#else
#  ifndef _SC_NPROCESSORS_ONLN
#    ifdef _SC_NPROC_ONLN
#      define _SC_NPROCESSORS_ONLN _SC_NPROC_ONLN
#    elif defined _SC_CRAY_NCPU
#      define _SC_NPROCESSORS_ONLN _SC_CRAY_NCPU
#    endif
#  endif
        cores = (int)(long)sysconf( _SC_NPROCESSORS_ONLN );
#endif
        if( cores <= 0 )
        {
            cores = 1;
        }
    }
    return cores;
}

void System::SetThreadName( const char* name )
{
#if defined _WIN32
    const DWORD MS_VC_EXCEPTION=0x406D1388;

#  pragma pack( push, 8 )
    struct THREADNAME_INFO
    {
       DWORD dwType;
       LPCSTR szName;
       DWORD dwThreadID;
       DWORD dwFlags;
    };
#  pragma pack(pop)

    DWORD ThreadId = GetThreadId( GetCurrentThread() );
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = ThreadId;
    info.dwFlags = 0;

    __try
    {
       RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
#elif defined __APPLE__
    pthread_setname_np( name );
#else
    pthread_setname_np( pthread_self(), name );
#endif
}
