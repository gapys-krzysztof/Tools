#include "String.hpp"

bool ReadLine( FILE* f, std::string& line )
{
    int r;
    char c;

    for(;;)
    {
        r = fread( &c, 1, 1, f );
        if( r <= 0 || c == '\n' )
        {
            return r > 0;
        }
        if( c != '\r' )
        {
            line += c;
        }
    }
}
