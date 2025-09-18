#pragma once

namespace TSSM::Core
{
struct RpAtomic;
struct RpLight;

struct RwRGBAReal
{
    float red;   // offset 0x0, size 0x4
    float green; // offset 0x4, size 0x4
    float blue;  // offset 0x8, size 0x4
    float alpha; // offset 0xC, size 0x4
};

struct RwV3d
{
    float x; // offset 0x0, size 0x4
    float y; // offset 0x4, size 0x4
    float z; // offset 0x8, size 0x4
};

struct RwMatrixTag
{
    RwV3d right;        // offset 0x0, size 0xC
    unsigned int flags; // offset 0xC, size 0x4
    RwV3d up;           // offset 0x10, size 0xC
    unsigned int pad1;  // offset 0x1C, size 0x4
    RwV3d at;           // offset 0x20, size 0xC
    unsigned int pad2;  // offset 0x2C, size 0x4
    RwV3d pos;          // offset 0x30, size 0xC
    unsigned int pad3;  // offset 0x3C, size 0x4
};
}
