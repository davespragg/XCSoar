/*

	DebugLog

Copyright_License {

  XCSoar Glide Computer - http://xcsoar.sourceforge.net/
  Copyright (C) 2000 - 2005  

  	M Roberts
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@bigfoot.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

}

*/


// Log Level
enum {
	DL_LOW,
	DL_MEDIUM,
	DL_HIGH
};

// TODO Think more about the set of event
// Log Type
enum {
	DL_GENERAL,
	DL_CONFIG,
	DL_LOG,
	DL_INPUT

};


// What level do we want to compile in
#define DL_COMPILE_LEVEL DL_LOW

// What types do we want to compile in
#define DL_COMPILE_TYPE DL_GENERAl & DL_CONFIG & DL_LOG & DL_INPUT

/*

	Check level and type before calling real event, thus removing those

	Then call real function
*/


#define DEBUGLOG(type, level, string) 

