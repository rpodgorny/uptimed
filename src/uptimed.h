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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <syslog.h>
#include <signal.h>

#include "milestone.h"
#include "misc.h"
#include "urec.h"

#define	EMAIL	128

int main(int, char *[]);
void bg(void);
void cheer(Milestone *, int);
time_t get_prev(void);
void read_config(void);
void handler(int);
void scan_args(int, char *[]);
void print_usage(char *[]);
void print_help(char *[]);
void print_version(void);
void mail(Milestone *, int);
