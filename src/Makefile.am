bin_PROGRAMS = uprecords
sbin_PROGRAMS = uptimed

uptimed_SOURCES = uptimed.c sd-daemon.c
uprecords_SOURCES = uprecords.c

uptimed_LDADD = @LIBOBJS@ $(top_builddir)/libuptimed/libuptimed.la
uprecords_LDADD = @LIBOBJS@ $(top_builddir)/libuptimed/libuptimed.la

AM_CPPFLAGS = -I$(top_srcdir)/libuptimed

# On Linux with glibc 2.17, sd-daemon.c support for POSIX message queues
# results in an otherwise unnecessary dependency on librt. Disable it.
AM_CPPFLAGS += -DSD_DAEMON_DISABLE_MQ

noinst_HEADERS = uprecords.h uptimed.h sd-daemon.h

EXTRA_DIST = getopt.c getopt.h

uptimed.o uprecords.o:
AM_CFLAGS = @CFLAGS@ -DFILE_CONFIG=\"$(sysconfdir)/uptimed.conf\"
