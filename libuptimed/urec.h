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
#include <sys/stat.h>

#ifdef PLATFORM_LINUX
#include <sys/sysinfo.h>
#endif

#ifdef PLATFORM_BSD
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#ifdef PLATFORM_SOLARIS
#include <unistd.h>
#include <sys/time.h>
#include <utmpx.h>
#include <fcntl.h>
#endif

#ifdef PLATFORM_HPUX
#include <unistd.h>
#include <sys/param.h>
extern void snprintf(char *, ...);
#define _INCLUDE_HPUX_SOURCE
#include <sys/pstat.h>
#endif

#ifdef PLATFORM_UNKNOWN
#include <time.h>
#endif

#include "misc.h"

#ifdef __ANDROID__
#define FILE_BOOTID "/data/uptimed/bootid"
#define FILE_RECORDS "/data/uptimed/records"
#else
#define FILE_BOOTID "/var/spool/uptimed/bootid"
#define FILE_RECORDS "/var/spool/uptimed/records"
#endif

typedef struct urec {
	time_t utime; /* uptime */
	time_t btime; /* time of boot up */
	time_t dtime; /* downtime */
	char sys[SYSMAX+1]; /* system type */
	struct urec *next;
} Urec;

extern Urec *urec_list;
extern Urec *u_current;

Urec *add_urec(time_t, time_t, char *);
void del_urec(Urec *urec);
void moveup(void);
char *read_sysinfo(void);
time_t read_uptime(void);
void calculate_downtime(void);
void read_records(time_t);
void save_records(int, time_t);
#ifndef PLATFORM_BSD
int createbootid(void);
#endif
int compare_urecs(Urec *, Urec *, int);
Urec *sort_urec(Urec *, int);
time_t readbootid(void);
