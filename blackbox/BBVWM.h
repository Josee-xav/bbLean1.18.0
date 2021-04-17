/* ==========================================================================

  This file is part of the bbLean source code
  Copyright © 2001-2003 The Blackbox for Windows Development Team
  Copyright © 2004-2009 grischka

  http://bb4win.sourceforge.net/bblean
  http://developer.berlios.de/projects/bblean

  bbLean is free software, released under the GNU General Public License
  (GPL version 2). For details see:

  http://www.fsf.org/licenses/gpl.html

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  ========================================================================== */
#pragma once

struct winlist;

//=========================================================
// Init/exit

void vwm_init ();
void vwm_reconfig (bool force);
void vwm_exit ();

//=========================================================
// update the list

void vwm_update_winlist ();
winlist * vwm_add_window (HWND hwnd);

//=========================================================
// set workspace

void vwm_switch (int newdesk);
void vwm_gather ();

//=========================================================
// Set window properties

bool vwm_set_desk (HWND hwnd, int desk, bool switchto);
bool vwm_set_location (HWND hwnd, taskinfo const * t, unsigned flags);
bool vwm_set_sticky (HWND hwnd, bool set);
bool vwm_set_onbg (HWND hwnd, bool set);
bool vwm_lower_window (HWND hwnd);

// Workaround for BBPager:
bool vwm_set_workspace (HWND hwnd, int desk);

//=========================================================
// status infos about windows

int vwm_get_desk (HWND hwnd);
bool vwm_get_location (HWND hwnd, taskinfo * t);

// values for "what":
enum E_VwmStatus {
    VWM_MOVED  = 1,
    VWM_HIDDEN,
    VWM_STICKY,
    VWM_ICONIC,
    VWM_ONBG
};
bool vwm_get_status (HWND hwnd, E_VwmStatus what);

//=========================================================
// required variables/functions from elswhere:
// vwm options
extern bool Settings_altMethod, Settings_styleXPFix;

