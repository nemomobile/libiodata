%define _name iodata-qt5
Name:     libiodata-qt5
Version:  0.17
Release:  1
Summary:  Library for input/ouput data
Group:    System/System Control
License:  LGPLv2
URL:      http://meego.gitorious.org/meego-middleware/iodata
Source0:  %{_name}-%{version}.tar.bz2

BuildRequires: pkgconfig(Qt5Core)
BuildRequires: bison
BuildRequires: flex
BuildRequires: libqmlog-qt5-devel

%description
This package provides a library for writing and reading structured data.

%package devel
Summary:  Development package for %{name}
Group:    Development/Libraries
Requires: pkgconfig(Qt5Core)
Requires: %{name} = %{version}-%{release}

%description devel
Provides header files for iodata library.

%package tests
Summary:  Testcases for iodata library
Group:    Development/System
Requires: testrunner-lite

%description tests
%{summary}.

%prep
%setup -q -n %{_name}-%{version}

%build
export IODATA_VERSION=`head -n1 debian/changelog | sed "s/.*(\([^)+]*\).*/\1/"`
%qmake
make

%install
%qmake_install
install -d %{buildroot}/%{_datadir}/%{name}-tests/
mv %{buildroot}/%{_datadir}/%{_name}-tests/tests.xml %{buildroot}/%{_datadir}/%{name}-tests/tests.xml

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc COPYING debian/changelog debian/copyright
%{_libdir}/%{name}.so.*

%files devel
%defattr(-,root,root,-)
%doc COPYING
%{_bindir}/iodata-qt5-type-to-c++
%{_includedir}/iodata-qt5/*
%{_libdir}/%{name}.so
%{_datadir}/qt5/mkspecs/features/iodata-qt5.prf

%files tests
%defattr(-,root,root,-)
%doc COPYING
%{_bindir}/%{_name}-test
%{_datadir}/%{name}-tests/tests.xml
