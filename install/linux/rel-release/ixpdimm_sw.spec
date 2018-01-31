%define package_name ixpdimm_sw
%define product_name ixpdimm
%define api_name lib%{product_name}
%define api_dname %{product_name}-devel
%define data_name %{product_name}-data
%define cim_lib_name lib%{product_name}-cim
%define monitor_name %{product_name}-monitor
%define cli_name %{product_name}-cli
%define cli_lib_name lib%{product_name}-cli

%define build_version 99.99.99.9999
%define invm_framework_build_version 99.99.99.9999
%define _unpackaged_files_terminate_build 0

Name: %{package_name}
Version: %{build_version}
Release: 1%{?dist}
Summary: API for development of IXPDIMM management utilities
License: BSD
Group: Applications/System
URL: https://01.org/ixpdimm-sw
Source: https://github.com/01org/ixpdimm_sw/releases/download/v%{version}/%{name}-%{version}.tar.gz
ExclusiveArch: x86_64

BuildRequires: pkgconfig(libkmod)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(libndctl)
BuildRequires: pkgconfig(openssl)
BuildRequires: numactl-devel
BuildRequires: sblim-cmpi-devel
BuildRequires: python2
BuildRequires: cmake
BuildRequires: gettext

%description
An application program interface (API) which provides programmatic access to
the IXPSIMM SW functionality.

%prep
%setup -q -n %{name}-%{version}

%package -n %{api_name}
Summary:        API for development of %{product_name} management utilities
Group:          System/Libraries
Requires:       %{data_name}
Requires:	ndctl-libs >= 58.2
Requires:	invm-frameworks%{?_isa} >= %{version}-%{release}

%description -n %{api_name}
An application program interface (API) which provides programmatic access to
the IXPSIMM SW functionality.

%package -n %{api_dname}
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{api_name}%{?_isa} = %{version}-%{release}

%description -n %{api_dname}
The %{api_dname} package contains header files for
developing applications that use IXPDIMM SW.

%package -n %{data_name}
Summary:        Data files for %{package_name}
Group:          System/Libraries

%description -n %{data_name}
Data files for %{package_name}

%package -n %{cim_lib_name}
Summary:        CIM provider library for IXPDIMM SW
Group:          Development/Libraries
Requires:       %{api_name}%{?_isa} = %{version}-%{release}
Requires:       pywbem
Requires(pre):  pywbem
Requires(post): pywbem

%description -n %{cim_lib_name}
A Common Information Model (CIM) provider library to expose the IXPDIMM SW
functionality as standard CIM objects to plug-in to common information
model object managers (CIMOMs).

%package -n %{monitor_name}
Summary:        Daemon for monitoring the status of IXPDIMM
Group:          System Environment/Daemons
Requires:       %{cim_lib_name}%{?_isa} = %{version}-%{release}
%{?systemd_requires}
BuildRequires:  systemd

%description -n %{monitor_name}
A monitor daemon for monitoring the health and status of IXPDIMMs.

%package -n %{cli_name}
Summary:        CLI for management of IXPDIMM
Group:          Development/Tools
Requires:       %{cli_lib_name}%{?_isa} = %{version}-%{release}

%description -n %{cli_name}
A Command Line Interface (CLI) application for configuring and
managing IXPDIMMs from the command line.

%package -n %{cli_lib_name}
Summary:        CLI for managment of %{product_name}
Group:          System/Management
Requires:       %{cim_lib_name}%{?_isa} = %{version}-%{release}

%description -n %{cli_lib_name}
A library for IXPDIMM CLI applications

%package -n invm-frameworks
Summary:        Library files for invm-frameworks
Group:          Development/Libraries
#The following packages are deprecated and now provided by invm-frameworks
Conflicts:      libinvm-cim
Conflicts:      libinvm-cli
Conflicts:      libinvm-i18n

%description -n invm-frameworks
Framework library supporting a subset of Internationalization (I18N)
functionality, storage command line interface (CLI) applications, storage
common information model (CIM) providers.

%package -n invm-frameworks-devel
Summary:        Development files for invm-frameworks-devel
Group:          Development/Libraries
Requires:       invm-frameworks%{?_isa} = %{version}-%{release}
#The following packages are deprecated and now provided by invm-frameworks-devel
Conflicts:      libinvm-cim-devel
Conflicts:      libinvm-cli-devel
Conflicts:      libinvm-i18n-devel

%description -n invm-frameworks-devel
The invm-frameworks-devel package contains header files for
developing applications that use invm-frameworks.

