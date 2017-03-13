/*
 * loadosundSDL.h
 *
 *  Created on: Mar 13, 2017
 *      Author: nick
 */

#ifndef INCLUDE_LOADSOUNDSDL_H_
#define INCLUDE_LOADSOUNDSDL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AL/alc.h"
#include "AL/al.h"
#include "AL/alext.h"

#include <SDL/SDL_sound.h>

static ALuint LoadSoundSDL(const char *filename);

#endif /* INCLUDE_LOADSOUNDSDL_H_ */
