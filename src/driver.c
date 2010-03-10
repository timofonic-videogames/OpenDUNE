/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "libemu.h"
#include "global.h"
#include "file.h"
#include "driver.h"
#include "mt32mpu.h"
#include "os/strings.h"

extern void emu_CustomTimer_AddHandler();
extern void emu_Drivers_Load();
extern void f__01F7_27FD_0037_E2C0();
extern void f__1DD7_1696_0011_A4E3();
extern void f__23E1_0004_0014_2BC0();
extern void f__23E1_01C2_0011_24E8();
extern void f__2756_07DA_0048_9F5D();
extern void f__2756_0A59_0023_D969();
extern void f__2756_0B8F_0025_D5D8();
extern void f__2756_0C0B_0021_873C();
extern void f__2756_0C31_0037_2A81();
extern void f__2756_0D12_0042_A9FA();
extern void f__2756_0D5F_0012_AE08();
extern void emu_MPU_TestPort();
extern void emu_DSP_GetInfo();
extern void emu_DSP_TestPort();
extern void emu_MPU_GetInfo();
extern void f__AB00_0B91_0014_89BD();
extern void f__AB00_0C08_0013_3E08();
extern void f__AB00_0DA4_0078_0101();
extern void f__AB00_1068_0020_E6F1();
extern void f__AB00_1122_001C_9408();
extern void f__AB00_118F_0029_4B06();
extern void f__AB00_1235_0013_28BA();
extern void f__AB01_0F24_0044_3584();
extern void emu_MPU_Init();
extern void f__AB01_2103_0040_93D2();
extern void f__AB01_2336_002C_4FDC();
extern void f__AB01_26EB_0047_41F4();

/* TOC bug: extern void f__AB00_02AB_000F_A43F(); */

