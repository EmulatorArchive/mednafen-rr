#ifndef AVIOUT_H_INCLUDED
#define AVIOUT_H_INCLUDED

#include "prefix.h"

#include "windows.h"
#include "mednafen.h"

#ifdef __cplusplus
extern "C" {
#endif
int DRV_AviIsRecording();
int DRV_AviBegin(const char* fname);

void DRV_AviEnd();
void DRV_AviVideoUpdate(const uint16* buffer, EmulateSpecStruct *espec);
void DRV_AviSoundUpdate(void* soundData, int soundLen);
#ifdef __cplusplus
}
#endif
#endif