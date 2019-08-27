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

#include "../config.h"
#include "urec.h"

#ifdef __ANDROID__
Urec *u_current;
#endif
Urec *urec_list = NULL;
static Urec *urec_last = NULL;

Urec *add_urec(time_t utime, time_t btime, char *sys) {
	Urec *u, *tmpu, *uprev = NULL;

	/* Allocate memory for the new entry. */
	if ((u=malloc(sizeof(Urec))) == NULL) {
		printf("error mallocing urec struct. this is serious shit! exiting.\n");
		exit(1);
	}

	/* Copy boottime, uptime and systeminfo into memory. */
	u->utime = utime;
	u->btime = btime;
	strncpy(u->sys, sys, SYSMAX);
	u->sys[SYSMAX]='\0';

	/* Add the entry to the linked list. */
	for(tmpu = urec_list; tmpu; tmpu = tmpu->next) {
		if (u->utime > tmpu->utime) {
			u->next=tmpu;
			if (tmpu == urec_list) return urec_list = u;
			return uprev->next = u;
		} else {
			uprev = tmpu;
		}
	}

	u->next = NULL;
	if (urec_last) {
		urec_last->next = u;
	} else {
		urec_list = u;
	}
	return urec_last = u;
}

void del_urec(Urec *u) {
	Urec *tmpu = urec_list;
	
	if (u == urec_list) {
		urec_list=u->next;
		if (!urec_list) urec_last = NULL;
	} else {
		for (tmpu = urec_list; tmpu->next && u != tmpu->next; tmpu = tmpu->next);
		if (!u->next) urec_last = tmpu;
		tmpu->next = u->next;
	}
	free(u);
}

void moveup(void) {
	/* Delete current session from the list. */
	del_urec(u_current);

	/* Re-add it. (it should be urec_list now) */
	u_current=add_urec(read_uptime(), readbootid(), read_sysinfo());
}

char *read_sysinfo(void) {
	struct utsname temp_uname;
	static char sys[SYSMAX+1];

	/* What kernel are we running at the moment? */
	if (!uname(&temp_uname)) {
		/* Name and number.. */
		snprintf(sys, SYSMAX, "%s %s", temp_uname.sysname, temp_uname.release);
		sys[SYSMAX]='\0';
		return sys;
	} else {
#ifdef PLATFORM_LINUX
		return "Linux";
#endif
#ifdef PLATFORM_BSD
		return "BSD";
#endif
#ifdef PLATFORM_SOLARIS
		return "Solaris";
#endif
#ifdef PLATFORM_HPUX
		return "HP/UX";
#endif
#ifdef PLATFORM_GNU
		return "GNU";
#endif
#ifdef PLATFORM_UNKNOWN
		return "unknown";
#endif
	}
}

#ifdef PLATFORM_LINUX
time_t read_uptime(void) {
	struct timespec ts;
	FILE *f;
	double upseconds = 0;
	struct sysinfo	si;


	if (clock_gettime(CLOCK_BOOTTIME, &ts) == 0)
		return ts.tv_sec;


	/* clock_gettime() failed */
	f=fopen("/proc/uptime", "r");
	if (f > 0) {
		if (fscanf(f, "%lf", &upseconds) > 0) {
			fclose(f);
			return((time_t)upseconds);
		}
		fclose(f);
	}


	/* reading of /proc/uptime failed */

	/* Until jiffies is declared to something different than unsigned long
	 * in the kernel sources, this value will probably on all 32b platforms
	 * wrap past approx. 497 days. This also applies to reading this value
	 * through /proc/uptime.
	 */
	if (sysinfo(&si) != 0) {
		printf ("uptimed: error getting uptime!\n");
		exit(-1);
	}
	return((time_t)si.uptime);
}
#endif

#ifdef PLATFORM_BSD
time_t read_uptime(void) {
	time_t now, up;
	struct timeval boottime;
	int mib[2];
	size_t size;

	(void)time(&now);
	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof (boottime);
	if (sysctl (mib, 2, &boottime, &size, NULL, 0) != -1 && boottime.tv_sec!= 0) {
		up = now - boottime.tv_sec;
	}
	
	return up;
}
#endif

#ifdef PLATFORM_SOLARIS
time_t read_uptime(void) {
	int fd;
	struct utmpx ut;
	int found=0;

	fd = open(UTMPX_FILE, O_RDONLY);
	if (fd >= 0) {
		while (!found) {
			if (read(fd, &ut, sizeof(ut)) < 0) {
				found = -1;
			} else if (ut.ut_type==BOOT_TIME) {
				found = 1;
			}
		}
		close(fd);
	}

	if (found == 1) return time(0) - ut.ut_tv.tv_sec;

	return 0;
}
#endif

