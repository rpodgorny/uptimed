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
#include "uptimed.h"

#ifdef PLATFORM_HPUX
#define	SYSLOG_PREFIX	"uptimed: "
#else
#define SYSLOG_PREFIX	""
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

Urec *u_current = NULL;
int our_pos=0;
int update_interval=60, create_bootid=0, send_email=0, foreground=0;
time_t log_min_uptime = 0, mail_min_uptime = 0;
unsigned int log_max_entries = 50, mail_min_position = 10;
char email[EMAIL+1], sendmail[EMAIL+1], pidfile[EMAIL+1];
       
int main(int argc, char *argv[])
{
	Milestone *next_milestone = NULL, *cheer_milestone = NULL;
	time_t prev=0;
	int cheer_pos=0;
	void handler(int);

	/* Properly initialize these to an empty string */
	email[0] = sendmail[0] = pidfile[0] = '\0';

	/* Read configuration settings. */
	read_config();

	/* Scan arguments. */
	scan_args(argc, argv);
 
 	if(!pidfile[0])
 	{
 		strncpy(pidfile, "/var/run/uptimed", EMAIL);
 		pidfile[EMAIL] = '\0';
 	}
 
 	{
 		FILE *pidf;
 		pid_t pid = 0;
 
 		pidf = fopen(pidfile, "r");
 		if(pidf) {
 			/* The pidfile exists. Now see if the process is
 			 * still running */
 			if(!fscanf(pidf, "%d", &pid))
 				pid = 0;
 			fclose(pidf);
 			if(pid && !kill(pid, 0)) {
 				printf("uptimed is already running.\n");
 				exit(1);
 			}
 		}
 	}		

#ifndef PLATFORM_BSD
	/* Create bootid and exit. Should be done once on startup. */
	if (create_bootid)
	{
		createbootid();
		syslog(LOG_INFO, SYSLOG_PREFIX "created bootid: %d", readbootid());
		exit(0);
	}
#endif

	if (update_interval<1)
		update_interval=1;

	if (log_max_entries < 0)
		log_max_entries = 1;

	/* Let's start with adding a nice entry for the current session. */
	u_current=add_urec(read_uptime(),readbootid(),read_sysinfo());

	/* How about adding the existing records as well? */
	read_records(u_current->btime);

	/* Initialize the first milestone to look for. */
	next_milestone = find_next_milestone(u_current->utime);

 	/* Add signal handler for hangups/terminators. */
 #ifndef PLATFORM_HPUX
 	signal(SIGHUP,handler);
 #endif
 	signal(SIGTERM,handler);
 
	/* Now run in the background. */
	if (!foreground) bg();

	{
		FILE *pidf;

		pidf = fopen(pidfile, "w");
		if(pidf)
		{
			fprintf(pidf, "%d", getpid());
			fclose(pidf);
		}
		else
		{
			pidfile[0] = '\0';
		}
	}

	/* The main loop. */
	while (1)
	{
		/* Update the uptime value of the current session. */
		u_current->utime=read_uptime();

		/* Find out if we're breaking existing records. */
		if (u_current!=urec_list)
		{
			if (!prev) prev=get_prev();

			/* Current uptime might break the old record. */
			if (u_current->utime > urec_list->utime)
			{
				moveup(); /* We should be urec_list ourselves now. */
				cheer_pos=1; /* Don't forget to cheer. */
			}
			/* Current uptime might move up in the list. */
			else if (prev && u_current->utime > prev)
			{
				moveup();
				prev=get_prev(); /* We will have to re-init the prev entry. */
				cheer_pos=our_pos; /* Don't forget to cheer. */
			}
		}

		/* Find out if we are breaking any milestones. */
		if (next_milestone && u_current->utime > next_milestone->time)
		{
			cheer_milestone = next_milestone;
			next_milestone = find_next_milestone(u_current->utime);
		}

		/* Celebrate good times! */		 
		if (cheer_pos || cheer_milestone)
		{
			cheer(cheer_milestone, cheer_pos);
			cheer_pos = 0;
			cheer_milestone = NULL;
		}
		
		/* Save all the records. */
		save_records(log_max_entries, log_min_uptime);

		/* Save valueable CPU cycles. */
		sleep(update_interval);
	}
}

