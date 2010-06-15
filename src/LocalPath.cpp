/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#include "LocalPath.hpp"
#include "Appearance.hpp"
#include "Compatibility/path.h"
#include "StringUtil.hpp"
#include "Asset.hpp"

#include <stdio.h>

#ifdef WIN32
#include <shlobj.h>
#endif

/*
	Get pathname & c. from GetModuleFilename (gmf)
	In case of problems, always return \ERRORxx\  as path name
	It will be displayed at startup and users will know that
	something is wrong reporting the error code to us.
	Approach not followed: It works but we don't know why
	Approach followed: It doesn't work and we DO know why

	These are temporary solutions to be improved
 */
const TCHAR*
gmfpathname()
{
#ifdef WIN32
  static TCHAR gmfpathname_buffer[MAX_PATH];
  TCHAR *p;

  if (GetModuleFileName(NULL, gmfpathname_buffer, MAX_PATH) <= 0)
    return(_T("\\ERROR_01\\") );

  if (gmfpathname_buffer[0] != '\\' )
    return(_T("\\ERROR_02\\"));

  // truncate for safety
  gmfpathname_buffer[MAX_PATH - 1] = '\0';

  for (p = gmfpathname_buffer + 1; *p != '\0'; p++)
    // search for the very first "\"
    if (*p == '\\')
      break;

  if (*p == '\0')
    return (_T("\\ERROR_03\\"));

  *++p = '\0';

  return gmfpathname_buffer;
#else
  return(_T("\\ERROR_01\\") );
#endif
}

void
LocalPath(TCHAR *buffer, const TCHAR *file)
{
  /*
  loc = CSIDL_PROGRAMS

  File system directory that contains the user's program groups (which
  are also file system directories).

  CSIDL_PERSONAL               File system directory that serves as a common
                               repository for documents.

  CSIDL_PROGRAM_FILES 0x0026   The program files folder.
  */
  #if defined(GNAV) && !defined(PCGNAV)
    _tcscpy(buffer, _T("\\NOR Flash"));
  #elif defined (PNA) && !defined(WINDOWSPC)
   /*
    * VENTA-ADDON "smartpath" for PNA only
    *
    * (moved up elif from bottom to here to prevent messy behaviour if a
    * PNA exec is loaded on a PPC)
    *
    * For PNAs the localpath is taken from the application exec path
    * example> \sdmmc\bin\Program.exe  results in localpath=\sdmmc\XCSoarData
    *
    * Then the basename is searched for an underscore char, which is
    * used as a separator for getting the model type.  example>
    * program_pna.exe results in GlobalModelType=pna
    *
    */

    /*
     * Force LOCALPATH to be the same of the executing program
     */
    _stprintf(buffer, _T("%s%s"), gmfpathname(), XCSDATADIR);
  // VENTA2 FIX PC BUG
  #elif defined (FIVV) && !defined(WINDOWSPC)
    _stprintf(buffer, _T("%s%s"), gmfpathname(), XCSDATADIR);
  #elif !defined(_WIN32) || defined(__WINE__)
    /* on Unix or WINE, use ~/.xcsoar */
    const char *home = getenv("HOME");
    if (home != NULL)
      _stprintf(buffer, _T("%s/.xcsoar"), home);
    else
      _tcscat(buffer, _T("/etc/xcsoar"));
  #else
    // everything else that's not special
    SHGetSpecialFolderPath(NULL, buffer, CSIDL_PERSONAL, false);
    _tcscat(buffer, _T(DIR_SEPARATOR_S));
    _tcscat(buffer, XCSDATADIR);
  #endif

  if (!string_is_empty(file)) {
    _tcsncat(buffer, _T(DIR_SEPARATOR_S), MAX_PATH);
    _tcsncat(buffer, file, MAX_PATH);
  }
}

#ifndef HAVE_POSIX
void
LocalPath(char *buffer, const TCHAR *file)
{
  TCHAR wbuffer[MAX_PATH];
  LocalPath(wbuffer, file);
  sprintf(buffer, "%S", wbuffer);
}
#endif

/**
 * Convert backslashes to slashes on platforms where it matters.
 * @param p Pointer to the string to normalize
 */
static void
normalize_backslashes(TCHAR *p)
{
#if !defined(_WIN32) || defined(__WINE__)
  while ((p = _tcschr(p, '\\')) != NULL)
    *p++ = '/';
#endif
}

void
ExpandLocalPath(TCHAR* filein)
{
  // If no string to convert -> return
  if (string_is_empty(filein))
    return;

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = _T("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];

  // Get the XCSoarData folder location (lpath)
  LocalPath(lpath);

  // Get the relative file name and location (ptr)
  const TCHAR *ptr = string_after_prefix(filein, code);
  if (!ptr || string_is_empty(ptr))
    return;

  // Replace the code "%LOCAL_PATH%\\" by the full local path (output)
  _stprintf(output, _T("%s%s"), lpath, ptr);
  // ... and copy it to the buffer (filein)
  _tcscpy(filein, output);

  // Normalize the backslashes (if necessary)
  normalize_backslashes(filein);
}

void
ContractLocalPath(TCHAR* filein)
{
  // If no string to convert -> return
  if (string_is_empty(filein))
    return;

  TCHAR lpath[MAX_PATH];
  TCHAR code[] = _T("%LOCAL_PATH%\\");
  TCHAR output[MAX_PATH];

  // Get the XCSoarData folder location (lpath)
  LocalPath(lpath);

  // Get the relative file name and location (ptr)
  const TCHAR *ptr = string_after_prefix(filein, lpath);
  if (!ptr || string_is_empty(ptr))
    return;

  // Replace the full local path by the code "%LOCAL_PATH%\\" (output)
  _stprintf(output, _T("%s%s"), code, ptr);
  // ... and copy it to the buffer (filein)
  _tcscpy(filein, output);
}

