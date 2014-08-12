#ifndef __INCLUDED_CLAW_PVRHEADER_HPP__
#define __INCLUDED_CLAW_PVRHEADER_HPP__

struct PvrHeaderV2
{
    unsigned int HeaderSize;
    unsigned int Height;
    unsigned int Width;
    unsigned int MipMapCount;
    unsigned int Flags;
    unsigned int DataSize;
    unsigned int BitCount;
    unsigned int RBitMask;
    unsigned int GBitMask;
    unsigned int BBitMask;
    unsigned int ABitMask;
    char FourCC[4];
    unsigned int NumSurfs;
};

struct PvrHeaderV3
{
    unsigned int Version;
    unsigned int Flags;
    unsigned int PixelFormat[2];
    unsigned int ColourSpace;
    unsigned int ChannelType;
    unsigned int Height;
    unsigned int Width;
    unsigned int Depth;
    unsigned int NumSurfs;
    unsigned int NumFaces;
    unsigned int MipMapCount;
    unsigned int MetaDataSize;
};

enum { H_PVR3 = 0x03525650 };
enum { H_3PVR = 0x50565203 };

#endif
