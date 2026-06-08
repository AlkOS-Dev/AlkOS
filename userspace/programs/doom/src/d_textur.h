//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Typedefs related to to textures etc.,
//	 isolated here to make it easier separating modules.
//

#ifndef USERSPACE_PROGRAMS_DOOM_SRC_D_TEXTUR_H_
#define USERSPACE_PROGRAMS_DOOM_SRC_D_TEXTUR_H_

#include "doomtype.h"

//
// Flats?
//
// a pic is an unmasked block of pixels
typedef struct {
    byte width;
    byte height;
    byte data;
} pic_t;

#endif  // USERSPACE_PROGRAMS_DOOM_SRC_D_TEXTUR_H_
