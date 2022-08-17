#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <memory>
#include <unordered_set>
#ifndef _WIN32
#  include <unistd.h>
#endif

#include "Atlas.hpp"
#include "Bitmap.hpp"
#include "Filesystem.hpp"
#include "Node.hpp"
#include "Rect.hpp"
#include "String.hpp"
#include "Processing.hpp"

constexpr const char* APP_NAME = "TextureAtlas";
constexpr unsigned VERSION_MAJOR = 1;
constexpr unsigned VERSION_MINOR = 2;
constexpr unsigned VERSION_PATCH = 0;

std::string input;
std::string output( "." );
int size = 1024;
std::string name( "atlas" );
int edges = 0;
int path = -1;
int numAtlases = 1;
std::string pathStripPrefix;
Atlas::Flags atlasFlags = Atlas::DefaultFlags;
int align = 0;
std::string prepend;
bool noalpha = false;
bool splashfill = true;
bool allowFlip = true;
bool writeRaw = false;
bool cascadeUp = false;
bool stats = false;
bool shrink = false;
bool groupDir = false;
bool bench = false;
bool dryRun = false;
std::string i18nBase;
std::vector<std::string> i18nLangs = { "" };

typedef std::map<std::string, std::vector<BRect>> ImgRectData;
std::map<std::string, ImgRectData> imageData;
std::map<std::string, std::vector<std::string>> imagesByPath;
std::vector<std::pair<int, std::string>> imagesByMaxRectSize;
std::vector<std::string> langsToWork;

std::chrono::high_resolution_clock::time_point benchData[10];

std::vector<BRect> LoadImages( const std::vector<std::string>& pngs, const std::vector<std::string>& names, const std::vector<std::string>& rectnames )
{
    std::vector<BRect> ret;

    auto nit = names.cbegin();
    auto rit = rectnames.cbegin();

    ret.reserve( pngs.size() );
    for( auto& png : pngs )
    {
        int maxRectSize = 0;
        FILE* f = fopen( rit->c_str(), "rb" );
        Bitmap* b = nullptr;
        if( !dryRun || !f )
        {
            b = new Bitmap( png.c_str() );
        }
        if( !f )
        {
            ret.push_back( BRect( 0, 0, b->Size().x, b->Size().y, b, *nit, b->Size().x * b->Size().y ) );
        }
        else
        {
            int size;
            fread( &size, 1, 4, f );
            if( size == 0 )
            {
                ret.push_back( BRect( 0, 0, 0, 0, b, *nit ) );
            }
            else
            {
                for( int i=0; i<size; i++ )
                {
                    BRect r( 0, 0, 0, 0, b, *nit );
                    fread( &r, 1, sizeof( uint16_t ) * 4, f );
                    ret.push_back( r );
                    maxRectSize = std::max( maxRectSize, r.w * r.h );
                }

                auto rectIt = ret.rbegin();
                for( int i = 0; i < size; ++i, ++rectIt )
                {
                    rectIt->maxRectSize = maxRectSize;
                }
            }
            fclose( f );
        }
        ++nit;
        ++rit;
    }

    return ret;
}

void FindFlipped( std::vector<BRect>& images )
{
    std::for_each( images.begin(), images.end(), []( BRect& img ){ if( img.h > img.w ) { img.flip = true; std::swap( img.w, img.h ); } } );
}

std::vector <std::string> ReadFilenames( const std::string& filename )
{
    std::vector <std::string> names;
    FILE* f = fopen( input.c_str(), "r" );
    if( !f ) return names;

    std::string line;
    while( ReadLine( f, line ) )
    {
        names.push_back( line );
        line.clear();
    }

    fclose( f );
    return names;
}

