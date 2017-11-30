/** @file src/sprites.c Sprite routines. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multichar.h"
#include "types.h"
#include "os/common.h"
#include "os/endian.h"
#include "os/sleep.h"
#include "os/strings.h"

#include "sprites.h"

#include "codec/format80.h"
#include "file.h"
#include "gfx.h"
#include "house.h"
#include "ini.h"
#include "input/mouse.h"
#include "gui/gui.h"
#include "script/script.h"
#include "string.h"
#include "tile.h"


uint8 **g_sprites = NULL;
static uint16 s_spritesCount = 0;
uint8 *g_spriteBuffer;
uint8 *g_iconRTBL = NULL;
uint8 *g_iconRPAL = NULL;
uint8 *g_spritePixels = NULL;
uint16 *g_iconMap = NULL;

uint8 *g_fileRgnclkCPS = NULL;
void *g_fileRegionINI = NULL;
uint16 *g_regions = NULL;

uint16 g_veiledSpriteID;
uint16 g_bloomSpriteID;
uint16 g_landscapeSpriteID;
uint16 g_builtSlabSpriteID;
uint16 g_wallSpriteID;

void *g_mouseSprite = NULL;
void *g_mouseSpriteBuffer = NULL;

static uint16 s_mouseSpriteSize = 0;
static uint16 s_mouseSpriteBufferSize = 0;

static bool s_iconLoaded = false;

/**
 * Gets the given sprite inside the given buffer.
 *
 * @param buffer The buffer containing sprites.
 * @param index The index of the sprite to get.
 * @return The sprite.
 */
static const uint8 *Sprites_GetSprite(const uint8 *buffer, uint16 index)
{
	uint32 offset;

	if (buffer == NULL) return NULL;
	if (READ_LE_UINT16(buffer) <= index) return NULL;

	buffer += 2;

	offset = READ_LE_UINT32(buffer + 4 * index);

	if (offset == 0) return NULL;

	return buffer + offset;
}

/**
 * Loads the sprites.
 *
 * @param index The index of the list of sprite files to load.
 * @param sprites The array where to store CSIP for each loaded sprite.
 */

//uh wut....
static void Sprites_Load(const char *filename)
{
	uint8 *buffer;
	uint16 count;
	uint16 i;

	buffer = Read_FileWholeFile(filename);

	count = READ_LE_UINT16(buffer);

	s_spritesCount += count;
	g_sprites = (uint8 **)realloc(g_sprites, s_spritesCount * sizeof(uint8 *));

	for (i = 0; i < count; i++) {
		const uint8 *src = Sprites_GetSprite(buffer, i);
		uint8 *dst = NULL;

		if (src != NULL) {
			uint16 size = READ_LE_UINT16(src + 6);
			dst = (uint8 *)malloc(size);
			memcpy(dst, src, size);
		}

		g_sprites[s_spritesCount - count + i] = dst;
	}

	free(buffer);
}

/**
 * Gets the width of the given sprite.
 *
 * @param sprite The sprite.
 * @return The width.
 */
uint8 Sprite_GetWidth(uint8 *sprite)
{
	if (sprite == NULL) return 0;

	return sprite[3];
}

/**
 * Gets the height of the given sprite.
 *
 * @param sprite The sprite.
 * @return The height.
 */
uint8 Sprite_GetHeight(uint8 *sprite)
{
	if (sprite == NULL) return 0;

	return sprite[2];
}

/**
 * Gets the type of the given sprite.
 *
 * @param sprite The sprite.
 * @return The type.
 */
uint16 Sprites_GetType(uint8 *sprite)
{
	if (sprite == NULL) return 0;

	return READ_LE_UINT16(sprite);
}

/**
 * Decodes an image.
 *
 * @param source The encoded image.
 * @param dest The place the decoded image will be.
 * @return The size of the decoded image.
 */

