/*
uptimed - Copyright (c) 1998-2004 Rob Kaper <rob@unixcode.org>
        - read_uptime code for BSD and Solaris platforms taken from upclient
          package by Martijn Broenland.

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

#include "milestone.h"

Milestone *milestone_list = NULL;
static Milestone *milestone_last = NULL;

Milestone *add_milestone(time_t time, char *desc)
{
	Milestone *m, *tmpm, *mprev=NULL;

	/* Allocate memory for the new entry. */	
	if ((m=malloc(sizeof(Milestone))) == NULL)
	{
		printf("error mallocing milestone struct. this is serious shit! exiting.\n");
		exit(1);
	}

	/* Copy boottime, uptime and systeminfo into memory. */
	m->time = time;
	strncpy(m->desc, desc, SYSMAX);
	m->desc[SYSMAX]='\0';

	/* Add the entry to the linked list. */
	for(tmpm=milestone_list;tmpm;tmpm=tmpm->next)
	{
		if (m->time < tmpm->time)
		{
			m->next=tmpm;
			if (tmpm==milestone_list) return milestone_list = m;
			else return mprev->next = m;
		}
		else mprev = tmpm;
	}

	m->next = NULL;
	if (milestone_last) milestone_last->next = m;
	else milestone_list = m;
	return milestone_last = m;
}

void del_milestone(Milestone *m)
{
	Milestone *tmpm=milestone_list;
	
	if (m==milestone_list)
	{
		milestone_list=m->next;
		if (!milestone_list) milestone_last = NULL;
	}
	else
	{
		for (tmpm=milestone_list; tmpm->next && m!=tmpm->next; tmpm=tmpm->next);
		if (!m->next) milestone_last = tmpm;
		tmpm->next = m->next;
	}
	free(m);
}

Milestone *find_next_milestone(time_t offset)
{
	Milestone *m;

	for (m=milestone_list;m && m->time < offset;m=m->next);

	if (m)
		return m;
	else
		return NULL;	 
}

time_t scantime(char *str) {
	char	*end;
	size_t	 len;
	time_t	 multiplier;

	/* Find the last char in string. */
	len = strlen(str);
	end = str + len - 1;

 	if (isdigit(*end))
 	{
 		/* It's a digit, so input was in seconds. */
 		multiplier = 1;
	}
	else
	{
		switch(tolower(*end))
		{
			case 's':
				/* Seconds. */
				multiplier = 1;
				break;

			case 'h':
				/* Hours */
				multiplier = 3600;
				break;

			case 'd':
				/* Days. */
				multiplier = 86400;
				break;

			case 'w':
				/* Weeks. */
				multiplier = 7 * 86400;
				break;

			case 'y':
				/* Solar years. */
				multiplier = (time_t) (365.24219 * 86400.00);
				break;

			default:
				/* Garbage. */
				multiplier = 0;
				break;
		}
		*end = '\0';		/* remove the time factor byte. */
	}
	return atol(str)*multiplier;
}