#ifdef PLATFORM_HPUX
time_t read_uptime(void) {
	struct pst_static _pst_static;
	pstat_getstatic( &_pst_static, sizeof(_pst_static), (size_t)1, 0);
	return (time_t)(time(0) - _pst_static.boot_time);
}
#endif

#if defined(PLATFORM_UNKNOWN) || defined(PLATFORM_GNU)
time_t read_uptime(void) {
/*
 * This is a quick and inaccurate hack calculating the uptime from the
 * current time and the timestamp made at boottime.
 *
 * This is inaccurate because:
 * 1) the boottime timestamp can be extremely delayed due to fscking etc.
 * 2) when the system time changes (for example timezone changes) the
 *    boottime timestamp does not, creating even more inaccuracy.
 */
 	if (u_current) return time(0) - u_current->btime;

	return time(0) - readbootid();
}
#endif

void calculate_downtime(void) {
	Urec *u, *sorted_list = sort_urec(urec_list, -1);

	for (u = sorted_list; u; u = u->next) {
		if (u->next) {
			 u->dtime = u->btime - (u->next->btime + u->next->utime);
		} else { /* First uptime recorded... No prior downtime data. */
			u->dtime = 0;
		}
	}

	urec_list = sort_urec(sorted_list, 0);
}


void read_records(time_t current) {
	FILE *f;
	char str[256];
	time_t utime, btime;
	long l_utime, l_btime;
	char buf[256], sys[SYSMAX+1];
	struct stat filestat, filestatold;
	int useold = 0;
	
	if (stat(FILE_RECORDS, &filestat))
		useold = 1;
	if (stat(FILE_RECORDS".old", &filestatold))
		useold = -1;

	/* assume that backupdb larger than normal db means normal is corrupted */
	if (!useold && (filestat.st_size < filestatold.st_size))
		useold = 1;

dbtry:
	switch (useold) {
		case 0:
			f = fopen(FILE_RECORDS, "r");
			break;
		case 1:
			f = fopen(FILE_RECORDS".old", "r");
			printf("uptimed: reading from backup database %s.old\n", FILE_RECORDS);
			break;
		default:
			/* this should probably terminate uptimed somehow */
			printf("uptimed: no useable database found.\n");
			return;
	}
			
	if (!f) {
		printf("uptimed: error opening database for reading.\n");
		return;
	}
	
	fgets(str, sizeof(str), f);
	while (!feof(f)) {
		/* Check for validity of input string. */
		if (sscanf(str, "%ld:%ld:%[^]\n]", &l_utime, &l_btime, buf) != 3) {
			/* database is corrupted */
			fclose(f);
			useold++;
			goto dbtry;
		} else {
			utime = (time_t)l_utime;
			btime = (time_t)l_btime;

			strncpy(sys, buf, SYSMAX);
			sys[SYSMAX]='\0';
			if (utime > 0 && btime != current) add_urec(utime, btime, sys);
		}
		fgets(str, sizeof(str), f);
	}
	fclose(f);
	
	calculate_downtime();
}

void save_records(int max, time_t log_threshold) {
	FILE *f;
	Urec *u;
	int i = 0;
	
	f = fopen(FILE_RECORDS".tmp", "w");
	if (!f) {
		printf("uptimed: cannot write to %s\n", FILE_RECORDS);
		return;
	}

	for (u = urec_list; u; u = u->next) {
		/* Ignore everything below the threshold */
		if (u->utime >= log_threshold) {
			fprintf(f, "%lu:%lu:%s\n", (unsigned long)u->utime, (unsigned long)u->btime, u->sys);
			/* Stop processing when we've logged the max number specified. */
			if ((max > 0) && (++i >= max)) break;
		}
	}
	fclose(f);
	rename(FILE_RECORDS, FILE_RECORDS".old");
	rename(FILE_RECORDS".tmp", FILE_RECORDS);
}

#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_GNU)
int createbootid(void) {
	/* these platforms doesn't need to create a bootid file.
	 * readbootid() fetches it directly from the system every time.
	 */
	return 0;
}
#endif


#ifdef PLATFORM_SOLARIS
int createbootid(void) {
	FILE *f;
	int fd;
	struct utmpx ut;
	int found = 0;
	time_t bootid = 0;

	fd = open (UTMPX_FILE, O_RDONLY);
	if (fd >= 0) {
		while(!found) {
			if (read(fd, &ut, sizeof(ut)) < 0) {
				found = -1;
			} else if (ut.ut_type==BOOT_TIME) {
				found = 1;
			}
		}
		close(fd);
	}

	if (found == 1) bootid = ut.ut_tv.tv_sec;

	f = fopen(FILE_BOOTID, "w");
	if (!f) {
		printf("Error writing bootid file, exiting!\n");  exit(-1);
	} else {
		fprintf(f, "%ld\n", bootid);
		fclose(f);
	}
	return 0;
}
#endif