bool ReadImageData( const std::vector<std::string>& langs )
{
    auto names = ReadFilenames( input );
    if( names.empty() ) return false;

    std::map <std::string, int> maxImageRect;

    for( const auto& lang : langs )
    {
        std::vector <std::string> pngnames, rectnames;
        bool skipLang = !lang.empty();
        for( const auto& name : names )
        {
            if( lang.empty() )
            {
                pngnames.push_back( name.substr( 0, name.rfind( '.' ) ) + ".png" );
                rectnames.push_back( name + ".csr" );
            }
            else
            {
                std::string png = name.substr( 0, name.rfind( '.' ) ) + ".png";
                std::string pfx = i18nBase + "/" + lang + "/";
                size_t pos = 0;
                for( int i=0; i<path; i++ )
                {
                    pos = png.find( '/', pos ) + 1;
                }

                auto tpng = prepend + png.substr( pos );
                std::string pfxpng = pfx + tpng;
                if( Exists( pfxpng ) )
                {
                    rectnames.push_back( pfxpng + ".csr" );
                    pngnames.push_back( std::move( pfxpng ) );
                    skipLang = false;
                }
                else
                {
                    pngnames.push_back( std::move( png ) );
                    rectnames.push_back( name + ".csr" );
                }
            }
        }

        if( skipLang ) continue;
        langsToWork.push_back( lang );

        if( groupDir )
        {
            for( const auto& imgPath : names )
            {
                const std::string dir = imgPath.substr( 0, imgPath.rfind( '/' ) + 1 );
                imagesByPath[dir].push_back( imgPath );
            }
        }

        std::vector<BRect> images( LoadImages( pngnames, names, rectnames ) );
        if( allowFlip )
        {
            FindFlipped( images );
        }

        for( const auto& img : images )
        {
            auto iter = maxImageRect.find( img.name );
            if( iter == maxImageRect.end() )
            {
                maxImageRect[img.name] = img.maxRectSize;
            }
            else
            {
                maxImageRect[img.name] = std::max( iter->second, img.maxRectSize );
            }

            auto& imgParts = imageData[lang][img.name];
            imgParts.push_back( img );
        }

        for( auto& imgParts : imageData[lang] )
        {
            std::stable_sort( imgParts.second.begin(), imgParts.second.end(), []( const BRect& i1, const BRect& i2 ) { if( i1.h != i2.h ) return i1.h > i2.h; else return i1.w > i2.w; } );
        }
    }

    for( const auto& p : maxImageRect )
    {
        imagesByMaxRectSize.emplace_back( p.second, p.first );
    }
    std::stable_sort( imagesByMaxRectSize.begin(), imagesByMaxRectSize.end(), []( const auto& a, const auto& b ) { return a.first > b.first; });

    return true;
}