bool Drivers_Init(const char *filename, csip32 fcsip, Driver *driver, csip32 dcsip, const char *extension, uint16 variable_0008)
{
	if (filename == NULL || !File_Exists(filename)) return false;

	emu_si = emu_get_memory16(emu_ss, emu_bp,  0x12);

	if (driver->dcontent.csip != 0) {
		if (strcasecmp((char *)emu_get_memorycsip(driver->dfilename), filename) == 0) return true;
		emu_push(dcsip.s.cs); emu_push(dcsip.s.ip);
		emu_push(emu_cs); emu_push(0x1330); f__1DD7_1696_0011_A4E3();
		emu_sp += 4;
	}

	emu_push(fcsip.s.cs); emu_push(fcsip.s.ip);
	emu_push(emu_cs); emu_push(0x133D); emu_Drivers_Load();
	emu_sp += 4;

	driver->dcontent.s.cs = emu_dx;
	driver->dcontent.s.ip = emu_ax;

	if (driver->dcontent.csip == 0) return false;

	if (variable_0008 != 0) {
		emu_bx = 0x2;
		emu_pushf();

		/* Call based on memory/register values */
		emu_ip = driver->dcontent.s.ip;
		emu_push(emu_cs);
		emu_cs = driver->dcontent.s.cs;
		emu_push(0x136D);
		switch ((emu_cs << 16) + emu_ip) {
			default:
				/* In case we don't know the call point yet, call the dynamic call */
				emu_last_cs = 0x1DD7; emu_last_ip = 0x136A; emu_last_length = 0x0007; emu_last_crc = 0x2888;
				emu_call();
				return false;
		}

		if ((variable_0008 & 0x8000) != 0) {
			emu_ax = 0x4;
			emu_bx = 0x10;
			emu_pushf();

			/* Call based on memory/register values */
			emu_ip = driver->dcontent.s.ip;
			emu_push(emu_cs);
			emu_cs = driver->dcontent.s.cs;
			emu_push(0x137B);
			switch ((emu_cs << 16) + emu_ip) {
				default:
					/* In case we don't know the call point yet, call the dynamic call */
					emu_last_cs = 0x1DD7; emu_last_ip = 0x1378; emu_last_length = 0x000E; emu_last_crc = 0xB970;
					emu_call();
					return false;
			}
		}

		emu_push(driver->dcontent.s.cs); emu_push(driver->dcontent.s.ip + 3);
		emu_push(emu_cs); emu_push(0x138F); emu_cs = 0x2756; emu_CustomTimer_AddHandler();
		emu_sp += 4;

		driver->customTimer = emu_ax;
		if (driver->customTimer == 0xFFFF) {
			emu_push(driver->dcontent.s.cs); emu_push(driver->dcontent.s.ip);
			emu_push(emu_cs); emu_push(0x13B2); emu_cs = 0x23E1; f__23E1_01C2_0011_24E8();
			emu_sp += 4;

			driver->dcontent.csip = 0;
			return false;
		}

		{
			int32 value = (int32)variable_0008;
			if (value < 0) value = -value;

			emu_push(value >> 16); emu_push(value & 0xFFFF);
			emu_push(driver->customTimer);
			emu_push(emu_cs); emu_push(0x13E1); emu_cs = 0x2756; f__2756_0B8F_0025_D5D8();
			emu_sp += 6;
		}
		emu_push(driver->customTimer);
		emu_push(emu_cs); emu_push(0x13F0); emu_cs = 0x2756; f__2756_0A59_0023_D969();
		emu_sp += 2;

		strcpy(driver->extension, (strcasecmp(filename, "alfx.drv") == 0) ? "adl" : "snd");
	} else {
		csip32 csip;

		emu_push(driver->dcontent.s.cs); emu_push(driver->dcontent.s.ip);
		emu_push(emu_cs); emu_push(0x1437); emu_cs = 0x2756; f__2756_0C31_0037_2A81();
		emu_sp += 4;

		driver->index = emu_ax;

		if (driver->index == 0xFFFF) {
			emu_push(driver->dcontent.s.cs); emu_push(driver->dcontent.s.ip);
			emu_push(emu_cs); emu_push(0x13B2); emu_cs = 0x23E1; f__23E1_01C2_0011_24E8();
			emu_sp += 4;

			driver->dcontent.csip = 0;
			return false;
		}

		emu_push(driver->index);
		emu_push(emu_cs); emu_push(0x1456); emu_cs = 0x2756; f__2756_0C0B_0021_873C();
		emu_sp += 2;

		csip.s.cs = emu_dx;
		csip.s.ip = emu_ax;

		memcpy(driver->variable_0A, &emu_get_memory8(csip.s.cs, csip.s.ip, 4), 4);
		strcpy(driver->extension, "xmi");

		if (strcasecmp(filename, "sbdig.adv") == 0 || strcasecmp(filename, "sbpdig.adv") == 0) {
			csip32 blaster;

			emu_push(emu_ds); emu_push(0x65FE); /* "BLASTER" */
			emu_push(emu_cs); emu_push(0x14CF); emu_cs = 0x01F7; f__01F7_27FD_0037_E2C0();
			emu_sp += 4;

			blaster.s.cs = emu_dx;
			blaster.s.ip = emu_ax;

			if (blaster.csip != 0) {
				char *val;

				val = strchr((char*)emu_get_memorycsip(blaster), 'A');
				if (val != NULL) {
					val++;
					emu_get_memory16(csip.s.cs, csip.s.ip, 0xC) = (uint16)strtoul(val, NULL, 16);
				}

				val = strchr((char*)emu_get_memorycsip(blaster), 'I');
				if (val != NULL) {
					val++;
					emu_get_memory16(csip.s.cs, csip.s.ip, 0x12) = (uint16)strtoul(val, NULL, 10);
					emu_get_memory16(csip.s.cs, csip.s.ip, 0xE)  = (uint16)strtoul(val, NULL, 10);
				}

				val = strchr((char*)emu_get_memorycsip(blaster), 'D');
				if (val != NULL) {
					val++;
					emu_get_memory16(csip.s.cs, csip.s.ip, 0x10) = (uint16)strtoul(val, NULL, 10);
				}
			}
		}

		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0x12));
		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0x10));
		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0xE));
		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0xC));
		emu_push(driver->index); /* unused, but needed for correct param accesses. */
		emu_ax = Drivers_CallFunction(driver->index, 0x65).s.ip;
		emu_sp += 8;

		if (emu_ax == 0) {
			emu_push(driver->index);
			emu_push(emu_cs); emu_push(0x15D8); emu_cs = 0x2756; f__2756_0D12_0042_A9FA();
			emu_sp += 2;

			emu_push(driver->dcontent.s.cs); emu_push(driver->dcontent.s.ip);
			emu_push(emu_cs); emu_push(0x13B2); emu_cs = 0x23E1; f__23E1_01C2_0011_24E8();
			emu_sp += 4;

			driver->dcontent.csip = 0;
			return false;
		}

		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0x12));
		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0x10));
		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0xE));
		emu_push(emu_get_memory16(csip.s.cs, csip.s.ip, 0xC));
		emu_push(driver->index);
		emu_push(emu_cs); emu_push(0x1603); emu_cs = 0x2756; f__2756_0D5F_0012_AE08();
		emu_sp += 10;

		{
			int32 value = (int32)Drivers_CallFunction(driver->index, 0x99).s.ip;
			if (value != 0) {
				emu_push(0);
				emu_push(value >> 16); emu_push(value & 0xFFFF);
				emu_push(emu_cs); emu_push(0x1625); emu_cs = 0x23E1; f__23E1_0004_0014_2BC0();
				/* Unresolved jump */ emu_ip = 0x1625; emu_last_cs = 0x1DD7; emu_last_ip = 0x1625; emu_last_length = 0x0014; emu_last_crc = 0x9695; emu_call();
			}
		}
	}

	driver->dfilename = fcsip;
	driver->extension[0] = 0;
	if (driver->dcontent.csip != 0) memcpy(driver->extension, extension, 4);

	return true;
}

