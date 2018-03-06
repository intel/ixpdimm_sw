#
# spec file for package ixpdimm_sw
#
# Copyright (c) 2016 Intel Corporation
# Copyright (c) 2016 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


%define product_name ixpdimm_sw
%define product_base_name ixpdimm
%define build_version 99.99.99.9999
%define api_release 1
%define apibase %{product_base_name}
%define apiname lib%{apibase}%{api_release}
%define cliname %{product_base_name}-cli
%define clilibname lib%{cliname}%{api_release}
%define monitorname %{product_base_name}-monitor
%define cimlibs lib%{product_base_name}-cim%{api_release}
%define dname %{product_base_name}-devel
%define srcname %{product_name}-%{build_version}
#define _unpackaged_files_terminate_build 0

Name:           %{product_name}
Version:        %{build_version}
Release:        1%{?dist}
Summary:        API for development of IXPDIMM management utilities
License:        BSD-3-Clause
Group:          System/Daemons
Url:            https://01.org/ixpdimm-sw
Source:         https://github.com/01org/%{product_name}/archive/ixpdimm_sw-%{build_version}.tar.gz
BuildRequires:  distribution-release
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(libkmod)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(libndctl)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  libnuma-devel
BuildRequires:  sblim-cmpi-devel
BuildRequires:  python
BuildRequires:  cmake
BuildRequires:  gettext
BuildRequires:  groff
BuildRequires:  libinvm-cli-devel
BuildRequires:  libinvm-cim-devel
BuildRequires:  libinvm-i18n-devel

ExclusiveArch: x86_64

%description
An application program interface (API) for configuring and managing
%{product_name}. Including basic inventory, capacity provisioning,
health monitoring, and troubleshooting.

%package -n %{apiname}
Summary:        API for development of %{product_name} management utilities
Group:          System/Libraries
Requires:       %{apibase}-data
Requires:       libndctl6 >= 58.2
Requires:       libinvm-i18n2 >= 01.01
Obsoletes:      ixpdimm_sw
Obsoletes:      libixpdimm-core

%description -n %{apiname}
An application program interface (API) for configuring and managing
%{product_name}. Including basic inventory, capacity provisioning,
health monitoring, and troubleshooting.

%package -n %{apibase}-data
Summary:        Data files for %{apibase}
Group:          System/Libraries
Conflicts:      ixpdimm_sw

%description -n %{apibase}-data
An application program interface (API) for configuring and managing
%{product_name}. Including basic inventory, capacity provisioning,
health monitoring, and troubleshooting.

%package -n %{dname}
Summary:        Development files for %{name}
Group:          Development/Libraries/C and C++
Requires:       %{apiname} = %{version}-%{release}
Obsoletes:      ixpdimm_sw-devel

%description -n %{dname}
The %{name}-devel package contains header files for
developing applications that use %{name}.

%package -n %{cimlibs}
Summary:        CIM provider for %{name}
Group:          System/Libraries
Requires:       libinvm-cim2 >= 01.01
Requires:       pywbem
Requires(post): pywbem
Requires(pre):  pywbem

%description -n %{cimlibs}
%{cimlibs} is a common information model (CIM) provider that exposes
%{product_name} as standard CIM objects in order to plug-in to various
common information model object managers (CIMOMS).

%package -n %{monitorname}
Summary:        Daemon for monitoring the status of %{product_name}
Group:          System/Monitoring
BuildRequires:  systemd-rpm-macros
%{?systemd_requires}
Requires:       %{cimlibs} = %{version}-%{release}

%description -n %{monitorname}
A daemon for monitoring the health and status of %{product_name}

%package -n %{cliname}
Summary:        CLI for managment of %{product_name}
Group:          System/Management
Requires:       %{clilibname} = %{version}-%{release}
Requires:       libinvm-cli2 >= 01.01
Requires:       libinvm-i18n2 >= 01.01

%description -n %{cliname}
A command line interface (CLI) application for configuring and
managing %{product_name}. Including commands for basic inventory,
capacity provisioning, health monitoring, and troubleshooting.