//This decided what compression to use, nothing to do directly with Shapes
//Other compression options in this function were LZW12 and LZW14, RLE and CMV
static uint32 Uncompress_Data(uint8 *source, uint8 *dest)
{
	uint32 size = 0;

	switch(*source) {
		case 0x0:
			source += 2;
			size = READ_LE_UINT32(source);
			source += 4;
			source += READ_LE_UINT16(source);
			source += 2;
			memmove(dest, source, size);
			break;

		case 0x4:
			source += 6;
			source += READ_LE_UINT16(source);
			source += 2;
			size = LCW_Uncomp(dest, source, 0xFFFF);
			break;

		default: break;
	}

	return size;
}

/**
 * Loads an ICN file.
 * NOTE : should be called "tiles"
 *
 * @param filename The name of the file to load.
 * @param screenID The index of a memory block where to store loaded sprites.
 */
static void Load_Icon_Set(const char *filename)
{
	uint8  fileIndex;

	uint32 spriteDataLength;
	uint32 tableLength;
	uint32 paletteLength;
	int8   info[4];

	fileIndex = ChunkFile_Open(filename);

	/* Get the length of the chunks */
	spriteDataLength = ChunkFile_Seek(fileIndex, HTOBE32(CC_SSET));
	tableLength      = ChunkFile_Seek(fileIndex, HTOBE32(CC_RTBL));
	paletteLength    = ChunkFile_Seek(fileIndex, HTOBE32(CC_RPAL));

	/* Read the header information */
	Read_Iff_Chunk(fileIndex, HTOBE32(CC_SINF), info, 4);
	GFX_Init_SpriteInfo(info[0], info[1]);

	/* Get the SpritePixels chunk */
	free(g_spritePixels);
	g_spritePixels = calloc(1, spriteDataLength);
	Read_Iff_Chunk(fileIndex, HTOBE32(CC_SSET), g_spritePixels, spriteDataLength);
	spriteDataLength = Uncompress_Data(g_spritePixels, g_spritePixels);
	/*g_spritePixels = realloc(g_spritePixels, spriteDataLength);*/

	/* Get the Table chunk */
	free(g_iconRTBL);
	g_iconRTBL = calloc(1, tableLength);
	Read_Iff_Chunk(fileIndex, HTOBE32(CC_RTBL), g_iconRTBL, tableLength);

	/* Get the Palette chunk */
	free(g_iconRPAL);
	g_iconRPAL = calloc(1, paletteLength);
	Read_Iff_Chunk(fileIndex, HTOBE32(CC_RPAL), g_iconRPAL, paletteLength);

	Close_Iff_File(fileIndex);
}

/**
 * Loads the sprites for tiles.
 */
void Sprites_LoadTiles(void)
{
	if (s_iconLoaded) return;

	s_iconLoaded = true;

	Load_Icon_Set("ICON.ICN");

	free(g_iconMap);
	g_iconMap = Read_FileWholeFileLE16("ICON.MAP");

	g_veiledSpriteID    = g_iconMap[g_iconMap[ICM_ICONGROUP_FOG_OF_WAR] + 16];
	g_bloomSpriteID     = g_iconMap[g_iconMap[ICM_ICONGROUP_SPICE_BLOOM]];
	g_builtSlabSpriteID = g_iconMap[g_iconMap[ICM_ICONGROUP_CONCRETE_SLAB] + 2];
	g_landscapeSpriteID = g_iconMap[g_iconMap[ICM_ICONGROUP_LANDSCAPE]];
	g_wallSpriteID      = g_iconMap[g_iconMap[ICM_ICONGROUP_WALLS]];

	Script_LoadFromFile("UNIT.EMC", g_scriptUnit, g_scriptFunctionsUnit, Get_Page(SCREEN_2));
}

/**
 * Unloads the sprites for tiles.
 */
void Free_Icon_Set(void)
{
	s_iconLoaded = false;
}

/**
 * Loads a CPS file.
 *
 * @param filename The name of the file to load.
 * @param screenID The index of a memory block where to store loaded data.
 * @param palette Where to store the palette, if any.
 * @return The size of the loaded image.
 */