bool DoWork()
{
    // no cascade check when creating subatlases
    if( cascadeUp && numAtlases == 1 )
    {
        bool bad = false;
        do
        {
            bad = false;
            auto test = std::make_unique<Node>( Rect( 0, 0, size, size ) );
            for( const auto& imgData : imageData.cbegin()->second )
            {
                for( const auto& img : imgData.second )
                {
                    Node* uv = test->Insert( Rect( edges, edges, img.w + edges * 2, img.h + edges * 2 ), align );
                    if( !uv )
                    {
                        size *= 2;
                        if( size > 32768 )
                        {
                            return false;
                        }
                        bad = true;
                        break;
                    }
                }
            }
        }
        while( bad );
    }

    std::vector<std::vector<Atlas>> atlases( langsToWork.size() );
    for( auto& atlasVec : atlases )
    {
        for( int i = 0; i < numAtlases; ++i )
        {
            atlasVec.emplace_back( size, size, prepend, edges, align );
        }
    }

    benchData[2] = std::chrono::high_resolution_clock::now();
    if( numAtlases == 1 )
    {
        for( size_t langIdx = 0; langIdx < langsToWork.size(); ++langIdx )
        {
            const std::string& lang = langsToWork[langIdx];
            std::vector<BRect> imgParts;
            Atlas& atlas = atlases[langIdx][0];

            for( const auto& p : imageData[lang] )
            {
                imgParts.insert( imgParts.end(), p.second.cbegin(), p.second.cend() );
            }

            std::stable_sort( imgParts.begin(), imgParts.end(), []( const BRect& i1, const BRect& i2 ) { if( i1.h != i2.h ) return i1.h > i2.h; else return i1.w > i2.w; } );

            for( const auto& img : imgParts )
            {
                if( !atlas.AddImage( img ) )
                {
                    if( lang.empty() )
                    {
                        fprintf( stderr, "Unable to fit image %s\n", img.name.c_str() );
                    }
                    else
                    {
                        fprintf( stderr, "Unable to fit image %s for language %s\n", img.name.c_str(), lang.c_str() );
                    }
                    return false;
                }
            }
        }
        if( dryRun )
        {
            return true;
        }
    }
    else
    {
        std::unordered_set<std::string> imagesProcessed;

        for( const auto& p : imagesByMaxRectSize )
        {
            if( imagesProcessed.find( p.second ) != imagesProcessed.end() ) continue;

            std::vector<std::string> imageGroup;
            if( groupDir )
            {
                const std::string dir = p.second.substr( 0, p.second.rfind( '/' ) + 1 );
                imageGroup = imagesByPath[dir];
            }
            else
            {
                imageGroup.push_back( p.second );
            }

            std::vector<std::vector<BRect>> imgParts( langsToWork.size() );
            for( unsigned langIdx = 0; langIdx < langsToWork.size(); ++langIdx )
            {
                const auto& lang = langsToWork[langIdx];
                auto& ipVec = imgParts[langIdx];

                for( const auto& imgName : imageGroup )
                {
                    const auto& id = imageData[lang][imgName];
                    ipVec.insert( ipVec.end(), id.cbegin(), id.cend() );
                }

                std::stable_sort( ipVec.begin(), ipVec.end(), []( const BRect& i1, const BRect& i2 ) { if( i1.h != i2.h ) return i1.h > i2.h; else return i1.w > i2.w; } );
            }

            bool fits = false;
            int atlasIdx = 0;
            while( !fits && atlasIdx < numAtlases )
            {
                fits = true;

                for( size_t langIdx = 0; langIdx < langsToWork.size() && fits; ++langIdx )
                {
                    const auto& ipVec = imgParts[langIdx];
                    if( ipVec.empty() ) continue;

                    Atlas& atlas = atlases[langIdx][atlasIdx];
                    if( !atlas.AssetFits( ipVec ) )
                    {
                        const bool optimized = atlas.Optimize();
                        if( !optimized || !atlas.AssetFits( ipVec ) )
                        {
                            fits = false;
                            ++atlasIdx;
                            break;
                        }
                    }
                }
            }

            if( !fits )
            {
                fprintf( stderr, "Unable to fit images in specified number of atlases (%d)\n", numAtlases );
                return false;
            }

            for( size_t langIdx = 0; langIdx < langsToWork.size(); ++langIdx )
            {
                Atlas& atlas = atlases[langIdx][atlasIdx];
                for( const auto& r : imgParts[langIdx] )
                {
                    if( !atlas.AddImage( r ) )
                    {
                        abort();
                    }
                }
            }

            for( const auto& img : imageGroup )
            {
                imagesProcessed.insert( img );
            }
        }

        if( dryRun )
        {
            return true;
        }

        for( int atlasIdx = 0; atlasIdx < numAtlases; ++atlasIdx )
        {
            for( size_t langIdx = 0; langIdx < langsToWork.size(); ++langIdx )
            {
                Atlas& atlas = atlases[langIdx][atlasIdx];
                atlas.Optimize();
            }
        }
    }
    benchData[3] = std::chrono::high_resolution_clock::now();

    for( size_t langIdx = 0; langIdx < langsToWork.size(); ++langIdx )
    {
        const std::string& lang = langsToWork[langIdx];

        for( int atlasIdx = 0; atlasIdx < numAtlases; ++atlasIdx )
        {
            std::string xmlName;
            std::string bitmapName;
            std::string atlasNumStr( atlasIdx == 0 ? "" : std::to_string( atlasIdx + 1 ) );
            if( lang.empty() )
            {
                xmlName = output + "/" + name + atlasNumStr + ".xml";
                bitmapName = output + "/" + name + atlasNumStr;
            }
            else
            {
                xmlName = output + "/" + lang + "_" + name + atlasNumStr + ".xml";
                bitmapName = output + "/" + lang + "_" + name + atlasNumStr;
            }

            Atlas& atlas = atlases[langIdx][atlasIdx];
            if( shrink )
            {
                while( atlas.Shrink() );
            }

            atlas.AdjustSize( atlasFlags );
            benchData[4] = std::chrono::high_resolution_clock::now();
            atlas.BlitImages();
            benchData[5] = std::chrono::high_resolution_clock::now();
            if( !atlas.Output( bitmapName, xmlName, pathStripPrefix, path, atlasFlags ) ) return false;

            if( stats )
            {
                printf( "[%s]\n", bitmapName.c_str() );
                atlas.ReportStats();
            }
        }
    }

    return true;
}

