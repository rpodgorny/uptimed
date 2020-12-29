sysconf_DATA = uptimed.conf-dist

EXTRA_DIST = uptimed.conf-dist rc.uptimed

if HAVE_SYSTEMD
systemdsystemunit_DATA = uptimed.service
endif
if HAVE_SMF
smfsitemanifest_DATA = uptimed.xml uptimed-bootid.xml
endif
