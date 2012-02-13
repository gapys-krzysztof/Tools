#include "Processing.hpp"

void SplashFill( Bitmap* bmp )
{
    Bitmap* tmp = new Bitmap( *bmp );

    const v2i s = bmp->Size();
    const int32 offsetTable[8] = { -1, 1, -s.x, s.x, -s.x-1, -s.x+1, s.x-1, s.x+1 };

    uint32* ptr = tmp->Data();
    for( int i=0; i<s.x*s.y; i++ )
    {
        if( ( *ptr & 0xFF000000 ) != 0 )
        {
            *ptr = 0xFF000000 | ( *ptr & 0x00FFFFFF );
        }
        ptr++;
    }

    for(;;)
    {
        int modified = 0;

        uint32* ptr = tmp->Data() + s.x + 1;

        for( int i=1; i<s.y-1; i++ )
        {
            for( int j=1; j<s.x-1; j++ )
            {
                if( ( *ptr & 0xFF000000 ) == 0 )
                {
                    modified++;

                    int t = 0;
                    uint32 r = 0;
                    uint32 g = 0;
                    uint32 b = 0;

                    for( int k=0; k < sizeof( offsetTable ) / sizeof( int32 ); k++ )
                    {
                        uint32* p = ptr + offsetTable[k];
                        if( ( *p & 0xFF000000 ) == 0xFF000000 )
                        {
                            t++;
                            r += *p & 0x00FF0000;
                            g += *p & 0x0000FF00;
                            b += *p & 0x000000FF;
                        }
                    }

                    if( t != 0 )
                    {
                        r = ( r/t ) & 0x00FF0000;
                        g = ( g/t ) & 0x0000FF00;
                        b = ( b/t ) & 0x000000FF;
                        *ptr = 0x88000000 | r | g | b;
                    }
                }

                ptr++;
            }
            ptr += 2;
        }

        if( modified == 0 )
        {
            break;
        }

        ptr = tmp->Data();
        for( int i=0; i<s.x*s.y; i++ )
        {
            if( ( *ptr & 0xFF000000 ) == 0x88000000 )
            {
                *ptr = 0xFF000000 | ( *ptr & 0x00FFFFFF );
            }
            ptr++;
        }
    }

    ptr = tmp->Data();
    uint32* dst = bmp->Data();

    for( int i=0; i<s.x*s.y; i++ )
    {
        *dst = ( *dst & 0xFF000000 ) | ( *ptr & 0x00FFFFFF );
        ptr++;
        dst++;
    }


    delete tmp;
}
