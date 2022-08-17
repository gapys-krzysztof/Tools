#ifndef __INCLUDED_CLAW_DDSHEADER_HPP__
#define __INCLUDED_CLAW_DDSHEADER_HPP__


struct DdsHeader
{
    unsigned int Magic;
    unsigned int Size;
    unsigned int Flags;
    unsigned int Height;
    unsigned int Width;
    unsigned int PitchOrLinearSize;
    unsigned int Depth;
    unsigned int MipMapCount;
    unsigned int Reserved1[ 11 ];

    //  DDPIXELFORMAT
    struct
    {
        unsigned int Size;
        unsigned int Flags;
        char FourCC[4];
        unsigned int RGBBitCount;
        unsigned int RBitMask;
        unsigned int GBitMask;
        unsigned int BBitMask;
        unsigned int AlphaBitMask;
    } PixelFormat;

    //  DDCAPS2
    struct
    {
        unsigned int Caps1;
        unsigned int Caps2;
        unsigned int DDSX;
        unsigned int Reserved;
    } Caps;
    unsigned int Reserved2;
};

struct DdsHeader10
{
    unsigned int Format;
    unsigned int Dimension;
    unsigned int MiscFlag;
    unsigned int ArraySize;
    unsigned int Reserved;
};

#endif
