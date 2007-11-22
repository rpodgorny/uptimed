# Makefile.am for uptimed

SUBDIRS = libuptimed src man etc

EXTRA_DIST = CREDITS INSTALL.cgi INSTALL.upgrade README.unsupported \
	sample-cgi/uprecords.conf sample-cgi/uprecords.header \
	sample-cgi/uprecords.footer uptimed.spec depcomp

install-data-local:
	$(INSTALL) -d /var/spool/uptimed
