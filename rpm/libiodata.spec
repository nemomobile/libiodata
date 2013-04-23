%define _name iodata
Name:     libiodata
Version:  0.19
Release:  1
Summary:  Library for input/ouput data
Group:    System/System Control
License:  LGPLv2
URL:      http://meego.gitorious.org/meego-middleware/iodata
Source0:  %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(QtCore) >= 4.5
BuildRequires: bison
BuildRequires: flex

%description
This package provides a library for writing and reading structured data.

%package devel
Summary:  Development package for %{name}
Group:    Development/Libraries
Requires: pkgconfig(QtCore) >= 4.5
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
%setup -q -n %{name}-%{version}

%build
export IODATA_VERSION=%version
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
%{_bindir}/iodata-type-to-c++
%{_includedir}/iodata/*
%{_libdir}/%{name}.so
%{_datadir}/qt4/mkspecs/features/iodata.prf

%files tests
%defattr(-,root,root,-)
%doc COPYING
%{_bindir}/%{_name}-test
%{_datadir}/%{name}-tests/tests.xml
