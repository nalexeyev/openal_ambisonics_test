/*
 * openal-info.h
 *
 *  Created on: Mar 1, 2017
 *      Author: nick
 */

#ifndef INCLUDE_OPENAL_INFO_H_
#define INCLUDE_OPENAL_INFO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AL/alc.h"
#include "AL/al.h"
#include "AL/alext.h"


#ifndef ALC_ENUMERATE_ALL_EXT
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif

#ifndef ALC_EXT_EFX
#define ALC_EFX_MAJOR_VERSION                    0x20001
#define ALC_EFX_MINOR_VERSION                    0x20002
#define ALC_MAX_AUXILIARY_SENDS                  0x20003
#endif

void printList(const char *list, char separator);
void printDeviceList(const char *list);
ALenum checkALErrors(int linenum);
//#define checkALErrors() checkALErrors(__LINE__)
ALCenum checkALCErrors(ALCdevice *device, int linenum);
//#define checkALCErrors(x) checkALCErrors((x),__LINE__)

void printALCInfo(ALCdevice *device);
void printHRTFInfo(ALCdevice *device);
void printALInfo(void);
void printEFXInfo(ALCdevice *device);

int showInfo(int argc, char *argv[]);

#define MAX_WIDTH  80


#endif /* INCLUDE_OPENAL_INFO_H_ */