%build
%cmake -DBUILDNUM=%{version} -DCMAKE_INSTALL_PREFIX=/usr -DRELEASE=ON \
    -DRPM_BUILD=ON -DLINUX_PRODUCT_NAME=%{name} -DRPM_ROOT=%{buildroot} \
    -DCMAKE_INSTALL_LIBDIR=%{_libdir} \
    -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} \
    -DCMAKE_INSTALL_BINDIR=%{_bindir} \
    -DCMAKE_INSTALL_DATAROOTDIR=%{_datadir} \
    -DCMAKE_INSTALL_MANDIR=%{_mandir} \
    -DCMAKE_INSTALL_FULL_LOCALSTATEDIR=%{_localstatedir} \
    -DINSTALL_UNITDIR=%{_unitdir} \
    -DCFLAGS_EXTERNAL="%{?optflags}"
make -f Makefile %{?_smp_mflags}

%install
%{!?_cmake_version: cd build}
make -f Makefile install DESTDIR=%{buildroot}

%post -n %{monitor_name}
%systemd_post ixpdimm-monitor.service

%post -n %{api_name} -p /sbin/ldconfig
%post -n %{cli_lib_name} -p /sbin/ldconfig
%post -n invm-frameworks -p /sbin/ldconfig

%post -n %{cim_lib_name}
/sbin/ldconfig
if [ -x /usr/sbin/cimserver ]
then
        cimserver --status &> /dev/null
        if [ $? -eq 0 ]
        then
        CIMMOF=cimmof
        else
    for repo in /var/lib/Pegasus /var/lib/pegasus /usr/local/var/lib/pegasus /var/local/lib/pegasus /var/opt/tog-pegasus /opt/ibm/icc/cimom
    do
      if [ -d $repo/repository ]
      then
          CIMMOF="cimmofl -R $repo"
      fi
    done
        fi
        for ns in interop root/interop root/PG_Interop;
        do
           $CIMMOF -E -n$ns %{_datadir}/%{name}/Pegasus/mof/pegasus_register.mof &> /dev/null
           if [ $? -eq 0 ]
           then
                $CIMMOF -uc -n$ns %{_datadir}/%{name}/Pegasus/mof/pegasus_register.mof &> /dev/null
                $CIMMOF -uc -n$ns %{_datadir}/%{name}/Pegasus/mof/profile_registration.mof &> /dev/null
                break
           fi
       done
       $CIMMOF -aE -uc -n root/intelwbem %{_datadir}/%{name}/Pegasus/mof/intelwbem.mof &> /dev/null
fi
if [ -x /usr/sbin/sfcbd ]
then
    RESTART=0
    systemctl is-active sblim-sfcb.service &> /dev/null
    if [ $? -eq 0 ]
    then
        RESTART=1
        systemctl stop sblim-sfcb.service &> /dev/null
    fi

    sfcbstage -n root/intelwbem -r %{_datadir}/%{name}/sfcb/INTEL_NVDIMM.reg %{_datadir}/%{name}/sfcb/sfcb_intelwbem.mof
    sfcbrepos -f

    if [[ $RESTART -gt 0 ]]
    then
        systemctl start sblim-sfcb.service &> /dev/null
    fi
fi

%postun -n %{api_name} -p /sbin/ldconfig
%postun -n %{cli_lib_name} -p /sbin/ldconfig
%postun -n invm-frameworks -p /sbin/ldconfig

%pre -n libixpdimm-cim
# If upgrading, deregister old version
if [ "$1" -gt 1 ]; then
        RESTART=0
        if [ -x /usr/sbin/cimserver ]
        then
                cimserver --status &> /dev/null
                if [ $? -gt 0 ]
                then
                        RESTART=1
                        cimserver enableHttpConnection=false enableHttpsConnection=false enableRemotePrivilegedUserAccess=false slp=false &> /dev/null
                fi
                cimprovider -d -m intelwbemprovider &> /dev/null
                cimprovider -r -m intelwbemprovider &> /dev/null
                mofcomp -v -r -n root/intelwbem %{_datadir}/%{name}/Pegasus/mof/intelwbem.mof &> /dev/null
                mofcomp -v -r -n root/intelwbem %{_datadir}/%{name}/Pegasus/mof/profile_registration.mof &> /dev/null
                if [[ $RESTART -gt 0 ]]
                then
                    cimserver -s &> /dev/null
                fi
        fi
fi

