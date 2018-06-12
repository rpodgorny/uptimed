/*
uptimed - Copyright (c) 1998-2004 Rob Kaper <rob@unixcode.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program;  if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef xPLATFORM_BSD
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#ifdef xPLATFORM_SOLARIS
#include <unistd.h>
#include <sys/time.h>
#include <utmpx.h>
#include <fcntl.h>
#endif

#ifdef xPLATFORM_HPUX
#include <unistd.h>
#include <sys/param.h>
extern void snprintf(char *,...);
#define _INCLUDE_HPUX_SOURCE
#include <sys/pstat.h>
#endif

#ifdef xPLATFORM_UNKNOWN
#include <time.h>
#endif

#include "misc.h"

typedef struct milestone
{
	time_t	time; /* uptime to reach */
	char	desc[SYSMAX+1]; /* little description */
	struct milestone *next;
} Milestone;

extern Milestone *milestone_list;

Milestone *add_milestone(time_t, char *);
void del_milestone(Milestone *milestone);
Milestone *find_next_milestone(time_t);
time_t scantime(char *);
