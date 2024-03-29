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

#include "../config.h"
#include "uprecords.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#define SYSWIDTH 24
#define DOWNWIDTH 20

#if defined(__ANDROID__) || defined(PLATFORM_AIX)
extern Urec *u_current;
#else
Urec	*u_current;
#endif
time_t	first, prev, tenth, second;
int		runas_cgi=0, show_max=10, show_milestone=0, layout=PRE, show_downtime=0, run_loop=0, update_interval=5;
int		sort_by=0, no_ansi=0, no_stats=0, no_current=0, wide_out=0;

int main(int argc, char *argv[])
{
	/* Read config file. */
	read_config();

	/* Check if we are being run as CGI program. */
	if (strstr(argv[0], ".cgi"))
	{
		runas_cgi=1;

		/* Print content-type header. */
		printf("Content-type: text/html\n\n");

		/* Read CGI config file. */
		read_config_cgi();
	}

	scan_args(argc, argv);

#ifdef PLATFORM_HPUX
	no_ansi=1;
#endif

	if (update_interval<1)
		update_interval=1;

	if (show_max<0)
		show_max=1;

	/* Print header file. */
	if (runas_cgi)
		cat("/etc/uprecords-cgi/uprecords.header");

	/* Read current uptime and entries from logfile. */
	u_current=add_urec(read_uptime(), readbootid(), read_sysinfo());
	read_records(u_current->btime);

	if (!runas_cgi && run_loop)
	{
		while(1)
		{
			/* Update current uptime. */
			u_current->utime=read_uptime();

			/* Current uptime might move up in the list. */
			if ((prev && prev - u_current->utime < 0) || (first && first - u_current->utime < 0))
			{
				moveup();
				first=0;
			}
			displayrecords(1);
			sleep(update_interval);
		}
	}
	else displayrecords(0);

	/* Print footer file. */
	if (runas_cgi)
		cat("/etc/uprecords-cgi/uprecords.footer");

	return 0;
}

