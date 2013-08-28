#include "Bitmap.hpp"

int main( int argc, char** argv )
{
    Bitmap bmp( argv[1] );
    bmp.Write( "out.png", false );
    bmp.AlphaFill();
    bmp.Write( "outa.png", false );

    return 0;
}