%preun -n libixpdimm-cim
RESTART=0
if [ -x /usr/sbin/cimserver ]
then
        cimserver --status &> /dev/null
        if [ $? -gt 0 ]
        then
                RESTART=1
                cimserver enableHttpConnection=false enableHttpsConnection=false enableRemotePrivilegedUserAccess=false slp=false &> /dev/null
        fi
        cimprovider -d -m intelwbemprovider &> /dev/null
        cimprovider -r -m intelwbemprovider &> /dev/null
        mofcomp -r -n root/intelwbem %{_datadir}/%{name}/Pegasus/mof/intelwbem.mof &> /dev/null
        mofcomp -v -r -n root/intelwbem %{_datadir}/%{name}/Pegasus/mof/profile_registration.mof &> /dev/null
        if [[ $RESTART -gt 0 ]]
        then
            cimserver -s &> /dev/null
        fi
fi

if [ -x /usr/sbin/sfcbd ]
then
    RESTART=0
    systemctl is-active sblim-sfcb.service &> /dev/null
    if [ $? -eq 0 ]
    then
        RESTART=1
        systemctl stop sblim-sfcb.service &> /dev/null
    fi

    sfcbunstage -n root/intelwbem -r INTEL_NVDIMM.reg sfcb_intelwbem.mof
    sfcbrepos -f

    if [[ $RESTART -gt 0 ]]
    then
        systemctl start sblim-sfcb.service &> /dev/null
    fi
fi

%preun -n %{monitor_name}
%systemd_preun stop ixpdimm-monitor.service

%postun -n  %{monitor_name}
%systemd_postun_with_restart ixpdimm-monitor.service

%files -n %{api_name}
%defattr(-,root,root)
%doc README.md
%{_libdir}/libixpdimm.so.*
%{_libdir}/libixpdimm-core.so.*
%{_libdir}/libixpdimm-common.so.*
%license LICENSE

%files -n %{api_dname}
%defattr(-,root,root)
%doc README.md
%{_libdir}/libixpdimm.so
%{_libdir}/libixpdimm-core.so
%{_libdir}/libixpdimm-common.so
%{_libdir}/libixpdimm-cli.so
%{_libdir}/libixpdimm-cim.so
%attr(644,root,root) %{_includedir}/nvm_types.h
%attr(644,root,root) %{_includedir}/nvm_management.h
%attr(644,root,root) %{_includedir}/export_api.h
%license LICENSE

%files -n %{data_name}
%defattr(644,root,root)
%dir %{_sharedstatedir}/%{name}
%{_sharedstatedir}/%{name}/*.pem
%config %{_sharedstatedir}/%{name}/*.dat*

%files -n %{cim_lib_name}
%defattr(-,root,root)
%doc README.md
%{_libdir}/libixpdimm-cim.so.*
%dir %{_datadir}/%{name}/Pegasus
%dir %{_datadir}/%{name}/Pegasus/mof
%dir %{_datadir}/%{name}/sfcb
%attr(644,root,root) %{_datadir}/%{name}/sfcb/*.reg
%attr(644,root,root) %{_datadir}/%{name}/sfcb/*.mof
%attr(644,root,root) %{_datadir}/%{name}/Pegasus/mof/*.mof
%license LICENSE

%files -n %{monitor_name}
%defattr(-,root,root)
%{_bindir}/ixpdimm-monitor
%{_unitdir}/ixpdimm-monitor.service
%attr(644,root,root) %{_mandir}/man8/ixpdimm-monitor*
%license LICENSE

%files -n %{cli_name}
%defattr(-,root,root)
%{_bindir}/ixpdimm-cli
%attr(644,root,root) %{_mandir}/man8/ixpdimm-cli*
%license LICENSE

%files -n %{cli_lib_name}
%defattr(-,root,root)
%{_libdir}/libixpdimm-cli.so.*

%files -n invm-frameworks
%defattr(-,root,root)
%doc README.md
%{_libdir}/libinvm-i18n.so.*
%{_libdir}/libinvm-cli.so.*
%{_libdir}/libinvm-cim.so.*
%license LICENSE

%files -n invm-frameworks-devel
%defattr(-,root,root)
%doc README.md
%{_libdir}/libinvm-i18n.so
%{_libdir}/libinvm-cli.so
%{_libdir}/libinvm-cim.so
%dir %{_includedir}/libinvm-i18n
%dir %{_includedir}/libinvm-cli
%dir %{_includedir}/libinvm-cim
%attr(644,root,root) %{_includedir}/libinvm-cli/*.h
%attr(644,root,root) %{_includedir}/libinvm-i18n/*.h
%attr(644,root,root) %{_includedir}/libinvm-cim/*.h
%license LICENSE

%changelog
