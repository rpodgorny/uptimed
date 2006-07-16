%define name	uptimed
%define ver	0.3.6
%define rel	0

Summary		: A daemon to record and keep track of system uptimes
Name		: %{name}
Version		: %{ver}
Release		: %{rel}
Copyright	: GPL
Group		: System Environment/Daemons
Source		: http://prdownloads.sourceforge.net/%{name}/%{name}-%{ver}.tar.gz
BuildRoot	: /var/tmp/%{name}-buildroot
PreReq		: chkconfig >= 0.9

%description
Uptimed is an uptime record daemon keeping track of the highest 
uptimes the system ever had. Instead of using a pid file to 
keep sessions apart from each other, it uses the system boot 
time. 

Uptimed has the ability to inform you of records and milestones 
though syslog and e-mail, and comes with a console front end to 
parse the records, which can also easily be used to show your 
records on your Web page

%prep
%setup -q

%build
%configure
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf "$RPM_BUILD_ROOT"
make DESTDIR="$RPM_BUILD_ROOT" install
install -m 755 -d $RPM_BUILD_ROOT/%_docdir/%{name}-%{version}/sample-cgi
install -m 644 sample-cgi/uprecords.* $RPM_BUILD_ROOT/%_docdir/%{name}-%{version}/sample-cgi
install -m 755 -d $RPM_BUILD_ROOT/etc/rc.d/init.d
install -m 755 etc/rc.uptimed $RPM_BUILD_ROOT/etc/rc.d/init.d/uptimed
mv $RPM_BUILD_ROOT/etc/uptimed.conf-dist $RPM_BUILD_ROOT/etc/uptimed.conf

%post
/sbin/ldconfig
install -m 755 -d /var/spool/uptimed
/sbin/chkconfig --add uptimed

if [ -f /etc/rc.d/rc.sysinit ]
then
	if [ ! `grep "/sbin/service uptimed createbootid" /etc/rc.d/rc.sysinit > /dev/null` ]
	then
		echo "/sbin/service uptimed createbootid" >> /etc/rc.d/rc.sysinit
	fi
else
echo
echo "Please add the following line to your rc.sysinit script"
echo "/sbin/service uptimed createbootid"
echo
fi

%postun
/sbin/ldconfig

%preun
/sbin/chkconfig --del uptimed

if [ -f /etc/rc.d/rc.sysinit ]
then
	grep -v "/sbin/service uptimed createbootid" /etc/rc.d/rc.sysinit > /tmp/rc.sysinit.$$ && mv /tmp/rc.sysinit.$$ /etc/rc.d/rc.sysinit
	chmod 755 /etc/rc.d/rc.sysinit
else
echo
echo "Please remove the following line to your rc.sysinit script"
echo "/sbin/service uptimed createbootid"
echo
fi

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%doc AUTHORS COPYING CREDITS ChangeLog INSTALL INSTALL.cgi INSTALL.upgrade README README.unsupported TODO sample-cgi/
%config(noreplace) /etc/uptimed.conf
%config /etc/rc.d/init.d/uptimed
%{_sbindir}/uptimed
%{_bindir}/uprecords
%{_mandir}/*/*
%{_libdir}/libuptimed.*

%changelog
* Tue May 14 2002 Brett Pemberton <generica@email.com>
- Reset release to 0 for uptimed-2.0
- Change source location

* Sat Mar 16 2002 Brett Pemberton <generica@email.com>
- Add /etc/rc.d/init.d/uptimed
- Use chkconfig

* Wed Mar 13 2002 Brett Pemberton <generica@email.com>
- Automate rc.{sysinit,local} add/remove
- Warn if rc.{sysinit,local} not found

* Fri Dec 21 2001 Brett Pemberton <generica@email.com>
- Handle sample-cgi dir properly
- Install uptimed.conf properly
- Install /var/spool/uptimed
- Warn user to finish configuring

* Thu Dec 20 2001 Brett Pemberton <generica@email.com>
- Initial spec-file