#ifdef PLATFORM_HPUX
int createbootid(void) {
	FILE *f;
	struct pst_static _pst_static;
	
	pstat_getstatic(&_pst_static, sizeof(_pst_static), (size_t)1, 0);
	f=fopen(FILE_BOOTID, "w");
	if (!f) {
		printf("Error writing bootid file, exiting!\n");  exit(-1);
	} else {
		fprintf(f, "%ld\n", _pst_static.boot_time);
		fclose(f);
	}
	return 0;
}
#endif

#ifdef PLATFORM_UNKNOWN
int createbootid(void) {
	FILE *f;
	time_t bootid=0;
	
	bootid=time(0);
	
	f = fopen(FILE_BOOTID, "w");
	if (!f) {
		printf("Error writing bootid file, exiting!\n");  exit(-1);
	} else {
		fprintf(f, "%ld\n", bootid);
		fclose(f);
	}
	return 0;
}
#endif

time_t readbootid(void) {
#ifdef PLATFORM_BSD
	time_t bootid = 0;
	struct timeval boottime;
	int mib[2];
	size_t size;

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof (boottime);
	if (sysctl (mib, 2, &boottime, &size, NULL, 0) != -1 && boottime.tv_sec != 0) {
		bootid = boottime.tv_sec;
	}

	return bootid;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_GNU)
	FILE *f;
	char str[256];
	time_t bootid = 0;

	f=fopen("/proc/stat", "r");
	if (!f) {
		printf ("Error opening /proc/stat file. Can not determine bootid, exiting!\n");
		exit(-1);
	} else {
		fgets(str, sizeof(str), f);
		while (!feof(f)) {
			if (strstr(str, "btime")) {
				bootid=atoi(str+6);
				break;
			}
			fgets(str, sizeof(str), f);
		}
		fclose(f);
	}
	if (bootid == 0) {
		printf ("Parsing btime from /proc/stat failed. Can not determine bootid, exiting!\n");
		exit(-1);
	}
	return bootid;
#else
	FILE *f;
	char str[256];

	f=fopen(FILE_BOOTID, "r");
	if (!f) {
		printf("Error reading boot id from file, exiting!\n\nYou probably forgot to create a bootid with with the -b option.\nYou really want the system to do this on bootup, read the INSTALL file!\n");
		exit(-1);
	}
	fgets(str, sizeof(str), f);
	fclose(f);
	return atoi(str);
#endif
}

int compare_urecs(Urec *a, Urec *b, int sort_by) {
	if (sort_by == 0) {
		return b->utime - a->utime;
	} else if (sort_by == 1) {
		return a->btime - b->btime;
	} else if (sort_by == -1) {
		return b->btime - a->btime;
	} else if (sort_by == 2) {
		return strcmp(a->sys, b->sys);
	} else if (sort_by == -2) {
		return strcmp(b->sys, a->sys);
	}

	return 0;
}

Urec* sort_urec(Urec* list, int sort_by) {
/*
 * sort_by:
 *      0: Nothing
 *      1: Boottime
 *     -1: Reverse Boottime
 *      2: Sysinfo
 *     -2: Reverse Sysinfo
 */

	Urec *p, *q, *e, *tail;
	int insize, nmerges, psize, qsize, i;

	insize = 1;

	for (;;) {
		p = list;
		list = NULL;
		tail = NULL;

		nmerges = 0;  /* count number of merges we do in this pass */

		while (p) {
			nmerges++;  /* there exists a merge to be done */
			/* step `insize' places along from p */
			q = p;
			psize = 0;
			for (i = 0; i < insize; i++) {
				psize++;
				q = q->next;
				if (!q) break;
			}

			/* if q hasn't fallen off end, we have two lists to merge */
			qsize = insize;

			/* now we have two lists; merge them */
			while (psize > 0 || (qsize > 0 && q)) {
				/* decide whether next element of merge comes from p or q */
				if (psize == 0) {
					/* p is empty; e must come from q. */
					e = q; q = q->next; qsize--;
				} else if (qsize == 0 || !q) {
					/* q is empty; e must come from p. */
					e = p; p = p->next; psize--;
				} else if (compare_urecs(p,q,sort_by) <= 0) {
					/* First element of p is lower (or same);
					 * e must come from p. */
					e = p; p = p->next; psize--;
				} else {
					/* First element of q is lower; e must come from q. */
					e = q; q = q->next; qsize--;
				}

				/* add the next element to the merged list */
				if (tail) {
					tail->next = e;
				} else {
					list = e;
				}
				tail = e;
			}

			/* now p has stepped `insize' places along, and q has too */
			p = q;
		}
		tail->next = NULL;

		/* If we have done only one merge, we're finished. */
		if (nmerges <= 1) return list; /* allow for nmerges==0, the empty list case */

		/* Otherwise repeat, merging lists twice the size */
		insize *= 2;
	}
}
