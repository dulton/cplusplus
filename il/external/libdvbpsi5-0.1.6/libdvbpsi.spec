%define name		libdvbpsi
%define version		0.1.6
%define release		1

%define major		5
%define lib_name	%{name}%{major}

%define redhat 0
%if %redhat
# some mdk macros that do not exist in rh
%define configure2_5x CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
%define make %__make
%define makeinstall_std %__make DESTDIR="$RPM_BUILD_ROOT" install
# adjust define for Redhat.
%endif


Summary:	A library for decoding and generating MPEG 2 and DVB PSI sections.
Name:		%{name}
Version:	%{version}
Release:	%{release}
Packager:	Yves Duret <yves@zarb.org>
License:	GPL
URL:		http://developers.videolan.org/libdvbpsi/
Group:		System/Libraries
Source:		http://www.videolan.org/pub/videolan/libdvbpsi/%{version}/%{name}%{major}-%{version}.tar.bz2
BuildRoot:	%_tmppath/%name%major-%version-%release-root

%description
libdvbpsi is a simple library designed for MPEG 2 TS and DVB PSI tables
decoding and generating. The important features are:
 * PAT decoder and genarator.
 * PMT decoder and generator.

%package -n %{lib_name}
Summary:	A library for decoding and generating MPEG 2 and DVB PSI sections.
Group:		System/Libraries
Provides:	%name

%description -n %{lib_name}
libdvbpsi is a simple library designed for MPEG 2 TS and DVB PSI tables
decoding and generating. The important features are:
 * PAT decoder and genarator.
 * PMT decoder and generator.

%package -n %{lib_name}-devel
Summary:	Development tools for programs which will use the libdvbpsi library.
Group:		Development/C
Provides:	%name-devel = %version-%release
Requires:	%{lib_name} = %version-%release

%description -n %{lib_name}-devel
The %{name}-devel package includes the header files and static libraries
necessary for developing programs which will manipulate MPEG 2 and DVB PSI
information using the %{name} library.

If you are going to develop programs which will manipulate MPEG 2 and DVB PSI
information you should install %{name}-devel.  You'll also need to have
the %name package installed.


%prep
%setup -q -n %{lib_name}-%{version}

%build
%configure2_5x --enable-release
%make 

%install
rm -rf %buildroot
%makeinstall_std

%clean
rm -rf %buildroot

%post -n %{lib_name} -p /sbin/ldconfig

%postun -n %{lib_name} -p /sbin/ldconfig

%files -n %{lib_name}
%defattr(-,root,root,-)
%doc AUTHORS README COPYING ChangeLog
%{_libdir}/*.so.*

%files -n %{lib_name}-devel
%defattr(-,root,root)
%doc COPYING
%{_libdir}/*a
%{_libdir}/*.so
%{_includedir}/*

%changelog
* Thu Oct, 22 2007 Jean-Paul Saman <jpsaman@videolan.org>
- New cat support
- Fix EIT discontinuities
- new example tool for checking an MPEG-2 TS file
- 0.1.6 release

* Thu Sep 22 2005 Jean-Paul Saman <jpsaman@videolan.org>
- Remove conflicting redefine of release
- Fix typo's

* Wed Jul 6 2005 Sam Hocevar <sam+rpm@zoy.org>
- 0.1.5 release.

* Fri Jan 2 2004 Sam Hocevar <sam@zoy.org>
- 0.1.4 release.

* Tue Jul 29 2003 Yves Duret <yves@zarb.org>
- 0.1.3 release.

* Fri Dec 13 2002 Yves Duret <yves@zarb.org> 0.1.2-2mdk
- s#Copyright#License#
- include the libtool .la files.
- use macros.
- update URL: tag.

* Fri Oct 11 2002 Samuel Hocevar <sam@zoy.org>
- 0.1.2 release.

* Sat May 18 2002 Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
- 0.1.1 release.

* Mon Apr 8 2002 Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
- split into two separate packages.

* Thu Apr 4 2002 Jean-Paul Saman <saman@natlab.research.philips.com>
- first version of package for redhat systems.