void displayrecords(int cls)
{
	Urec *u, *uprev=NULL;
	time_t since, now, tmp, totalutime = 0, totaldtime = 0;
	float availability;
	int	i=0, currentdone=0;

	now=time(0);

	/* Open output for CGI. */
	if (runas_cgi)
	{
		if (layout==TABLE)
			printf("<table border=1>\n");
		else if (layout==LIST)
			printf("<ol>\n");
		else
			printf("<pre>\n");
	}
	/* Clear screen. */
	else if (cls && !no_ansi)
		printf(CLS);

	if (layout==PRE)
	{
		if (!show_downtime) {
			printf("   %3s%21s | %-*s %*s\n", "#", "Uptime", SYSWIDTH, "System", TIMEMAX, "Boot up");
		} else {
			printf("   %3s%21s | %*s %*s\n", "#", "Uptime", DOWNWIDTH, "Last downtime", DOWNTIMEMAXWIDTH, "Boot up");
		}
		print_line();
	}
	else if (runas_cgi && layout==TABLE)
	{
		printf("<th>%s</th>\n", "Position");
		printf("<th>%s</th>\n", "Uptime");
		if (!show_downtime) {
			printf("<th>%s</th>\n", "System");
		} else {
			printf("<th>%s</th>\n", "Last downtime");
		}
		printf("<th>%s</th>\n", "Boot up");
	}

	urec_list = sort_urec(urec_list, sort_by);

	for ( u=urec_list; u ; u=u->next )
	{
		if (++i<=show_max || show_max==0)
		{
			if (u==u_current)
			{
				if (!show_downtime) {
					print_entry(u->utime, u->sys, u->btime, "-> ", i, 1);
				} else {
					print_downtime_entry(u->utime, u->dtime, u->btime, "-> ", i, 1);
				}
				currentdone++;
				if (uprev) prev=uprev->utime;
				else
				{
 					prev=0; first=0; tenth=0; second=0;
				}
			}
			else
			{
				if (!show_downtime) {
					print_entry(u->utime, u->sys, u->btime, "   ", i, 0);
				} else {
					print_downtime_entry(u->utime, u->dtime, u->btime, "   ", i, 0);
				}
				if (i==1) first=u->utime;
 				if (i==2) second=u->utime;
 				if (i==10) tenth=u->utime;
			}
		}
		else if (u==u_current)
		{
			if (uprev) prev=uprev->utime;
			break;
		}
		uprev=u;
	}

	if (runas_cgi && layout==LIST)
	{
		printf("</ol>\n");
		printf("<ul>\n");
	}

	if (!no_current && !currentdone)
	{
		if (layout==PRE)
			print_line();

		print_entry(u_current->utime, u_current->sys, u_current->btime, "-> ", i, 1);
		if (uprev) prev=uprev->utime;
	}

	if (!no_stats)
	{
		if (layout==PRE)
			print_line();

		if (prev && prev!=first)
		{
			tmp=now + prev - u_current->utime;
			print_entry(prev - u_current->utime + 1, "at", tmp, "1up in", 0, 0);
		}
		if (tenth && prev<=tenth && prev!=first)
		{
			tmp=now + tenth - u_current->utime;
			print_entry(tenth - u_current->utime + 1, "at", tmp, "t10 in", 0, 0);
		}
		if (first)
		{
			tmp=now + first - u_current->utime;
			print_entry(first - u_current->utime + 1, "at", tmp, "no1 in", 0, 0);
		} else {
			tmp=now + second - u_current->utime;
			print_entry(u_current->utime - second - 1, "since", tmp, "NewRec", 0, 0);
		}
		if (show_milestone)
		{
			Milestone *m;

			m = find_next_milestone(u_current->utime);
			if (m!=NULL)
			{
				tmp=now + m->time - u_current->utime;
				print_entry(m->time - u_current->utime + 1, m->desc, tmp, "mst in", 0, 0);
			}
		}

		/* Printing total uptime and downtime. */
		for (u = urec_list; u; u = u->next){
			if (u->dtime == 0) {
				since = u->btime;
			}
			totaldtime += u->dtime;
			totalutime += u->utime;
		}

		print_entry(totalutime, "since", since, "up", 0, 0);
		print_entry(totaldtime, "since", since, "down", 0, 0);

		/* Printing availability. */
		availability = (float)totalutime / (float)(totalutime + totaldtime) * 100;
		print_availability(availability, since);
	}

	/* End output for CGI. */
	if (runas_cgi)
	{
		if (layout==TABLE)
			printf("</table>\n");
		else if (layout==LIST)
			printf("</ul>\n");
		else
			printf("</pre>\n");

		printf("<small><a href=\"https://github.com/rpodgorny/uptimed\">uptimed</a> by Rob Kaper (<a href=\"mailto:rob@unixcode.org\">rob@unixcode.org</a>) - currently maintained by Radek Podgorny (<a href=\"mailto:radek@podgorny.cz\">radek@podgorny.cz</a>)</small>\n");
	}
}

void read_config(void)
{
	FILE *f;
	char str[256];
	time_t milestone_time;
	char *milestone_str;

	f=fopen(FILE_CONFIG, "r");
	if (!f) return;

	fgets(str, sizeof(str), f);
	while (!feof(f))
	{
		if (!strncmp(str, "MILESTONE",  9))
		{
			char *cp = strtok(str+10, ":");
			milestone_time=scantime(cp);
			milestone_str=strtok(NULL, "\n");
			add_milestone(milestone_time, milestone_str);
		}
		fgets(str, sizeof(str), f);
	}
}

void read_config_cgi(void)
{
	FILE *f;
	char str[256];

	f=fopen("/etc/uprecords-cgi/uprecords.conf", "r");
	if (!f)
		return;

	fgets(str, sizeof(str), f);
	while(!feof(f))
	{
		if (!strncmp(str, "LAYOUT", 6))
		{
			if (!strncmp(str+7, "table", 5))
				layout=TABLE;
			else if (!strncmp(str+7, "list", 4))
				layout=LIST;
			else
				layout=PRE;
		}
		else if (!strncmp(str, "SHOW_MAX", 8))
			show_max=atoi(str+9);
		else if (!strncmp(str, "TYPE", 4))
		{
			if (!strncmp(str+5, "system", 6))
				show_downtime = 0;
			else if (!strncmp(str+5, "downtime", 8))
				show_downtime = 1;
		}
		fgets(str, sizeof(str), f);
	}
	fclose(f);
}