uint16 Drivers_Sound_Init(uint16 index)
{
	char *filename;
	MSDriver *driver;
	Driver *sound;
	Driver *music;

	driver = &g_global->soundDrv[index];
	sound  = &g_global->soundDriver;
	music  = &g_global->musicDriver;

	if (driver->filename.csip == 0x0) return index;

	filename = (char *)emu_get_memorycsip(driver->filename);

	if (music->dfilename.csip != 0x0 && !strcasecmp((char *)emu_get_memorycsip(music->dfilename), filename)) {
		memcpy(sound, music, sizeof(Driver));
	} else {
		csip32 sound_csip;
		sound_csip.csip = 0x353F6302;
		if (!Drivers_Init(filename, driver->filename, sound, sound_csip, (char *)emu_get_memorycsip(driver->extension), driver->variable_0008)) return 0;
	}

	if (driver->variable_0008 == 0) {
		uint8 i;
		int32 value;

		value = (int32)Drivers_CallFunction(sound->index, 0x96).s.ip;

		for (i = 0; i < 4; i++) {
			MSBuffer *buf = &g_global->soundBuffer[i];

			emu_push(0x10);
			emu_push(value >> 16); emu_push(value & 0xFFFF);
			emu_push(emu_cs); emu_push(0x1014); emu_cs = 0x23E1; f__23E1_0004_0014_2BC0();
			emu_sp += 6;

			buf->buffer.s.cs = emu_dx;
			buf->buffer.s.ip = emu_ax;
			buf->index       = 0xFFFF;
		}
		g_global->soundBufferIndex = 0;
	}

	if (driver->variable_000A != 0) {
		g_global->variable_6328 = 1;
	}

	return index;
}

