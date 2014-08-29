#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#  include <windows.h>
#else
#  include <dirent.h>
#endif

#include "VFS.hpp"

bool CreateDirStruct( const std::string& path )
{
    struct stat buf;
    if( stat( path.c_str(), &buf ) == 0 )
    {
        return true;
    }

    if( errno != ENOENT )
    {
        printf( "%s", strerror( errno ) );
        assert( false );
        return false;
    }

    size_t pos = 0;
    do
    {
        pos = path.find( '/', pos+1 );
#ifdef _WIN32
        if( pos == 2 ) continue;    // Don't create drive name.
        if( _mkdir( path.substr( 0, pos ).c_str() ) != 0 )
#else
        if( mkdir( path.substr( 0, pos ).c_str(), S_IRWXU ) != 0 )
#endif
        {
            if( errno != EEXIST )
            {
                printf( "%s\n", strerror( errno ) );
                assert( false );
                return false;
            }
        }
    }
    while( pos != std::string::npos );

    return true;
}