void print_entry(time_t utime, char *sys, time_t btime, char *ident, int pos, int hilite)
{
	char *bold = BOLD, *plain = PLAIN, *current = "";
	char *ctimec = NULL;

	if (runas_cgi)
	{
		bold = "<b>";
		plain = "</b>";

		if (hilite)
			current = " (current)";

		if (!strcmp(ident, "-> "))
			ident="-&gt; ";

		if (layout!=PRE)
		{
			if (!strcmp(ident, "1up in"))
				ident="One up in";
			else if (!strcmp(ident, "no1 in"))
				ident="Number one in";
		}
	}

	if (!hilite || no_ansi)
	{
		bold = "";
		plain = "";
	}

	switch(layout)
	{
		case TABLE:
			printf("<tr>\n");
			if (pos)
				printf("<td>%s%d%s%s</td>\n", bold, pos, plain, current);
			else
				printf("<td>%s</td>\n", ident);

			printf("<td>%s%s%s</td>\n", bold, time2uptime(utime), plain);
			printf("<td>%s%s%s</td>\n", bold, sys, plain);
			printf("<td>%s%s%s</td>\n", bold, ctime(&btime), plain);

			printf("</tr>\n");
			break;
		case LIST:
			if (pos>show_max)
				printf("<li>%s%s, %s, %s%s%s (position %d)", bold, time2uptime(utime), sys, ctime(&btime), plain, current, pos);
			else if (pos)
				printf("<li>%s%s, %s, %s%s%s", bold, time2uptime(utime), sys, ctime(&btime), plain, current);
			else
				printf("<li>%s%s %s %s %s%s", bold, ident, time2uptime(utime), sys, ctime(&btime), plain);
			break;
		default:
			if((ctimec = ctime(&btime)))
				ctimec[TIMEMAX-1] = '\0';	/* erase the ending '\n' */
			if (!wide_out && strlen(sys) > SYSWIDTH)
				sys[SYSWIDTH] = '\0';		/* truncate for 80 cols */
			if (pos)
				printf("%s%3s%3d%21s %s|%s %-*s %*s%s\n", bold, ident, pos, time2uptime(utime), plain, bold, SYSWIDTH, sys, TIMEMAX, ctimec, plain);
			else
				printf("%s%6s%21s %s|%s %-*s %*s%s\n", bold, ident, time2uptime(utime), plain, bold, SYSWIDTH, sys, TIMEMAX, ctimec, plain);
	}
}

void print_downtime_entry(time_t utime, time_t dtime, time_t btime, char *ident, int pos, int hilite)
{
	char *bold = BOLD, *plain = PLAIN, *current = "";
	char *ctimec = NULL;
	char timebuf[20] = "", dtimebuf[20] = "";

	strcpy(timebuf, time2uptime(utime));
	strcpy(dtimebuf, time2uptime(dtime));

	if (runas_cgi)
	{
		bold = "<b>";
		plain = "</b>";

		if (hilite)
			current = " (current)";

		if (!strcmp(ident, "-> "))
			ident="-&gt; ";
	}

	if (!hilite || no_ansi)
	{
		bold = "";
		plain = "";
	}

	switch(layout)
	{
		case TABLE:
			printf("<tr>\n");
			if (pos)
				printf("<td>%s%d%s%s</td>\n", bold, pos, plain, current);
			else
				printf("<td>%s</td>\n", ident);

			printf("<td>%s%s%s</td>\n", bold, timebuf, plain);
			printf("<td>%s%s%s</td>\n", bold, dtimebuf, plain);
			printf("<td>%s%s%s</td>\n", bold, ctime(&btime), plain);

			printf("</tr>\n");
			break;
		case LIST:
			if (pos>show_max)
				printf("<li>%s%s, %s, %s%s%s (position %d)", bold, timebuf, dtimebuf, ctime(&btime), plain, current, pos);
			else if (pos)
				printf("<li>%s%s, %s, %s%s%s", bold, timebuf, dtimebuf, ctime(&btime), plain, current);
			else
				printf("<li>%s%s %s at %s%s", bold, ident, timebuf, ctime(&btime), plain);
			break;
		default:
			if((ctimec = ctime(&btime)))
				ctimec[TIMEMAX-1] = '\0';	/* erase the ending '\n' */
			if (pos) {
				printf("%s%3s%3d%21s %s|%s %*s %*s%s\n", bold, ident, pos, timebuf, plain, bold, DOWNWIDTH, dtimebuf, DOWNTIMEMAXWIDTH, ctimec, plain);
			} else {
				printf("%s%6s%21s %s|%s %*s %*s%s\n", bold, ident, timebuf, plain, bold, DOWNWIDTH, dtimebuf, DOWNTIMEMAXWIDTH, ctimec, plain);
			}
	}
}