uint16 Drivers_Music_Init(uint16 index)
{
	char *filename;
	MSDriver *driver;
	Driver *music;
	Driver *sound;
	int32 value;

	driver = &g_global->musicDrv[index];
	music  = &g_global->musicDriver;
	sound  = &g_global->soundDriver;

	if (driver->filename.csip == 0x0) return index;

	filename = (char *)emu_get_memorycsip(driver->filename);

	if (sound->dfilename.csip != 0x0 && !strcasecmp((char *)emu_get_memorycsip(sound->dfilename), filename)) {
		memcpy(music, sound, sizeof(Driver));
	} else {
		csip32 music_csip;
		music_csip.csip = 0x353F6344;
		if (!Drivers_Init(filename, driver->filename, music, music_csip, (char *)emu_get_memorycsip(driver->extension), driver->variable_0008)) return 0;
	}

	g_global->variable_636A = driver->variable_000A;
	if (driver->variable_0008 != 0) return index;

	value = (int32)Drivers_CallFunction(music->index, 0x96).s.ip;

	emu_push(0x10);
	emu_push(value >> 16); emu_push(value & 0xFFFF);
	emu_push(emu_cs); emu_push(0x1014); emu_cs = 0x23E1; f__23E1_0004_0014_2BC0();
	emu_sp += 6;

	g_global->musicBuffer.buffer.s.cs = emu_dx;
	g_global->musicBuffer.buffer.s.ip = emu_ax;
	g_global->musicBuffer.index       = 0xFFFF;

	return index;
}

uint16 Drivers_Voice_Init(uint16 index)
{
	char *filename;
	DSDriver *driver;
	Driver *voice;
	csip32 voice_csip;

	driver = &g_global->voiceDrv[index];
	voice  = &g_global->voiceDriver;

	if (driver->filename.csip == 0x0) return index;

	filename = (char *)emu_get_memorycsip(driver->filename);
	voice_csip.csip = 0x353F6374;

	if (!Drivers_Init(filename, driver->filename, voice, voice_csip, "VOC", 0)) return 0;

	return index;
}

void Drivers_All_Init(uint16 sound, uint16 music, uint16 voice)
{
	emu_push(emu_cs); emu_push(0x03A3); emu_cs = 0x2756; f__2756_07DA_0048_9F5D();

	emu_push(0x2BD1); emu_push(0x6);
	emu_push(emu_cs); emu_push(0x03B0); emu_cs = 0x2756; emu_CustomTimer_AddHandler();
	emu_sp += 2;
	g_global->variable_639C = emu_ax;

	emu_push(0);
	emu_push(0x3C);
	emu_push(g_global->variable_639C);
	emu_push(emu_cs); emu_push(0x03C5); emu_cs = 0x2756; f__2756_0B8F_0025_D5D8();
	emu_sp += 6;

	emu_push(g_global->variable_639C);
	emu_push(emu_cs); emu_push(0x03D1); emu_cs = 0x2756; f__2756_0A59_0023_D969();
	emu_sp += 2;

	g_global->variable_6D8D = Drivers_Music_Init(music);
	g_global->variable_6D8B = Drivers_Sound_Init(sound);
	g_global->variable_6D8F = Drivers_Voice_Init(voice);
}

csip32 Drivers_GetFunctionCSIP(uint16 driver, uint16 function)
{
	csip32 csip;

	if (driver >= 10) {
		csip.csip = 0;
		return csip;
	}

	csip = emu_get_csip32(0x2756, driver * 4, 0x128);
	if (csip.csip == 0) return csip;

	for (;; csip.s.ip += 4) {
		uint16 f = emu_get_memory16(csip.s.cs, csip.s.ip, 0);
		if (f == function) break;
		if (f != 0xFFFF) continue;
		csip.csip = 0;
		return csip;
	}

	csip.s.ip = emu_get_memory16(csip.s.cs, csip.s.ip, 2);
	return csip;
}

