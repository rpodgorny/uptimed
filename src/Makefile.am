bin_PROGRAMS = uprecords
sbin_PROGRAMS = uptimed

uptimed_SOURCES = uptimed.c
uprecords_SOURCES = uprecords.c

uptimed_LDADD = @LIBOBJS@ $(top_builddir)/libuptimed/libuptimed.la
uprecords_LDADD = @LIBOBJS@ $(top_builddir)/libuptimed/libuptimed.la

INCLUDES = -I$(top_srcdir)/libuptimed

noinst_HEADERS = uprecords.h uptimed.h

EXTRA_DIST = getopt.c getopt.h
  
uptimed.o uprecords.o:
CFLAGS = @CFLAGS@ -DFILE_CONFIG=\"$(sysconfdir)/uptimed.conf\"
