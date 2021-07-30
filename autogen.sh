#! /bin/sh
# this is stolen from https://github.com/rakshasa/rtorrent/blob/master/autogen.sh and modified slightly

echo aclocal...
(aclocal --version) < /dev/null > /dev/null 2>&1 || {
	echo aclocal not found
	exit 1
}

aclocal -I . ${ACLOCAL_FLAGS} || exit 1

echo autoheader...
(autoheader --version) < /dev/null > /dev/null 2>&1 || {
	echo autoheader not found
	exit 1
}

autoheader || exit 1

echo -n "libtoolize... "
if ( (glibtoolize --version) < /dev/null > /dev/null 2>&1 ); then
	echo "using glibtoolize"
	glibtoolize --automake --copy --force || exit 1
elif ( (libtoolize --version) < /dev/null > /dev/null 2>&1 ) ; then
	echo "using libtoolize"
	libtoolize --automake --copy --force || exit 1
else
    echo "neither libtoolize nor glibtoolize found"
    exit 1
fi

echo automake...
(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo automake not found
	exit 1
}

automake --add-missing --copy --gnu || exit 1

echo autoconf...
(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo autoconf not found
	exit 1
}

autoconf || exit 1

echo ready to configure

exit 0