void Usage()
{
    printf( R"___(Usage: TextureAtlas [options]

    -v, --version      displays version
    -i, --input        input text file with input files list
    -o, --output       output path (default: current dir)
    -s, --size         target atlas size (default: 1024)
    -e, --edges        duplicate image edges (default: 0)
    -n, --name         name of generated files (default: atlas)
    -d, --divide       creates up to N subatlases if images don't fit in a single one (default: 1)
    -h, --help         prints this message
    -P, --path         path strip depth
    --strip-prefix PFX prefix to be stripped from all asset paths (makes -P option
                       ineffective; executed before path prepending action)
    -W, --potw         make width of atlas a power of two
    -H, --poth         make height of atlas a power of two
    -a, --align        align textures (default: disabled)
    -p, --prepend      prepend given string to all asset paths
    -q, --square       make width equal to height
    -N, --noalpha      no alpha channel
    --nosplashfill     disable splash fill
    --noflip           disable fragment flipping
    --raw              write raw data
    --shrink           try to shrink the resulting atlases (only for square atlases)
    -c, --cascade      try bigger atlas size, if data does not fit (disabled if -d is set)
    --stats            print stats
    -O, --override     i18n override directory
    --groupdir         group assets by directory path
    --bench            display timing statistics
    --dryrun           do not generate atlas, only check if images fit in
)___");
}

void Error()
{
    Usage();
    exit( 1 );
}

