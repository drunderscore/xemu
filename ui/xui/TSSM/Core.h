#pragma once

#include "GuestPtr.h"
#include "RenderWare.h"

namespace TSSM::Core
{
struct xMemPool
{
    GuestPtr<void> FreeList;   // offset 0x0, size 0x4
    unsigned short NextOffset; // offset 0x4, size 0x2
    unsigned short Flags;      // offset 0x6, size 0x2
    GuestPtr<void> UsedList;   // offset 0x8, size 0x4
    // void (*InitCB)(xMemPool*, void*); // offset 0xC, size 0x4
    GuestPtr<void> InitCB;
    GuestPtr<void> Buffer;     // offset 0x10, size 0x4
    unsigned short Size;       // offset 0x14, size 0x2
    unsigned short NumRealloc; // offset 0x16, size 0x2
    unsigned int Total;        // offset 0x18, size 0x4
};

struct xVec3
{
    float x, y, z;
};

struct xLinkAsset
{
    unsigned short srcEvent;         // offset 0x0, size 0x2
    unsigned short dstEvent;         // offset 0x2, size 0x2
    unsigned int dstAssetID;         // offset 0x4, size 0x4
    float param[4];                  // offset 0x8, size 0x10
    unsigned int paramWidgetAssetID; // offset 0x18, size 0x4
    unsigned int chkAssetID;         // offset 0x1C, size 0x4
};

struct xBase
{
public:
    GuestPtr<void> vtable;
    unsigned int id;           // offset 0x0, size 0x4
    unsigned char baseType;    // offset 0x4, size 0x1
    unsigned char linkCount;   // offset 0x5, size 0x1
    unsigned short baseFlags;  // offset 0x6, size 0x2
    GuestPtr<xLinkAsset> link; // offset 0x8, size 0x4
    GuestPtr<void> eventFunc;
    // void (*eventFunc)(xBase*, xBase*, unsigned int, float*, xBase*, unsigned int); // offset 0xC, size 0x4
};

struct xBaseAsset
{
    unsigned int id;          // offset 0x0, size 0x4
    unsigned char baseType;   // offset 0x4, size 0x1
    unsigned char linkCount;  // offset 0x5, size 0x1
    unsigned short baseFlags; // offset 0x6, size 0x2
};

struct xEntAsset
{
    xBaseAsset base;
    unsigned char flags;      // offset 0x8, size 0x1
    unsigned char subtype;    // offset 0x9, size 0x1
    unsigned char pflags;     // offset 0xA, size 0x1
    unsigned char moreFlags;  // offset 0xB, size 0x1
    unsigned int surfaceID;   // offset 0xC, size 0x4
    xVec3 ang;                // offset 0x10, size 0xC
    xVec3 pos;                // offset 0x1C, size 0xC
    xVec3 scale;              // offset 0x28, size 0xC
    float redMult;            // offset 0x34, size 0x4
    float greenMult;          // offset 0x38, size 0x4
    float blueMult;           // offset 0x3C, size 0x4
    float seeThru;            // offset 0x40, size 0x4
    float seeThruSpeed;       // offset 0x44, size 0x4
    unsigned int modelInfoID; // offset 0x48, size 0x4
    unsigned int animListID;  // offset 0x4C, size 0x4
};

struct xModelInstance;
struct xModelPool
{
    // total size: 0xC
    GuestPtr<xModelPool> Next;     // offset 0x0, size 0x4
    unsigned int NumMatrices;      // offset 0x4, size 0x4
    GuestPtr<xModelInstance> List; // offset 0x8, size 0x4
};

struct xModelPipe
{
    // total size: 0x8
    unsigned int Flags;         // offset 0x0, size 0x4
    unsigned char Layer;        // offset 0x4, size 0x1
    unsigned char AlphaDiscard; // offset 0x5, size 0x1
    unsigned short PipePad;     // offset 0x6, size 0x2
};

struct xAnimPhysicsData
{
    GuestPtr<xVec3> tranTable; // offset 0x0, size 0x4
    GuestPtr<float> yawTable;  // offset 0x4, size 0x4
    signed int tranCount;      // offset 0x8, size 0x4
};

struct xAnimFile
{
    // total size: 0x24
    GuestPtr<xAnimFile> Next;               // offset 0x0, size 0x4
    GuestPtr<char> Name;                    // offset 0x4, size 0x4
    unsigned int ID;                        // offset 0x8, size 0x4
    unsigned int FileFlags;                 // offset 0xC, size 0x4
    float Duration;                         // offset 0x10, size 0x4
    float TimeOffset;                       // offset 0x14, size 0x4
    unsigned short BoneCount;               // offset 0x18, size 0x2
    unsigned char NumAnims[2];              // offset 0x1A, size 0x2
    GuestPtr<GuestPtr<void>> RawData;       // offset 0x1C, size 0x4
    GuestPtr<xAnimPhysicsData> PhysicsData; // offset 0x20, size 0x4
};

struct xAnimEffect
{
    GuestPtr<xAnimEffect> Next; // offset 0x0, size 0x4
    unsigned short Flags;       // offset 0x4, size 0x2
    unsigned short Probability; // offset 0x6, size 0x2
    float StartTime;            // offset 0x8, size 0x4
    float EndTime;              // offset 0xC, size 0x4
    // unsigned int (* Callback)(unsigned int, class xAnimActiveEffect *, class xAnimSingle *, void *); // offset
    // 0x10, size 0x4
    GuestPtr<void> Callback;
};

struct xAnimState;
struct xAnimTransition
{
    GuestPtr<xAnimTransition> Next; // offset 0x0, size 0x4
    GuestPtr<xAnimState> Dest;      // offset 0x4, size 0x4
    // unsigned int (*Conditional)(class xAnimTransition*, class xAnimSingle*, void*); // offset 0x8, size 0x4
    // unsigned int (*Callback)(class xAnimTransition*, class xAnimSingle*, void*);    // offset 0xC, size 0x4
    GuestPtr<void> Conditional;
    GuestPtr<void> Callback;
    unsigned int Flags;                   // offset 0x10, size 0x4
    unsigned int UserFlags;               // offset 0x14, size 0x4
    float SrcTime;                        // offset 0x18, size 0x4
    float DestTime;                       // offset 0x1C, size 0x4
    unsigned short Priority;              // offset 0x20, size 0x2
    unsigned short QueuePriority;         // offset 0x22, size 0x2
    float BlendRecip;                     // offset 0x24, size 0x4
    GuestPtr<unsigned short> BlendOffset; // offset 0x28, size 0x4
};

class xAnimTransitionList
{
    // total size: 0x8
public:
    GuestPtr<xAnimTransitionList> Next; // offset 0x0, size 0x4
    GuestPtr<xAnimTransition> T;        // offset 0x4, size 0x4
};

struct xAnimMultiFileBase
{
    unsigned int Count; // offset 0x0, size 0x4
};

struct xAnimMultiFileEntry
{
    unsigned int ID;          // offset 0x0, size 0x4
    GuestPtr<xAnimFile> File; // offset 0x4, size 0x4
};

struct xAnimMultiFile
{
    xAnimMultiFileBase base;
    xAnimMultiFileEntry Files[1]; // offset 0x4, size 0x8
};

struct xAnimState
{
    // total size: 0x4C
    GuestPtr<xAnimState> Next;             // offset 0x0, size 0x4
    GuestPtr<char> Name;                   // offset 0x4, size 0x4
    unsigned int ID;                       // offset 0x8, size 0x4
    unsigned int Flags;                    // offset 0xC, size 0x4
    unsigned int UserFlags;                // offset 0x10, size 0x4
    float Speed;                           // offset 0x14, size 0x4
    GuestPtr<xAnimFile> Data;              // offset 0x18, size 0x4
    GuestPtr<xAnimEffect> Effects;         // offset 0x1C, size 0x4
    GuestPtr<xAnimTransitionList> Default; // offset 0x20, size 0x4
    GuestPtr<xAnimTransitionList> List;    // offset 0x24, size 0x4
    GuestPtr<float> BoneBlend;             // offset 0x28, size 0x4
    GuestPtr<float> TimeSnap;              // offset 0x2C, size 0x4
    float FadeRecip;                       // offset 0x30, size 0x4
    GuestPtr<unsigned short> FadeOffset;   // offset 0x34, size 0x4
    GuestPtr<void> CallbackData;           // offset 0x38, size 0x4
    GuestPtr<xAnimMultiFile> MultiFile;    // offset 0x3C, size 0x4
    // void (*BeforeEnter)(struct xAnimPlay*, struct xAnimState*, void*); // offset 0x40, size 0x4
    // void (*StateCallback)(struct xAnimState*, struct xAnimSingle*, void*); // offset 0x44, size 0x4
    // void (*BeforeAnimMatrices)(struct xAnimPlay*, struct xQuat*, struct xVec3*, signed int); // offset 0x48, size
    // 0x4
    GuestPtr<void> BeforeEnter;
    GuestPtr<void> StateCallback;
    GuestPtr<void> BeforeAnimMatrices;
};

enum iSndHandle
{
};

struct xAnimActiveEffect
{
    GuestPtr<xAnimEffect> Effect;  // offset 0x0, size 0x4
    union {                        // inferred
        unsigned int Handle;       // offset 0x4, size 0x4
        enum iSndHandle SndHandle; // offset 0x4, size 0x4
    };
};

struct xAnimTable
{
    GuestPtr<char> Name;                      // offset 0x0, size 0x4
    GuestPtr<xAnimTransition> TransitionList; // offset 0x4, size 0x4
    GuestPtr<xAnimState> StateList;           // offset 0x8, size 0x4
    unsigned int AnimIndex;                   // offset 0xC, size 0x4
    unsigned int MorphIndex;                  // offset 0x10, size 0x4
    unsigned int UserFlags;                   // offset 0x14, size 0x4
};

struct xAnimPlay;
struct xAnimSingle
{
    // total size: 0x50
    unsigned int SingleFlags;               // offset 0x0, size 0x4
    GuestPtr<xAnimState> State;             // offset 0x4, size 0x4
    float Time;                             // offset 0x8, size 0x4
    float CurrentSpeed;                     // offset 0xC, size 0x4
    float BilinearLerp[2];                  // offset 0x10, size 0x8
    GuestPtr<xAnimEffect> Effect;           // offset 0x18, size 0x4
    unsigned int ActiveCount;               // offset 0x1C, size 0x4
    float LastTime;                         // offset 0x20, size 0x4
    GuestPtr<xAnimActiveEffect> ActiveList; // offset 0x24, size 0x4
    GuestPtr<xAnimPlay> Play;               // offset 0x28, size 0x4
    GuestPtr<xAnimTransition> Sync;         // offset 0x2C, size 0x4
    GuestPtr<xAnimTransition> Tran;         // offset 0x30, size 0x4
    GuestPtr<xAnimSingle> Blend;            // offset 0x34, size 0x4
    float BlendFactor;                      // offset 0x38, size 0x4
    xVec3 PhysDisp;                         // offset 0x3C, size 0xC
    float YawDisp;                          // offset 0x48, size 0x4
    unsigned int pad[1];                    // offset 0x4C, size 0x4
};

struct xAnimPlay
{
    // total size: 0x20
    GuestPtr<xAnimPlay> Next;           // offset 0x0, size 0x4
    unsigned short NumSingle;           // offset 0x4, size 0x2
    unsigned short BoneCount;           // offset 0x6, size 0x2
    GuestPtr<xAnimSingle> Single;       // offset 0x8, size 0x4
    GuestPtr<void> Object;              // offset 0xC, size 0x4
    GuestPtr<xAnimTable> Table;         // offset 0x10, size 0x4
    GuestPtr<xMemPool> Pool;            // offset 0x14, size 0x4
    GuestPtr<xModelInstance> ModelInst; // offset 0x18, size 0x4
    // void (*BeforeAnimMatrices)(struct xAnimPlay*, struct xQuat*, struct xVec3*, signed int); // offset 0x1C, size
    // 0x4
    GuestPtr<void> BeforeAnimMatrices;
};

struct xSurface;

struct xModelBucket
{
    GuestPtr<RpAtomic> Data;                      // offset 0x0, size 0x4
    GuestPtr<RpAtomic> OriginalData;              // offset 0x4, size 0x4
    union {                                       // inferred
        GuestPtr<xModelInstance> List;            // offset 0x8, size 0x4
        GuestPtr<GuestPtr<xModelBucket>> BackRef; // offset 0x8, size 0x4
    };
    signed int ClipFlags; // offset 0xC, size 0x4
    xModelPipe Pipe;      // offset 0x10, size 0x8
};

struct xLightKitLight
{
    unsigned int type;           // offset 0x0, size 0x4
    RwRGBAReal color;            // offset 0x4, size 0x10
    float matrix[16];            // offset 0x14, size 0x40
    float radius;                // offset 0x54, size 0x4
    float angle;                 // offset 0x58, size 0x4
    GuestPtr<RpLight> platLight; // offset 0x5C, size 0x4
};

struct xLightKit
{
    unsigned int tagID;                 // offset 0x0, size 0x4
    unsigned int groupID;               // offset 0x4, size 0x4
    unsigned int lightCount;            // offset 0x8, size 0x4
    GuestPtr<xLightKitLight> lightList; // offset 0xC, size 0x4
};

struct xBox
{
    struct xVec3 upper; // offset 0x0, size 0xC
    struct xVec3 lower; // offset 0xC, size 0xC
};

struct xModelInstance
{
    // total size: 0xA4
    GuestPtr<xModelInstance> Next;           // offset 0x0, size 0x4
    GuestPtr<xModelInstance> Parent;         // offset 0x4, size 0x4
    GuestPtr<xModelPool> Pool;               // offset 0x8, size 0x4
    GuestPtr<xAnimPlay> Anim;                // offset 0xC, size 0x4
    GuestPtr<RpAtomic> Data;                 // offset 0x10, size 0x4
    xModelPipe Pipe;                         // offset 0x14, size 0x8
    unsigned char InFrustum;                 // offset 0x1C, size 0x1
    unsigned char TrueClip;                  // offset 0x1D, size 0x1
    signed char sortBias;                    // offset 0x1E, size 0x1
    unsigned char modelpad;                  // offset 0x1F, size 0x1
    float RedMultiplier;                     // offset 0x20, size 0x4
    float GreenMultiplier;                   // offset 0x24, size 0x4
    float BlueMultiplier;                    // offset 0x28, size 0x4
    float Alpha;                             // offset 0x2C, size 0x4
    float FadeStart;                         // offset 0x30, size 0x4
    float FadeEnd;                           // offset 0x34, size 0x4
    GuestPtr<xSurface> Surf;                 // offset 0x38, size 0x4
    GuestPtr<GuestPtr<xModelBucket>> Bucket; // offset 0x3C, size 0x4
    GuestPtr<xModelInstance> BucketNext;     // offset 0x40, size 0x4
    GuestPtr<xLightKit> LightKit;            // offset 0x44, size 0x4
    GuestPtr<void> Object;                   // offset 0x48, size 0x4
    unsigned short Flags;                    // offset 0x4C, size 0x2
    unsigned char BoneCount;                 // offset 0x4E, size 0x1
    unsigned char BoneIndex;                 // offset 0x4F, size 0x1
    GuestPtr<unsigned char> BoneRemap;       // offset 0x50, size 0x4
    GuestPtr<RwMatrixTag> Mat;               // offset 0x54, size 0x4
    xVec3 Scale;                             // offset 0x58, size 0xC
    xBox animBound;                          // offset 0x64, size 0x18
    xBox combinedAnimBound;                  // offset 0x7C, size 0x18
    unsigned int modelID;                    // offset 0x94, size 0x4
    unsigned int shadowID;                   // offset 0x98, size 0x4
    GuestPtr<RpAtomic> shadowmapAtomic;      // offset 0x9C, size 0x4
    struct                                   /* @struct */
    {
        // total size: 0x4
        GuestPtr<xVec3> verts; // offset 0x0, size 0x4
    } anim_coll;               // offset 0xA0, size 0x4
};

struct xRot
{
    xVec3 axis;  // offset 0x0, size 0xC
    float angle; // offset 0xC, size 0x4
};

struct xMat3x3
{
    xVec3 right;       // offset 0x0, size 0xC
    signed int flags;  // offset 0xC, size 0x4
    xVec3 up;          // offset 0x10, size 0xC
    unsigned int pad1; // offset 0x1C, size 0x4
    xVec3 at;          // offset 0x20, size 0xC
    unsigned int pad2; // offset 0x2C, size 0x4
};

struct xMat4x3
{
    xMat3x3 base;
    xVec3 pos;         // offset 0x30, size 0xC
    unsigned int pad3; // offset 0x3C, size 0x4
};

struct xEntFrame
{
    xMat4x3 mat;       // offset 0x0, size 0x40
    xMat4x3 oldmat;    // offset 0x40, size 0x40
    xVec3 oldvel;      // offset 0x80, size 0xC
    xRot oldrot;       // offset 0x8C, size 0x10
    xRot drot;         // offset 0x9C, size 0x10
    xRot rot;          // offset 0xAC, size 0x10
    xVec3 dvel;        // offset 0xBC, size 0xC
    xVec3 vel;         // offset 0xC8, size 0xC
    unsigned int mode; // offset 0xD4, size 0x4
    xVec3 dpos;        // offset 0xD8, size 0xC
};

struct xCollis
{
    unsigned int flags;            // offset 0x0, size 0x4
    unsigned int oid;              // offset 0x4, size 0x4
    GuestPtr<void> optr;           // offset 0x8, size 0x4
    GuestPtr<xModelInstance> mptr; // offset 0xC, size 0x4
    float dist;                    // offset 0x10, size 0x4
    float test_dist;               // offset 0x14, size 0x4
    xVec3 norm;                    // offset 0x18, size 0xC
    xVec3 tohit;                   // offset 0x24, size 0xC
    xVec3 depen;                   // offset 0x30, size 0xC
    xVec3 hdng;                    // offset 0x3C, size 0xC
                                   // tri_data union below
    float t;                       // offset 0x0, size 0x4
    float u;                       // offset 0x4, size 0x4
    float v;                       // offset 0x8, size 0x4
};

struct xEntCollis
{
    unsigned char chk;       // offset 0x0, size 0x1
    unsigned char pen;       // offset 0x1, size 0x1
    unsigned char env_sidx;  // offset 0x2, size 0x1
    unsigned char env_eidx;  // offset 0x3, size 0x1
    unsigned char npc_sidx;  // offset 0x4, size 0x1
    unsigned char npc_eidx;  // offset 0x5, size 0x1
    unsigned char dyn_sidx;  // offset 0x6, size 0x1
    unsigned char dyn_eidx;  // offset 0x7, size 0x1
    unsigned char stat_sidx; // offset 0x8, size 0x1
    unsigned char stat_eidx; // offset 0x9, size 0x1
    unsigned char idx;       // offset 0xA, size 0x1
    xCollis colls[18];       // offset 0xC, size 0x5E8
    // void (*post)(xEnt *, xScene *, float, xEntCollis *); // offset 0x5F4, size 0x4
    // unsigned int (*depenq)(xEnt *, xEnt *, xScene *, float, xCollis *); // offset 0x5F8, size 0x4
    GuestPtr<void> post;
    GuestPtr<void> depenq;
};

struct xGridBound;
struct xGrid
{
    unsigned char ingrid_id;              // offset 0x0, size 0x1
    unsigned char pad[3];                 // offset 0x1, size 0x3
    unsigned short nx;                    // offset 0x4, size 0x2
    unsigned short nz;                    // offset 0x6, size 0x2
    float minx;                           // offset 0x8, size 0x4
    float minz;                           // offset 0xC, size 0x4
    float maxx;                           // offset 0x10, size 0x4
    float maxz;                           // offset 0x14, size 0x4
    float csizex;                         // offset 0x18, size 0x4
    float csizez;                         // offset 0x1C, size 0x4
    float inv_csizex;                     // offset 0x20, size 0x4
    float inv_csizez;                     // offset 0x24, size 0x4
    float maxr;                           // offset 0x28, size 0x4
    GuestPtr<GuestPtr<xGridBound>> cells; // offset 0x2C, size 0x4
    GuestPtr<xGridBound> other;           // offset 0x30, size 0x4
    signed int iter_active;               // offset 0x34, size 0x4
};

struct xGridBound
{
    GuestPtr<void> data;                 // offset 0x0, size 0x4
    unsigned short gx;                   // offset 0x4, size 0x2
    unsigned short gz;                   // offset 0x6, size 0x2
    unsigned char oversize;              // offset 0x8, size 0x1
    unsigned char deleted;               // offset 0x9, size 0x1
    unsigned char gpad;                  // offset 0xA, size 0x1
    unsigned char pad;                   // offset 0xB, size 0x1
    GuestPtr<xGrid> grid;                // offset 0xC, size 0x4
    GuestPtr<GuestPtr<xGridBound>> head; // offset 0x10, size 0x4
    GuestPtr<xGridBound> next;           // offset 0x14, size 0x4
};

struct xSphere
{
    xVec3 center; // offset 0x0, size 0xC
    float r;      // offset 0xC, size 0x4
};

struct xBBox
{
    xVec3 center; // offset 0x0, size 0xC
    xBox box;     // offset 0xC, size 0x18
};

struct xCylinder
{
    xVec3 center; // offset 0x0, size 0xC
    float r;      // offset 0xC, size 0x4
    float h;      // offset 0x10, size 0x4
};

struct xQCData
{
    signed char xmin;     // offset 0x0, size 0x1
    signed char ymin;     // offset 0x1, size 0x1
    signed char zmin;     // offset 0x2, size 0x1
    signed char zmin_dup; // offset 0x3, size 0x1
    signed char xmax;     // offset 0x4, size 0x1
    signed char ymax;     // offset 0x5, size 0x1
    signed char zmax;     // offset 0x6, size 0x1
    signed char zmax_dup; // offset 0x7, size 0x1
    xVec3 min;            // offset 0x8, size 0xC
    xVec3 max;            // offset 0x14, size 0xC
};

struct xBound
{
    xQCData qcd;          // offset 0x0, size 0x20
    unsigned char type;   // offset 0x20, size 0x1
    unsigned char pad[3]; // offset 0x21, size 0x3
    union {               // inferred
        xSphere sph;      // offset 0x24, size 0x10
        xBBox box;        // offset 0x24, size 0x24
        xCylinder cyl;    // offset 0x24, size 0x14
    };
    GuestPtr<xMat4x3> mat; // offset 0x48, size 0x4
};

struct xFFX
{
    unsigned int flags; // offset 0x0, size 0x4
    // void (*doEffect)(xEnt *, xScene *, float, void *); // offset 0x4, size 0x4
    GuestPtr<void> doEffect;
    GuestPtr<void> fdata; // offset 0x8, size 0x4
    GuestPtr<xFFX> next;  // offset 0xC, size 0x4
};

struct anim_coll_data
{
    unsigned int flags;      // offset 0x0, size 0x4
    unsigned int bones;      // offset 0x4, size 0x4
    xMat4x3 old_mat;         // offset 0x10, size 0x40
    xMat4x3 new_mat;         // offset 0x50, size 0x40
    unsigned int verts_size; // offset 0x90, size 0x4
    GuestPtr<xVec3> verts;   // offset 0x94, size 0x4
    GuestPtr<xVec3> normals; // offset 0x98, size 0x4
};

struct xShadowSimpleCache;
struct xEntShadow;
struct xEnt
{
    // total size: 0xD8
    xBase base;
    GuestPtr<xEntAsset> asset;                // offset 0x10, size 0x4
    unsigned short idx;                       // offset 0x14, size 0x2
    unsigned char flags;                      // offset 0x16, size 0x1
    unsigned char miscflags;                  // offset 0x17, size 0x1
    unsigned char subType;                    // offset 0x18, size 0x1
    unsigned char pflags;                     // offset 0x19, size 0x1
    unsigned short moreFlags;                 // offset 0x1A, size 0x2
    unsigned char _isCulled : 2;              // offset 0x1C, size 0x1
    unsigned char collisionEventReceived : 2; // offset 0x1C, size 0x1
    unsigned char driving_count;              // offset 0x1D, size 0x1
    unsigned char num_ffx;                    // offset 0x1E, size 0x1
    unsigned char collType;                   // offset 0x1F, size 0x1
    unsigned char collLev;                    // offset 0x20, size 0x1
    unsigned char chkby;                      // offset 0x21, size 0x1
    unsigned char penby;                      // offset 0x22, size 0x1
    // void (*visUpdate)(xEnt*);                        // offset 0x24, size 0x4
    GuestPtr<void> visUpdate;
    GuestPtr<xModelInstance> model;        // offset 0x28, size 0x4
    GuestPtr<xModelInstance> collModel;    // offset 0x2C, size 0x4
    GuestPtr<xModelInstance> camcollModel; // offset 0x30, size 0x4
    // void (*update)(xEnt*, xScene*, float);           // offset 0x34, size 0x4
    // void (*endUpdate)(xEnt*, xScene*, float);        // offset 0x38, size 0x4
    // void (*bupdate)(xEnt*, xVec3*);                  // offset 0x3C, size 0x4
    // void (*move)(xEnt*, xScene*, float, xEntFrame*); // offset 0x40, size 0x4
    // void (*render)(xEnt*);                           // offset 0x44, size 0x4
    GuestPtr<void> update;
    GuestPtr<void> endUpdate;
    GuestPtr<void> bupdate;
    GuestPtr<void> move;
    GuestPtr<void> render;
    GuestPtr<xEntFrame> frame;   // offset 0x48, size 0x4
    GuestPtr<xEntCollis> collis; // offset 0x4C, size 0x4
    xGridBound gridb;            // offset 0x50, size 0x18
    xBound bound;                // offset 0x68, size 0x4C
    // void (*transl)(xEnt*, xVec3*, xMat4x3*); // offset 0xB4, size 0x4
    GuestPtr<void> transl;
    GuestPtr<xFFX> ffx;                      // offset 0xB8, size 0x4
    GuestPtr<xEnt> driver;                   // offset 0xBC, size 0x4
    GuestPtr<xEnt> driven;                   // offset 0xC0, size 0x4
    signed int driveMode;                    // offset 0xC4, size 0x4
    GuestPtr<xShadowSimpleCache> simpShadow; // offset 0xC8, size 0x4
    GuestPtr<xEntShadow> entShadow;          // offset 0xCC, size 0x4
    GuestPtr<anim_coll_data> anim_coll;      // offset 0xD0, size 0x4
    GuestPtr<void> user_data;                // offset 0xD4, size 0x4
};

struct xEnv;
struct xScene
{
    unsigned int sceneID;              // offset 0x0, size 0x4
    unsigned short flags;              // offset 0x4, size 0x2
    unsigned short num_trigs;          // offset 0x6, size 0x2
    unsigned short num_stats;          // offset 0x8, size 0x2
    unsigned short num_dyns;           // offset 0xA, size 0x2
    unsigned short num_npcs;           // offset 0xC, size 0x2
    unsigned short num_act_ents;       // offset 0xE, size 0x2
    float gravity;                     // offset 0x10, size 0x4
    float drag;                        // offset 0x14, size 0x4
    float friction;                    // offset 0x18, size 0x4
    unsigned short num_ents_allocd;    // offset 0x1C, size 0x2
    unsigned short num_trigs_allocd;   // offset 0x1E, size 0x2
    unsigned short num_stats_allocd;   // offset 0x20, size 0x2
    unsigned short num_dyns_allocd;    // offset 0x22, size 0x2
    unsigned short num_npcs_allocd;    // offset 0x24, size 0x2
    GuestPtr<GuestPtr<xEnt>> trigs;    // offset 0x28, size 0x4
    GuestPtr<GuestPtr<xEnt>> stats;    // offset 0x2C, size 0x4
    GuestPtr<GuestPtr<xEnt>> dyns;     // offset 0x30, size 0x4
    GuestPtr<GuestPtr<xEnt>> npcs;     // offset 0x34, size 0x4
    GuestPtr<GuestPtr<xEnt>> act_ents; // offset 0x38, size 0x4
    GuestPtr<xEnv> env;                // offset 0x3C, size 0x4
    xMemPool mempool;                  // offset 0x40, size 0x1C
    // xBase* (*resolvID)(unsigned int); // offset 0x5C, size 0x4
    // char* (*base2Name)(xBase*);       // offset 0x60, size 0x4
    // char* (*id2Name)(unsigned int);   // offset 0x64, size 0x4
    GuestPtr<void> resolvID;
    GuestPtr<void> base2Name;
    GuestPtr<void> id2Name;
};

struct xPortalAsset
{
    xBaseAsset base;
    unsigned int assetCameraID; // offset 0x8, size 0x4
    unsigned int assetMarkerID; // offset 0xC, size 0x4
    float ang;                  // offset 0x10, size 0x4
    unsigned int sceneID;       // offset 0x14, size 0x4
};

struct xEnvAsset
{
    xBaseAsset base;
    unsigned int bspAssetID;           // offset 0x8, size 0x4
    unsigned int startCameraAssetID;   // offset 0xC, size 0x4
    unsigned int climateFlags;         // offset 0x10, size 0x4
    float climateStrengthMin;          // offset 0x14, size 0x4
    float climateStrengthMax;          // offset 0x18, size 0x4
    unsigned int bspLightKit;          // offset 0x1C, size 0x4
    unsigned int objectLightKit;       // offset 0x20, size 0x4
    unsigned int flags;                // offset 0x24, size 0x4
    unsigned int bspCollisionAssetID;  // offset 0x28, size 0x4
    unsigned int bspFXAssetID;         // offset 0x2C, size 0x4
    unsigned int bspCameraAssetID;     // offset 0x30, size 0x4
    unsigned int bspMapperID;          // offset 0x34, size 0x4
    unsigned int bspMapperCollisionID; // offset 0x38, size 0x4
    unsigned int bspMapperFXID;        // offset 0x3C, size 0x4
    float loldHeight;                  // offset 0x40, size 0x4
    xVec3 minBounds;                   // offset 0x44, size 0xC
    xVec3 maxBounds;                   // offset 0x50, size 0xC
};

struct _zPortal
{
    xBase base;
    GuestPtr<xPortalAsset> passet; // offset 0x10, size 0x4
};

struct _zEnv
{
    xBase base;
    GuestPtr<xEnvAsset> easset; // offset 0x10, size 0x4
};

struct zSceneParameters;
// This is probably close enough.
struct zScene
{
    // Well isn't that funny.
    xScene _base;
    GuestPtr<_zPortal> pendingPortal;      // offset 0x68, size 0x4
    signed int num_base;                   // offset 0x6C, size 0x4
    GuestPtr<GuestPtr<xBase>> base;        // offset 0x70, size 0x4
    unsigned int num_update_base;          // offset 0x74, size 0x4
    GuestPtr<GuestPtr<xBase>> update_base; // offset 0x78, size 0x4
    signed int baseCount[140];             // offset 0x7C, size 0x230
    xBase* baseList[140];                  // offset 0x2AC, size 0x230
    GuestPtr<_zEnv> zen;                   // offset 0x4DC, size 0x4
    GuestPtr<zSceneParameters> parameters; // offset 0x4E0, size 0x4
    unsigned char enableDrawing;           // offset 0x4E4, size 0x1
};

// We're missing xGlobals and zGlobals because it is difficult to
// create a proper structure for them. It is also huge, and rarely
// would it be worthwhile to read the entire object from the guest.
// Instead, just keep track of the offsets into the globals we need.
struct xGlobalsOffsets
{
    static constexpr unsigned int ___player_ent_dont_use_directly = 0x7F8;
    static constexpr unsigned int sceneCur = 0x7FC;
};

struct xGlobals
{
    static constexpr unsigned int globals = 0x2AF630;

    static constexpr GuestPtr<GuestPtr<xEnt>> ___player_ent_dont_use_directly =
        globals + xGlobalsOffsets::___player_ent_dont_use_directly;

    static constexpr GuestPtr<GuestPtr<zScene>> sceneCur = globals + xGlobalsOffsets::sceneCur;
};
}