%package -n %{clilibname}
Summary:        CLI for managment of %{product_name}
Group:          System/Management
Requires:       libinvm-cli2 >= 01.01
Requires:       libinvm-cim2 >= 01.01
Requires:       libinvm-i18n2 >= 01.01

%description -n %{clilibname}
A command line interface (CLI) application for configuring and
managing %{product_name}. Including commands for basic inventory,
capacity provisioning, health monitoring, and troubleshooting.

%prep
%setup -q -n %{srcname}

%build
%cmake -DBUILDNUM=%{version} -DCMAKE_INSTALL_PREFIX=/usr -DRELEASE=ON \
    -DRPM_BUILD=ON -DLINUX_PRODUCT_NAME=%{name} \
    -DCMAKE_INSTALL_LIBDIR=%{_libdir} \
    -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} \
    -DCMAKE_INSTALL_BINDIR=%{_bindir} \
    -DCMAKE_INSTALL_DATAROOTDIR=%{_datadir} \
    -DCMAKE_INSTALL_MANDIR=%{_mandir} \
    -DCMAKE_INSTALL_FULL_LOCALSTATEDIR=%{_localstatedir} \
    -DINSTALL_UNITDIR=%{_unitdir} \
    -DCFLAGS_EXTERNAL="%{?optflags}" \
    -DEXTERNAL=ON
make -f Makefile %{?_smp_mflags}

%install
%{!?_cmake_version: cd build}
make -f Makefile install DESTDIR=%{buildroot}
mkdir -p %{buildroot}%{_prefix}/sbin
ln -sf service %{buildroot}%{_sbindir}/rc%{monitorname}

%pre -n %{monitorname}
%service_add_pre ixpdimm-monitor.service

%post -n %{apiname} -p /sbin/ldconfig
%post -n %{clilibname} -p /sbin/ldconfig
%post -n %{cimlibs} -p /sbin/ldconfig

%post -n %{monitorname}
%service_add_post ixpdimm-monitor.service

%postun -n %{apiname} -p /sbin/ldconfig
%postun -n %{cimlibs} -p /sbin/ldconfig
%postun -n %{clilibname} -p /sbin/ldconfig

%preun -n %{monitorname}
%service_del_preun ixpdimm-monitor.service

%postun -n %{monitorname}
%service_del_postun ixpdimm-monitor.service

%files -n %{apiname}
%defattr(-,root,root)
%{_libdir}/libixpdimm.so.*
%{_libdir}/libixpdimm-core.so.*
%{_libdir}/libixpdimm-common.so.*
%doc LICENSE

%files -n %{apibase}-data
%defattr(644,root,root)
%dir %{_localstatedir}/lib/%{product_name}
%{_localstatedir}/lib/%{product_name}/*.pem*
%{_localstatedir}/lib/%{product_name}/*.dat*

%files -n %{dname}
%defattr(-,root,root)
%{_libdir}/libixpdimm.so
%{_libdir}/libixpdimm-core.so
%{_libdir}/libixpdimm-common.so
%{_libdir}/libixpdimm-cli.so
%{_libdir}/libixpdimm-cim.so
%attr(644,root,root) %{_includedir}/nvm_types.h
%attr(644,root,root) %{_includedir}/nvm_management.h
%attr(644,root,root) %{_includedir}/export_api.h
%doc LICENSE

%files -n %{cimlibs}
%defattr(-,root,root)
%{_libdir}/libixpdimm-cim.so.*
%doc LICENSE

%files -n %{monitorname}
%defattr(-,root,root)
%{_bindir}/ixpdimm-monitor
%{_sbindir}/rcixpdimm-monitor
%{_unitdir}/ixpdimm-monitor.service
%attr(644,root,root) %{_mandir}/man8/ixpdimm-monitor*
%doc LICENSE

%files -n %{cliname}
%defattr(-,root,root)
%{_bindir}/ixpdimm-cli
%attr(644,root,root) %{_mandir}/man8/ixpdimm-cli*
%doc LICENSE

%files -n %{clilibname}
%defattr(-,root,root)
%{_libdir}/libixpdimm-cli.so.*

%changelog