csip32 Drivers_CallFunction(uint16 driver, uint16 function)
{
	csip32 csip;

	csip = Drivers_GetFunctionCSIP(driver, function);
	if (csip.csip == 0) return csip;

	/* Fake return CS:IP */
	emu_sp -= 4;

	/* Call/jump based on memory/register values */
	emu_ip = csip.s.ip;
	emu_cs = csip.s.cs;
	switch ((emu_cs << 16) + emu_ip) {
		case 0x44AF045A: emu_MPU_TestPort(); break; /* 0x65 */
		case 0x44AF0B73: emu_DSP_GetInfo(); break; /* 0x64 */
		case 0x44AF0B91: f__AB00_0B91_0014_89BD(); break; /* 0x68 */
		case 0x44AF0C08: f__AB00_0C08_0013_3E08(); break; /* 0x81 */
		case 0x44AF0C3F: emu_DSP_TestPort(); break; /* 0x65 */
		case 0x44AF0C96: emu_MPU_GetInfo(); break; /* 0x64 */
		case 0x44AF0DA4: f__AB00_0DA4_0078_0101(); break; /* 0x66 */
		case 0x44AF0F02: emu_MPU_GetUnknownSize(); break; /* 0x99 */
		case 0x44AF0F24: f__AB01_0F24_0044_3584(); break; /* 0x9B */
		case 0x44AF1068: f__AB00_1068_0020_E6F1(); break; /* 0x7B */
		case 0x44AF1122: f__AB00_1122_001C_9408(); break; /* 0x7D */
		case 0x44AF118F: f__AB00_118F_0029_4B06(); break; /* 0x7E */
		case 0x44AF1235: f__AB00_1235_0013_28BA(); break; /* 0x7C */
		case 0x44AF1FA8: emu_MPU_Init(); break; /* 0x66 */
		case 0x44AF2103: f__AB01_2103_0040_93D2(); break; /* 0x68 */
		case 0x44AF2191: emu_MPU_GetDataSize(); break; /* 0x96 */
		case 0x44AF21F0: emu_MPU_SetData(); break; /* 0x97 */
		case 0x44AF2336: f__AB01_2336_002C_4FDC(); break; /* 0x98 */
		case 0x44AF237A: emu_MPU_Play(); break; /* 0xAA */
		case 0x44AF240F: emu_MPU_Stop(); break; /* 0xAB */
		case 0x44AF26EB: f__AB01_26EB_0047_41F4(); break; /* 0xB1 */
		default:
			/* In case we don't know the call point yet, call the dynamic call */
			emu_last_cs = 0x2756; emu_last_ip = 0x050B; emu_last_length = 0x0003; emu_last_crc = 0x6FD4;
			emu_call();
			return csip; /* not reached */
	}

	csip.s.cs = emu_dx;
	csip.s.ip = emu_ax;
	return csip;
}

bool Driver_Music_IsPlaying()
{
	Driver *driver = &g_global->musicDriver;
	MSBuffer *buffer = &g_global->musicBuffer;

	if (driver->index == 0xFFFF) {
		if (driver->dcontent.csip == 0) return false;

		emu_ax = 0x0;
		emu_bx = 0x7;
		emu_pushf();

		/* Call based on memory/register values */
		emu_ip = driver->dcontent.s.ip;
		emu_push(emu_cs);
		emu_cs = driver->dcontent.s.cs;
		emu_push(0x08E2);
		switch ((emu_cs << 16) + emu_ip) {
			default:
				/* In case we don't know the call point yet, call the dynamic call */
				emu_last_cs = 0x1DD7; emu_last_ip = 0x08DF; emu_last_length = 0x0020; emu_last_crc = 0xBD30;
				emu_call();
				return false;
		}

		return emu_ax == 1;
	}

	if (buffer->index == 0xFFFF) return false;

	return MPU_IsPlaying(buffer->index) == 1;
}