static uint32 Load_Uncompress(const char *filename, Screen screenID, uint8 *palette)
{
	uint8 index;
	uint16 size;
	uint8 *buffer;
	uint8 *buffer2;
	uint16 paletteSize;

	buffer = Get_Page(screenID);

	index = File_Open(filename, FILE_MODE_READ);

	size = Read_File_LE16(index);

	Read_File(index, buffer, 8);

	size -= 8;

	paletteSize = READ_LE_UINT16(buffer + 6);

	if (palette != NULL && paletteSize != 0) {
		Read_File(index, palette, paletteSize);
	} else {
		Seek_File(index, paletteSize, 1);
	}

	buffer[6] = 0;	/* dont read palette next time */
	buffer[7] = 0;
	size -= paletteSize;

	buffer2 = Get_Page(screenID);
	buffer2 += Get_Buff(screenID) - size - 8;

	memmove(buffer2, buffer, 8);
	Read_File(index, buffer2 + 8, size);

	Close_File(index);

	return Uncompress_Data(buffer2, buffer);
}

/**
 * Loads an image.
 *
 * @param filename The name of the file to load.
 * @param memory1 The index of a memory block where to store loaded data.
 * @param memory2 The index of a memory block where to store loaded data.
 * @param palette Where to store the palette, if any.
 * @return The size of the loaded image.
 */
uint16 Load_Picture(const char *filename, Screen screenID, uint8 *palette)
{
#if 0
	uint8 index;
	uint32 header;

	index = File_Open(filename, FILE_MODE_READ);
	if (index == FILE_INVALID) return 0;

	Read_File(index, &header, 4);
	Close_File(index);
#endif
	return Load_Uncompress(filename, screenID, palette) / 8000;
}

void Set_Mouse_Cursor(uint16 hotSpotX, uint16 hotSpotY, uint8 *sprite)
{
	uint16 size;

	if (sprite == NULL || MDisabled != 0) return;

	while (MouseUpdate != 0) sleepIdle();

	MouseUpdate++;

	Low_Hide_Mouse();

	size = GFX_GetSize(READ_LE_UINT16(sprite + 3) + 16, sprite[5]);

	if (s_mouseSpriteBufferSize < size) {
		g_mouseSpriteBuffer = realloc(g_mouseSpriteBuffer, size);
		s_mouseSpriteBufferSize = size;
	}

	size = READ_LE_UINT16(sprite + 8) + 10;
	if ((*sprite & 0x1) != 0) size += 16;

	if (s_mouseSpriteSize < size) {
		g_mouseSprite = realloc(g_mouseSprite, size);
		s_mouseSpriteSize = size;
	}

	if ((*sprite & 0x2) != 0) {
		memcpy(g_mouseSprite, sprite, READ_LE_UINT16(sprite + 6) * 2);
	} else {
		uint8 *dst = (uint8 *)g_mouseSprite;
		uint8 *buf = g_spriteBuffer;
		uint16 flags = READ_LE_UINT16(sprite) | 0x2;

		dst[0] = sprite[0] | 0x2;
		dst[1] = sprite[1];
		dst += 2;
		sprite += 2;

		memcpy(dst, sprite, 6);
		dst += 6;
		sprite += 6;

		size = READ_LE_UINT16(sprite);
		dst[0] = sprite[0];
		dst[1] = sprite[1];
		dst += 2;
		sprite += 2;

		if ((flags & 0x1) != 0) {
			memcpy(dst, sprite, 16);
			dst += 16;
			sprite += 16;
		}

		LCW_Uncomp(buf, sprite, size);

		memcpy(dst, buf, size);
	}

	g_mouseSpriteHotspotX = hotSpotX;
	g_mouseSpriteHotspotY = hotSpotY;

	sprite = g_mouseSprite;
	g_mouseHeight = sprite[5];
	g_mouseWidth = (READ_LE_UINT16(sprite + 3) >> 3) + 2;

	Low_Show_Mouse();

	MouseUpdate--;
}

static void InitRegions(void)
{
	uint16 *regions = g_regions;
	uint16 i;
	char textBuffer[81];

	Ini_GetString("INFO", "TOTAL REGIONS", NULL, textBuffer, lengthof(textBuffer) - 1, g_fileRegionINI);

	sscanf(textBuffer, "%hu", &regions[0]);

	for (i = 0; i < regions[0]; i++) regions[i + 1] = 0xFFFF;
}