void bg(void)
{
	int i;
	/* Simple fork to run proces in the background. */
	switch(fork())
	{
		case 0:
			break;
		case -1:
			perror("fork failed"); exit(1);
		default:
			exit(0);
	}

	if (-1==setsid()) {
		perror("setsid failed"); exit(1);
	}

	/* Close probably all file descriptors */
	for (i = 0; i<NOFILE; i++)
		close(i);

	/* Be nice to umount */
	chdir("/");
}

void cheer(Milestone *milestone, int position)
{
	/* This function will do various sorts of recordbreaking celebrating. */
	switch(position)
	{
		case 0:
			break;
		case 1:
			syslog(LOG_INFO, SYSLOG_PREFIX "new uptime record: %s", time2uptime(u_current->utime));
			break;
		default:
			syslog(LOG_INFO, SYSLOG_PREFIX "moving up to position %d: %s", position, time2uptime(u_current->utime));
			break;
	}
	
	if (milestone!=NULL && milestone->time>0)
		syslog(LOG_INFO, SYSLOG_PREFIX "milestone: %s (%s)", time2uptime(milestone->time), milestone->desc);

	/* Send email if it's requested. */
	if ((u_current->utime > mail_min_uptime) && (!position || (position <= mail_min_position)) && (send_email==1 || (send_email==2 && milestone) || (send_email==3 && position)) && email[0])
		mail(milestone, position);
}

time_t get_prev(void)
{
	Urec *uprev=NULL, *u;

	our_pos=0;
	for (u=urec_list;u;u=u->next)
	{
		our_pos++;
		if (u==u_current)
			if (uprev)
				return uprev->utime;
		uprev=u;
	}
	return 0;
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
		if (!strncmp(str, "UPDATE_INTERVAL",  15))
			update_interval=atoi(str+16);
		else if (!strncmp(str, "MAX_ENTRIES",  11))
			log_max_entries = atoi(str+12); /* MAX_ENTRIES is deprecated now, keep for backward compat */
		else if (!strncmp(str, "LOG_MAXIMUM_ENTRIES",  19))
			log_max_entries = atoi(str+20);
		else if (!strncmp(str, "EMAIL",  5) && strstr(str+6, "@"))
		{
			strncpy(email, strtok(str+6, "\n"), EMAIL);
			email[EMAIL]='\0';
		}
		else if (!strncmp(str, "SEND_EMAIL",  10))
			send_email=atoi(str+11);
		else if (!strncmp(str, "SENDMAIL",  8))
		{
			strncpy(sendmail, strtok(str+9, "\n"), EMAIL);
			sendmail[EMAIL]='\0';
		}
		else if (!strncmp(str, "MILESTONE",  9))
		{
			char *cp = strtok(str+10, ":");
			milestone_time = scantime(cp);
			milestone_str = strtok(NULL, "\n");
			add_milestone(milestone_time, milestone_str);
		}
		else if (!strncmp(str, "LOG_MINIMUM_UPTIME",  18))
			log_min_uptime = scantime(strtok(str+19, "\n\0"));
		else if (!strncmp(str, "MAIL_MINIMUM_UPTIME",  19))
			mail_min_uptime = scantime(strtok(str+20, "\n\0"));
		else if (!strncmp(str, "MAIL_MINIMUM_POSITION",  21))
			mail_min_position = scantime(strtok(str+22, "\n\0"));
		else if (!strncmp(str, "PIDFILE",  7))
		{
			strncpy(pidfile, strtok(str+8, "\n"), EMAIL);
			pidfile[EMAIL]='\0';
		}

		fgets(str, sizeof(str), f);
	}
	fclose(f);
}

/*
Handler does not care about sorting the list or cheering, there's no time
for all that when we've received a SIGHUP or SIGTERM.

When uptimed is restarted the list will be sorted automatically and we'll
try not to shed any tears over any missing syslog messages that might have
been printed at the end of the interval.
*/
void handler(int s)
{
	/* Update the uptime value of the current session. */
	u_current->utime=read_uptime();

	/* Save all the records. */
	save_records(log_max_entries, log_min_uptime);

	/* Exit gracefully. */
	if(pidfile[0])
		unlink(pidfile);
	exit(0);
}

