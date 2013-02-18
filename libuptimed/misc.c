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

#include "misc.h"

char hd[256];

char *time2uptime (time_t t)
{
	static char	timebuf[21] = "";
	int		sec, min, hour, day;

	sec = t % 60;
	t /= 60;
	min = t % 60;
	t /= 60;
	hour = t % 24;
	t /= 24;
	day = t;

	snprintf(timebuf, sizeof(timebuf)-1, "%d %s, %.2d:%.2d:%.2d", day, (day == 1 ? "day " : "days"), hour, min, sec); 
	timebuf[sizeof(timebuf)-1] = 0;
	return timebuf;
}

void read_homedir(void)
{
	strncpy(hd, getenv("HOME"), sizeof(hd)-1);
	hd[sizeof(hd)-1]='\0';

	if(!hd[0])
	{
		printf("Could not initialize your homedir, exiting.\n");
	    exit(-1);
	}
}

void cat(char *filename)
{
	FILE *f;
	char str[512];
	
	f=fopen(filename, "r");
	if (!f)
		return;

	fgets(str, sizeof(str), f);
	while (!feof(f))
	{
		printf("%s", str);
		fgets(str, sizeof(str), f);
	}
	fclose(f);
}