int main( int argc, char** argv )
{
#define CSTR(x) strcmp( argv[i], x ) == 0

    for( int i=1; i<argc; i++ )
    {
        if( CSTR( "-v" ) || CSTR( "--version" ) )
        {
            printf( "%s %u.%u.%u", APP_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH );
            exit( 0 );
        }
        if( CSTR( "-i" ) || CSTR( "--input" ) )
        {
            input = argv[i+1];
            i++;
        }
        else if( CSTR( "-o" ) || CSTR( "--output" ) )
        {
            output = argv[i+1];
            i++;
        }
        else if( CSTR( "-s" ) || CSTR( "--size" ) )
        {
            size = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-e" ) || CSTR( "--edges" ) )
        {
            edges = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-n" ) || CSTR( "--name" ) )
        {
            name = argv[i+1];
            i++;
        }
        else if( CSTR( "-d" ) || CSTR( "--divide" ) )
        {
            numAtlases = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-h" ) || CSTR( "--help" ) )
        {
            Usage();
            return 0;
        }
        else if( CSTR( "-P" ) || CSTR( "--path" ) )
        {
            path = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "--strip-prefix" ) )
        {
            if (argc <= i+1)
            {
                fprintf(stderr, "\nTextureAtlas: error: Missing --strip-prefix option argument.\n");
                exit(1);
            }

            pathStripPrefix = argv[i+1];
            ++i;
        }
        else if( CSTR( "-W" ) || CSTR( "--potw" ) )
        {
            atlasFlags |= Atlas::POTWidth;
        }
        else if( CSTR( "-H" ) || CSTR( "--poth" ) )
        {
            atlasFlags |= Atlas::POTHeight;
        }
        else if( CSTR( "-a" ) || CSTR( "--align" ) )
        {
            align = atoi( argv[i+1] );
            i++;
        }
        else if( CSTR( "-p" ) || CSTR( "--prepend" ) )
        {
            prepend = argv[i+1];
            i++;
        }
        else if( CSTR( "-q" ) || CSTR( "--square" ) )
        {
            atlasFlags |= Atlas::Square;
        }
        else if( CSTR( "-N" ) || CSTR( "--noalpha" ) )
        {
            atlasFlags |= Atlas::NoAlpha;
        }
        else if( CSTR( "--nosplashfill" ) )
        {
            atlasFlags &= ~Atlas::SplashFill;
        }
        else if( CSTR( "--noflip" ) )
        {
            allowFlip = false;
        }
        else if( CSTR( "--raw" ) )
        {
            atlasFlags |= Atlas::WriteRaw;
        }
        else if( CSTR( "--shrink" ) )
        {
            shrink = true;
        }
        else if( CSTR( "-c" ) || CSTR( "--cascade" ) )
        {
            cascadeUp = true;
        }
        else if( CSTR( "--stats" ) )
        {
            stats = true;
        }
        else if( CSTR( "-O" ) || CSTR( "--override" ) )
        {
            i18nBase = argv[i+1];
            const auto tmp = ListDirectory( i18nBase );
            for( auto& v : tmp )
            {
                i18nLangs.emplace_back( v );
            }
            i++;
        }
        else if( CSTR( "--groupdir" ) )
        {
            groupDir = true;
        }
        else if( CSTR( "--bench" ) )
        {
            bench = true;
        }
        else if( CSTR( "--dryrun" ) )
        {
            dryRun = true;
        }
        else
        {
            fprintf(stderr, "\nTextureAtlas: error: unknown argument: %s\n", argv[i]);
            Error();
        }
    }

#undef CSTR

    benchData[0] = std::chrono::high_resolution_clock::now();
    if( !ReadImageData( i18nLangs ) )
    {
        fprintf( stderr, "Unable to read image data, exiting now...\n" );
        return 1;
    }
    benchData[1] = std::chrono::high_resolution_clock::now();

    if( !DoWork() )
    {
        fprintf( stderr, "Atlas creation failed\n" );

        for( const std::string& lang : i18nLangs )
        {
            std::string base;
            if( lang.empty() )
            {
                base = output + "/" + name;
            }
            else
            {
                base = output + "/" + lang + "_" + name;
            }
            unlink( ( base + ".png" ).c_str() );
            unlink( ( base + ".raw" ).c_str() );
            unlink( ( base + ".xml" ).c_str() );
        }

        return 1;
    }

    printf( "done!\n" );

    if( bench )
    {
        printf( "Benchmark data:\n" );
        printf( "  Images loading time: %.3f ms\n", std::chrono::duration_cast<std::chrono::microseconds>( benchData[1] - benchData[0] ).count() * 0.001f );
        printf( "  Fitting: %.3f ms\n", std::chrono::duration_cast<std::chrono::microseconds>( benchData[3] - benchData[2] ).count() * 0.001f );
        printf( "  Blitting: %.3f ms\n", std::chrono::duration_cast<std::chrono::microseconds>( benchData[5] - benchData[4] ).count() * 0.001f );
        printf( "  Splash fill: %.3f ms\n", std::chrono::duration_cast<std::chrono::microseconds>( benchData[7] - benchData[6] ).count() * 0.001f );
        printf( "  Saving atlas: %.3f ms\n", std::chrono::duration_cast<std::chrono::microseconds>( benchData[9] - benchData[8] ).count() * 0.001f );
    }

    return 0;
}
