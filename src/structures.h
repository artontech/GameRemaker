#pragma once

#include "stdafx.h"

#include "format.h"

// Header name enum
enum HeaderName: uint32_t {
	HEADER_FORM = 0x4D524F46,			// FORM
	HEADER_GENERAL = 0x384E4547,		// GEN8
	HEADER_OPTIONS = 0x4E54504F,		// OPTN
	HEADER_LANGUAGE = 0x474E414C,		// LANG
	HEADER_EXTENSIONS = 0x4E545845,		// EXTN
	HEADER_SOUNDS = 0x444E4F53,			// SOND
	HEADER_AUDIO_GROUPS = 0x50524741,	// AGRP
	HEADER_SPRITES = 0x54525053,		// SPRT
	HEADER_BACKGROUNDS = 0x444E4742,	// BGND
	HEADER_PATHS = 0x48544150,			// PATH
	HEADER_SCRIPTS = 0x54504353,		// SCPT
	HEADER_GLOBALS = 0x424F4C47,		// GLOB
	HEADER_SHADERS = 0x52444853,		// SHDR
	HEADER_FONTS = 0x544E4F46,			// FONT
	HEADER_TIMELINES = 0x4E4C4D54,		// TMLN
	HEADER_OBJECTS = 0x544A424F,		// OBJT
	HEADER_ROOMS = 0x4D4F4F52,			// ROOM
	HEADER_DATA_FILES = 0x4C464144,		// DAFL
	HEADER_TEXTURE_PAGE = 0x47415054,	// TPAG
	HEADER_CODE = 0x45444F43,			// CODE
	HEADER_VARIABLES = 0x49524156,		// VARI
	HEADER_FUNCTIONS = 0x434E5546,		// FUNC
	HEADER_STRINGS = 0x47525453,		// STRG
	HEADER_TEXTURES = 0x52545854,		// TXTR
	HEADER_AUDIO = 0x4F445541,			// AUDO
	HEADER_EMBED_IMAGE = 0x49424D45,	// EMBI
};


// Section header
struct Header {
	HeaderName name;
	uint32_t size;

	string getName() const;
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma pack(push)
#pragma pack(1)

struct Point {
	int32_t x;
	int32_t y;
};

enum InfoFlag : uint32_t {
	FLAG_FULL_SCREEN = 0x0001,
	FLAG_SYNC_VERTEX1 = 0x0002,
	FLAG_SYNC_VERTEX2 = 0x0004,
	FLAG_INTERPOLATE = 0x0008,
	FLAG_UNKNOWN = 0x0010,
	FLAG_SHOW_CURSOR = 0x0020,
	FLAG_SIZEABLE = 0x0040,
	FLAG_SCREEN_KEY = 0x0080,
	FLAG_SYNC_VERTEX3 = 0x0100,
	FLAG_STUDIO_VERSION_B1 = 0x0200,
	FLAG_STUDIO_VERSION_B2 = 0x0400,
	FLAG_STUDIO_VERSION_B3 = 0x0800,
	FLAG_STUDIO_VERSION_MASK = FLAG_STUDIO_VERSION_B1 | FLAG_STUDIO_VERSION_B2 | FLAG_STUDIO_VERSION_B3,
	FLAG_STEAM_ENABLED = 0x1000,
	FLAG_LOCAL_DATA_ENABLED = 0x2000,
	FLAG_BORDERLESS_WINDOW = 0x4000
};

// For General
struct GEN8 {
	uint32_t debug : 8;
	uint32_t byteCodeVersion : 24;
	uint32_t filenameOffset;
	uint32_t configOffset;
	uint32_t lastObj;
	uint32_t lastTile;
	uint32_t gameID;
	uint32_t _pad1[4]; // 0, 0, 0, 0
	uint32_t nameOffset;
	int32_t major;
	int32_t minor;
	int32_t release;
	int32_t build;
	Point windowSize;
	InfoFlag info;
	uint8_t md5[0x10];
	uint32_t crc32;
	uint64_t timestamp; // UNIX time (64-bit, luckily)
	uint32_t displayNameOffset;
	uint32_t activeTargets;
	uint32_t _unknown[4]; // unknown, more flags?
	uint32_t appID;
	uint32_t numberCount;
};

// For Texture
struct TextureEntry {
	uint32_t _pad;
	uint32_t padOrOffset;
	uint32_t offset;
};

enum PNGChunkType : uint32_t {
	PNG_CHUNK_IHDR = 0x52444849, // IHDR
	PNG_CHUNK_SRGB = 0x42475273, // sRGB 
	PNG_CHUNK_IEND = 0x444E4549, // IEND
	PNG_CHUNK_GAMA = 0x414D4167, // gAMA
	PNG_CHUNK_PHYS = 0x73594870, // pHYs
	PNG_CHUNK_IDAT = 0x54414449, // IDAT
};

struct PNGChunk {
	uint32_t length;
	PNGChunkType type;
};

struct PNGHeader {
	uint64_t _pad; // <0x89>PNG <uint length?>
	PNGChunk chunk;
	uint32_t width, height;
};

// For Sound
enum SoundFlag : uint32_t {
	SOUND_EMBEDDED = 0x01, // NotStreamed?
	SOUND_COMPRESSED = 0x02,
	SOUND_NORMAL = 0x04 | 0x20 | 0x40 // all seem to have these flags -> unimportant?
};
void operator|=(SoundFlag &self, const SoundFlag &f);

struct SoundEntry {
	uint32_t nameOffset;
	SoundFlag flags;
	uint32_t typeOffset;
	uint32_t filenameOffset;
	uint32_t _pad; // effects?
	float_t volume;
	float_t pitch;
	int32_t groupID;
	int32_t audioID;
};

#pragma pack(pop)
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// PNG file
struct PNGFile {
	PNGHeader header;
	uint32_t length;
	uint8_t* data;
};

// Sound info
struct SoundInfo {
	SoundEntry entry;

	string name;
	int nameID;

	string type;
	int typeID;

	string filename;
	int filenameID;

	string group;

	bool embedded;
	bool compressed;
};

// Audio group info
struct AudioGroupInfo {
	uint32_t nameOffset;
	string name;
	int nameID;
};

// Audio file
struct AudioFile {
	uint32_t length;
	uint8_t* data;

	string group;
	string filename;
};
