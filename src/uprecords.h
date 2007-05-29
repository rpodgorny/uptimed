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
#include <ctype.h>

#include "urec.h"
#include "milestone.h"
#include "misc.h"

#define	BOLD	"[1m"
#define	PLAIN	"[0m"
#define	CLS		"[H[J"

#define	PRE		1
#define	LIST	2
#define	TABLE	3

/* ctime returns a pointer to a 26-character string of the form:
           Thu Nov 24 18:22:48 1986\n\0
   of which we'll replace the \n by '\0', hence the 25 length */
#define	TIMEMAX	25

int main(int, char *[]);
void displayrecords(int);
void read_config(void);
void read_config_cgi(void);
void print_entry(time_t, char *, time_t, char *, int, int);
void print_line(void);
void print_downtime(time_t dtime, int hilite);
void scan_args(int, char *[]);
void print_usage(char *[]);
void print_help(char *[]);
void print_version(void);