void scan_args(int argc, char *argv[])
{
	int index;

	while((index = getopt(argc, argv, "e:i:m:p:t:?bfv")) != EOF)
	{
		switch(index)
		{
			case '?':
				print_help(argv);
				break;
			case 'v':
				print_version();
				break;
			case 'f':
				foreground = 1;
				break;
			case 'e':
				strcpy(email, optarg);
				send_email = 1;
				break;
			case 'p':
				strcpy(pidfile, optarg);
				break;
			case 'b':
				create_bootid++;
				break;
			case 'i':
				if(isdigit(*optarg))
					update_interval=atoi(optarg);
				else
				{
					printf("%s: option requires argument -- %c\n", argv[0], index);
					print_usage(argv);
				}
				break;
			case 'm':
				if(isdigit(*optarg))
					log_max_entries = atoi(optarg);
				else
				{
					printf("%s: option requires argument -- %c\n", argv[0], index);
					print_usage(argv);
				}
				break;
			case 't':
				if(isdigit(*optarg))
					log_min_uptime = scantime(optarg);
				else
				{
					printf("%s: option requires argument -- %c\n", argv[0], index);
					print_usage(argv);
				}
				break;
		}
	}
}

void print_usage(char *argv[])
{
	printf("usage: %s [-?bv] [-e email] [-i interval] [-m count]\n", argv[0]);
	printf("               [-p pidfile] [-t log threshold]\n");
	exit(1);
}

void print_help(char *argv[])
{
	printf("usage: %s [OPTION]...\n", argv[0]);
	printf("commandline options override settings from configuration file\n\n");
	printf("  -?             this help\n");
	printf("  -b             create bootid and exit [ignored on FreeBSD]\n");
	printf("  -f             run in foreground [don't fork]\n");
	printf("  -e EMAIL       send mail to EMAIL at milestones/records\n");
	printf("  -i INTERVAL    use INTERVAL seconds for loop\n");
	printf("  -m COUNT       log a maximum of COUNT entries\n");
	printf("  -t TIMESPEC    minimum uptime to be considered a record\n");
	printf("  -p FILE        write PID to FILE\n");
	printf("  -v             version information\n");
	exit(0);
}

void print_version(void)
{
	printf("uptimed " VERSION " by Rob Kaper <rob@unixcode.org>\n");
	printf("enhanced and maintained by Radek Podgorny <radek@podgorny.cz>\n");
	exit(0);
}

/* Ugly mail hack, assumes sendmail compatible mail agent. */
void mail(Milestone *milestone, int position)
{
	char buf[256], str[2048], cmd[2048], hostname[256];

	gethostname(hostname, sizeof(hostname)-1);
	hostname[sizeof(hostname)-1] = 0;

	snprintf(str, sizeof(str)-1, "To: %s\nSubject: Congratulations (Uptimed@%s)\n\n", email, hostname);
	str[sizeof(str)-1] = 0;

	snprintf(buf, sizeof(buf)-1, "Uptimed noticed an uptime event!\n\n");
	buf[sizeof(buf)-1] = 0;
	strncat(str, buf, sizeof(str)-1);
	str[sizeof(str)-1] = 0;

	if (milestone!=NULL && milestone->time>0)
	{
		snprintf(buf, sizeof(buf)-1, "The uptime of %s has reached a milestone:\n\t%s (%s)\n", hostname, time2uptime(milestone->time), milestone->desc);
		buf[sizeof(buf)-1] = 0;
		strncat(str, buf, sizeof(str)-1);
		str[sizeof(str)-1] = 0;
	}

	if (position>0)
	{
		if (position==1)
			snprintf(buf, sizeof(buf)-1, "The uptime is a new record for %s:\n\t%s\n", hostname, time2uptime(u_current->utime));
		else
			snprintf(buf, sizeof(buf)-1, "The uptime of %s moved up to position %d:\n\t%s\n", hostname, position, time2uptime(u_current->utime));
		buf[sizeof(buf)-1] = 0;
		strncat(str, buf, sizeof(str)-1);
		str[sizeof(str)-1] = 0;
	}

	strncat(str, "\nCongratulations!\n\nUptimed author,\nRob Kaper <rob@unixcode.org>\n-- \nThis message was automatically generated by Uptimed.\nUptimed e-mail notifications can be configured from the uptimed.conf file.\n", sizeof(str)-1);
	str[sizeof(str)-1] = 0;
	
	snprintf(cmd, sizeof(cmd)-1, "printf \"%s\" | %s", str, sendmail);	
	cmd[sizeof(cmd)-1] = 0;
	system(cmd);
}
