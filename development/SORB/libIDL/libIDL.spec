%define version 0.5.8
%define release SNAP

Summary:   IDL parsing library
Name:      libIDL
Version:   %{version}
Release:   %{release}
Source:    libIDL-%PACKAGE_VERSION.tar.gz
Vendor:    Andrew T. Veliath <andrewtv@usa.net>
URL:       http://www.rpi.edu/~veliaa/libIDL
Copyright: GPL
Group:     Libraries
Prereq:    /sbin/install-info
Requires:  gcc, glib >= 1.1.4
Prefix:    /usr
Docdir:    %prefix/doc
BuildRoot: /var/tmp/libIDL-%{version}-root

%changelog
* Fri Nov 28 1998 Andrew T. Veliath <andrewtv@usa.net>
- Initial version

%description
libIDL is a small library for creating parse trees of CORBA v2.2
compliant Interface Definition Language (IDL) files, which is a
specification for defining interfaces which can be used between
different CORBA implementations.

%package devel
Summary:  Header files and libraries needed for libIDL development
Group:    Development/Libraries
Requires: libIDL = %{version}

%description devel
This package includes the header files and libraries needed for
developing programs using libIDL.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=$RPM_BUILD_ROOT/%prefix
gzip -9 $RPM_BUILD_ROOT/%prefix/info/*.info

%post   -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%post devel
/sbin/install-info %prefix/info/libIDL.info.gz %prefix/info/dir

%preun devel
if [ $1 = 0 ]; then
	/sbin/install-info --delete %prefix/info/libIDL.info.gz %prefix/info/dir
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc ChangeLog AUTHORS README* NEWS BUGS
%prefix/lib/lib*.so.*.*

%files devel
%defattr(-,root,root)
%prefix/lib/lib*.a
%prefix/lib/lib*.so
%prefix/info/libIDL.info.gz
%prefix/include/*