void print_availability(float percent, time_t since)
{
	char *ctimec = NULL, *msg = "%up";

	if (runas_cgi) {
		if (layout!=PRE) {
			msg = "Availability (%)";
		}
	}

	switch(layout)
	{
		case TABLE:
			printf("<tr>\n");
			printf("<td>%s</td>\n", msg);
			printf("<td>%.3f</td>\n", percent);
			printf("<td>since</td>\n");
			printf("<td>%s</td>\n", ctime(&since));

			printf("</tr>\n");
			break;
		case LIST:
				printf("<li>%s %.3f since %s", msg, percent, ctime(&since));
			break;
		default:
			if (ctimec = ctime(&since)) {
				ctimec[TIMEMAX-1] = '\0';	/* erase the ending '\n' */
			}
			printf("%6s %20.3f | %-*s %*s\n", msg, percent, SYSWIDTH, "since", TIMEMAX, ctimec);
	}
}

void print_line(void)
{
	printf("----------------------------+---------------------------------------------------\n");
}

void scan_args(int argc, char *argv[])
{
	int i;

	while((i = getopt(argc, argv, "i:m:?acbdBkKfsMwv")) != EOF)
	{
		switch(i)
		{
				case '?':
#ifdef __ANDROID__
						fputc('\n', stderr);
#endif
						print_help(argv);
						break;
				case 'a':
						no_ansi++;
						break;
				case 'b':
						sort_by = 1;
						no_stats++;
						break;
				case 'B':
						sort_by = -1;
						no_stats++;
						break;
				case 'k':
						sort_by = 2;
						no_stats++;
						break;
				case 'K':
						sort_by = -2;
						no_stats++;
						break;
				case 'd':
						show_downtime++;
						break;
				case 'c':
						no_current++;
						break;
				case 'f':
						run_loop++;
						break;
				case 's':
						no_stats++;
						break;
				case 'w':
						wide_out++;
						break;
				case 'v':
						print_version();
						break;
				case 'M':
					show_milestone++;
					break;
				case 'i':
					if (isdigit(*optarg))
						update_interval=atoi(optarg);
					else
					{
						printf("%s: option requires argument -- %c\n", argv[0], i);
						print_usage(argv);
					}
					run_loop++;
					break;
				case 'm':
					if (isdigit(*optarg))
						show_max=atoi(optarg);
					else
					{
						printf("%s: option requires argument -- %c\n", argv[0], i);
						print_usage(argv);
					}
					break;
		}
	}
}

void print_usage(char *argv[])
{
	printf("usage: %s [-?acdfMswv] [-i interval] [-m count]\n", argv[0]);
	exit(1);
}

void print_help(char *argv[])
{
	printf("usage: %s [OPTION]...\n\n", argv[0]);
	printf("  -?             this help\n");
	printf("  -a             do not print ansi codes\n");
	printf("  -b             sort by boottime\n");
	printf("  -B             reverse sort by boottime\n");
	printf("  -k             sort by sysinfo\n");
	printf("  -K             reverse sort by sysinfo\n");
	printf("  -d             print downtime seen before every uptimes instead of system\n");
	printf("  -c             do not show current entry if not in top entries\n");
	printf("  -f             run continuously in a loop\n");
	printf("  -s             do not print extra statistics\n");
	printf("  -w             wide output (more than 80 cols per line)\n");
	printf("  -i INTERVAL    use INTERVAL seconds for loop instead of 5, implies -f\n");
	printf("  -m COUNT       show a maximum of top COUNT entries instead of 10\n");
	printf("  -M             show next milestone\n");
	printf("  -v             version information\n");
	exit(0);
}

void print_version(void)
{
	printf("uprecords " VERSION " by Rob Kaper <rob@unixcode.org>\n");
	printf("enhanced and maintained by Radek Podgorny <radek@podgorny.cz>\n");
	exit(0);
}
