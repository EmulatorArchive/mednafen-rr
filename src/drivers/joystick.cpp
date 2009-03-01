/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *  Copyright (C) 2002 Paul Kuliniewicz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* PK: SDL joystick input stuff */

#include <errno.h>

#include "main.h"
#include "input.h"

#include "../md5.h"

#define MAX_JOYSTICKS	32
static SDL_Joystick *Joysticks[MAX_JOYSTICKS] = { NULL };
static uint64 UniqueID[MAX_JOYSTICKS];
static int AnalogThreshold;

uint64 GetJoystickUniqueID(SDL_Joystick *joystick)
{
 uint8 digest[16];
 int tohash[4];
 md5_context hashie;
 uint64 ret = 0;
 int x;

 tohash[0] = SDL_JoystickNumAxes(joystick);
 tohash[1] = SDL_JoystickNumBalls(joystick);
 tohash[2] = SDL_JoystickNumHats(joystick);
 tohash[3] = SDL_JoystickNumButtons(joystick);

 hashie.starts();
 hashie.update((uint8 *)tohash, sizeof(tohash));
 hashie.finish(digest);

 for(x=0;x<16;x++)
 {
  ret ^= (uint64)digest[x] << ((x & 7) * 8);
 }

 return(ret);
}

uint64 GetJoystickUniqueID(int num) // Called by input-config.cpp code after joysticks are initialized
{
 return(UniqueID[num]);
}

int DTestButtonJoy(ButtConfig &bc)
{
 if(bc.DeviceNum >= MAX_JOYSTICKS)
  return(0);

 if(bc.ButtonNum & 0x8000)	/* Axis "button" */
 {
	int pos;
        pos = SDL_JoystickGetAxis(Joysticks[bc.DeviceNum], bc.ButtonNum&16383);

        if ((bc.ButtonNum & 0x4000) && pos <= -AnalogThreshold)
	 return(1);
        else if (!(bc.ButtonNum & 0x4000) && pos >= AnalogThreshold)
	 return(1);
  }
  else if(bc.ButtonNum & 0x2000)	/* Hat "button" */
  {
   if( SDL_JoystickGetHat(Joysticks[bc.DeviceNum],(bc.ButtonNum>>8)&0x1F) & (bc.ButtonNum&0xFF))
    return(1);
  }
  else
   if(SDL_JoystickGetButton(Joysticks[bc.DeviceNum], bc.ButtonNum)) 
    return(1);

 return(0);
}

/* Cleanup opened joysticks. */
int KillJoysticks (void)
{
	for (int n = 0; n < MAX_JOYSTICKS; n++)
	{
		if (Joysticks[n] != 0)
 	  	 SDL_JoystickClose(Joysticks[n]);
		Joysticks[n] = 0;
	}
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	return(1);
}

/* Initialize joysticks. */
int InitJoysticks (void)
{
	int total;

	MDFN_printf("Initializing joysticks...\n");
	MDFN_indent(1);

        SDL_InitSubSystem(SDL_INIT_JOYSTICK);

	total = SDL_NumJoysticks();
	if(total > MAX_JOYSTICKS) total = MAX_JOYSTICKS;

	for(int n = 0; n < total; n++)
	{
 	 /* Open the joystick under SDL. */
	 Joysticks[n] = SDL_JoystickOpen(n);
	 if(Joysticks[n])
	 {
	  UniqueID[n] = GetJoystickUniqueID(Joysticks[n]);

	  int plusplus = 0;
	  for(int x = 0; x < n; x++)
	  {
	   if(UniqueID[x] == UniqueID[n])
	    plusplus++;
	  }
	  UniqueID[n] += plusplus;	// Work around hash collisions
	  MDFN_printf(_("Joystick %d - %s - Unique ID: %016llx\n"), n, SDL_JoystickName(n), UniqueID[n]);
	 }
	 //printf("Could not open joystick %d: %s.\n",
	 //joy[n] - 1, SDL_GetError());
 	 continue;
	}

	AnalogThreshold = (int) ((32767 * MDFN_GetSettingF("analogthreshold") / 100) + 0.5); //adelikat: adding +0.5 to induce rounding instead of truncating. Also, adding explicit cast.
	MDFN_indent(-1);
	return(1);
}


static int FindByUniqueID(uint64 id)
{
 int x;

 for(x=0; x< MAX_JOYSTICKS; x++)
  if(Joysticks[x])
  {
   if(UniqueID[x] == id)
    return(x);
  }

 return(0xFFFF);
}

void JoyClearBC(ButtConfig &bc)
{
 bc.DeviceNum = FindByUniqueID(bc.DeviceID);
}

