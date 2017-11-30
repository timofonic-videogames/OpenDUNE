/** @file src/audio/mt32mpu.h MPU definitions. */

#ifndef MT32MPU_H
#define MT32MPU_H

extern void MPU_Interrupt(void);
extern uint16 MPU_SetData(uint8 *file, uint16 index, void *msdata);
extern void MPU_Play(uint16 index);
extern void MPU_Stop(uint16 index);
extern uint16 AIL_sequence_status(uint16 index);
extern uint16 MPU_GetDataSize(void);
extern bool MPU_Init(void);
extern void MPU_Uninit(void);
extern void MPU_ClearData(uint16 index);
extern void MPU_SetVolume(uint16 index, uint16 volume, uint16 time);
#if defined(_WIN32)
extern void MPU_StartThread(uint32 usec);
extern void MPU_StopThread(void);
#endif /* _WIN32 */

#endif /* MT32MPU_H */
