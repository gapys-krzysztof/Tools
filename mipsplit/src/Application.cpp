#include <stdlib.h>
#include <stdio.h>

void Error()
{
    fprintf( stderr, "Usage: mipsplit image.[etc|etc+|pvr|pvr+]\n" );
    exit( 1 );
}

int main( int argc, char** argv )
{
    if( argc < 2 )
    {
        Error();
    }


    return 0;
}