void Sprites_CPS_LoadRegionClick(void)
{
	uint8 *buf;
	uint8 i;
	char filename[16];

	buf = Get_Page(SCREEN_2);

	g_fileRgnclkCPS = buf;
	Load_Uncompress("RGNCLK.CPS", SCREEN_2, NULL);
	for (i = 0; i < 120; i++) memcpy(buf + (i * 304), buf + 7688 + (i * 320), 304);
	buf += 120 * 304;

	g_fileRegionINI = buf;
	snprintf(filename, sizeof(filename), "REGION%c.INI", g_table_HouseType[g_playerHouseID].name[0]);
	buf += Read_FileFile(filename, buf);

	g_regions = (uint16 *)buf;

	InitRegions();
}

/**
 * Check if a spriteID is part of the veiling sprites.
 * @param spriteID The sprite to check for.
 * @return True if and only if the spriteID is part of the veiling sprites.
 */
bool Sprite_Revealed(uint16 spriteID)
{
	if (spriteID > g_veiledSpriteID) return true;
	if (spriteID < g_veiledSpriteID - 15) return true;

	return false;
}

void Sprites_Init(void)
{
	g_spriteBuffer = calloc(1, 20000);
	Sprites_Load("MOUSE.SHP");                       /*   0 -   6 */
	Sprites_Load(Language_Name("BTTN"));   /*   7 -  11 */
	Sprites_Load("SHAPES.SHP");                      /*  12 - 110 */
	Sprites_Load("UNITS2.SHP");                      /* 111 - 150 */
	Sprites_Load("UNITS1.SHP");                      /* 151 - 237 */
	Sprites_Load("UNITS.SHP");                       /* 238 - 354 */
	Sprites_Load(Language_Name("CHOAM"));  /* 355 - 372 */
	Sprites_Load(Language_Name("MENTAT")); /* 373 - 386 */
	Sprites_Load("MENSHPH.SHP");                     /* 387 - 401 */
	Sprites_Load("MENSHPA.SHP");                     /* 402 - 416 */
	Sprites_Load("MENSHPO.SHP");                     /* 417 - 431 */
	Sprites_Load("MENSHPM.SHP");                     /* 432 - 446 (Placeholder - Fremen) */
	Sprites_Load("MENSHPM.SHP");                     /* 447 - 461 (Placeholder - Sardaukar) */
	Sprites_Load("MENSHPM.SHP");                     /* 462 - 476 */
	Sprites_Load("PIECES.SHP");                      /* 477 - 504 */
	Sprites_Load("ARROWS.SHP");                      /* 505 - 513 */
	Sprites_Load("CREDIT1.SHP");                     /* 514 */
	Sprites_Load("CREDIT2.SHP");                     /* 515 */
	Sprites_Load("CREDIT3.SHP");                     /* 516 */
	Sprites_Load("CREDIT4.SHP");                     /* 517 */
	Sprites_Load("CREDIT5.SHP");                     /* 518 */
	Sprites_Load("CREDIT6.SHP");                     /* 519 */
	Sprites_Load("CREDIT7.SHP");                     /* 520 */
	Sprites_Load("CREDIT8.SHP");                     /* 521 */
	Sprites_Load("CREDIT9.SHP");                     /* 522 */
	Sprites_Load("CREDIT10.SHP");                    /* 523 */
	Sprites_Load("CREDIT11.SHP");                    /* 524 */
}

void Sprites_Uninit(void)
{
	uint16 i;

	for (i = 0; i < s_spritesCount; i++) free(g_sprites[i]);
	free(g_sprites); g_sprites = NULL;

	free(g_spriteBuffer); g_spriteBuffer = NULL;

	free(g_mouseSpriteBuffer); g_mouseSpriteBuffer = NULL;
	free(g_mouseSprite); g_mouseSprite = NULL;

	free(g_spritePixels); g_spritePixels = NULL;
	free(g_iconRTBL); g_iconRTBL = NULL;
	free(g_iconRPAL); g_iconRPAL = NULL;

	free(g_iconMap); g_iconMap = NULL;
}
