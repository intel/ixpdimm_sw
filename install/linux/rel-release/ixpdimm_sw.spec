%define build_version 99.99.99.9999
%define invm_framework_build_version 99.99.99.9999
%define _unpackaged_files_terminate_build 0

Name: ixpdimm_sw
Version: %{build_version}
Release: 2%{?dist}
Summary: API for development of IXPDIMM management utilities
License: BSD
Group: Applications/System
URL: https://01.org/ixpdimm-sw
Source: https://github.com/01org/ixpdimm_sw/releases/download/v%{version}/%{name}-%{version}.tar.gz
Requires: ndctl-libs >= 58.2
Requires: invm-frameworks >= 1.0.0.2007
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

%package -n %{name}-devel
Summary:        Development files for %{name}
License:        BSD
Group:          Development/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description -n %{name}-devel
The %{name}-devel package contains header files for
developing applications that use IXPDIMM SW.

%package -n libixpdimm-core
Summary:        Development files for %{name}
License:        BSD
Group:          Development/Libraries
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description -n libixpdimm-core
The libixpdimm-core package contains libraries that support
other IXPDIMM SW products.

%package -n libixpdimm-cim
Summary:        CIM provider library for IXPDIMM SW
License:        BSD
Group:          Development/Libraries
Requires:       libixpdimm-core%{?_isa} = %{version}-%{release}
Requires:       pywbem
Requires(pre):  pywbem
Requires(post): pywbem

%description -n libixpdimm-cim
A Common Information Model (CIM) provider library to expose the IXPDIMM SW
functionality as standard CIM objects to plug-in to common information
model object managers (CIMOMs).

%package -n ixpdimm-monitor
Summary:        Daemon for monitoring the status of IXPDIMM
License:        BSD
Group:          System Environment/Daemons
Requires:       libixpdimm-cim%{?_isa} = %{version}-%{release}
Requires:       systemd-units
BuildRequires:  systemd

%description -n ixpdimm-monitor
A monitor daemon for monitoring the health and status of IXPDIMMs.

%package -n ixpdimm-cli
Summary:        CLI for management of IXPDIMM
License:        BSD
Group:          Development/Tools
Requires:       libixpdimm-cim%{?_isa} = %{version}-%{release}

%description -n ixpdimm-cli
A Command Line Interface (CLI) application for configuring and
managing IXPDIMMs from the command line.

%package -n invm-frameworks
Version:	%{invm_framework_build_version}
Summary:        Library files for invm-frameworks
License:        BSD
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
Version:	%{invm_framework_build_version}
Summary:        Development files for invm-frameworks-devel
License:        BSD
Group:          Development/Libraries
Requires:       invm-frameworks%{?_isa} = %{invm_framework_build_version}-%{release}
#The following packages are deprecated and now provided by invm-frameworks-devel
Conflicts:      libinvm-cim-devel
Conflicts:      libinvm-cli-devel
Conflicts:      libinvm-i18n-devel

%description -n invm-frameworks-devel
The invm-frameworks-devel package contains header files for
developing applications that use invm-frameworks.

Version: %{build_version}

%build
%cmake -DBUILDNUM=%{build_version} -DRELEASE=ON -DRPM_BUILD=ON -DLINUX_PRODUCT_NAME=%{name} -DRPM_ROOT=%{buildroot} -DLIB_DIR=%{_libdir} -DINCLUDE_DIR=%{_includedir} -DBIN_DIR=%{_bindir} -DDATADIR=%{_sharedstatedir} -DUNIT_DIR=%{_unitdir} -DSYSCONF_DIR=%{_sysconfdir} -DMANPAGE_DIR=%{_mandir} -DCFLAGS_EXTERNAL="%{?optflags}"
make -f Makefile %{?_smp_mflags}

%install
%{!?_cmake_version: cd build}
make -f Makefile install
cp -rf ./output/build/linux/real/release/include/libinvm-i18n %{buildroot}%{_includedir}
cp -rf ./output/build/linux/real/release/include/libinvm-cim %{buildroot}%{_includedir}
cp -rf ./output/build/linux/real/release/include/libinvm-cli %{buildroot}%{_includedir}


%post -n libixpdimm-core -p /sbin/ldconfig

%post -n libixpdimm-cim
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
           $CIMMOF -E -n$ns %{_sharedstatedir}/%{name}/Pegasus/mof/pegasus_register.mof &> /dev/null
           if [ $? -eq 0 ]
           then
                $CIMMOF -uc -n$ns %{_sharedstatedir}/%{name}/Pegasus/mof/pegasus_register.mof &> /dev/null
                $CIMMOF -uc -n$ns %{_sharedstatedir}/%{name}/Pegasus/mof/profile_registration.mof &> /dev/null
                break
           fi
       done
       $CIMMOF -aE -uc -n root/intelwbem %{_sharedstatedir}/%{name}/Pegasus/mof/intelwbem.mof &> /dev/null
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

    sfcbstage -n root/intelwbem -r %{_sharedstatedir}/%{name}/sfcb/INTEL_NVDIMM.reg %{_sharedstatedir}/%{name}/sfcb/sfcb_intelwbem.mof
    sfcbrepos -f

    if [[ $RESTART -gt 0 ]]
    then
        systemctl start sblim-sfcb.service &> /dev/null
    fi
fi

%post -n ixpdimm-monitor
%systemd_post ixpdimm-monitor.service
/bin/systemctl --no-reload enable ixpdimm-monitor.service &> /dev/null || :

%post -n ixpdimm_sw -p /sbin/ldconfig

%postun -n ixpdimm_sw
/sbin/ldconfig
rm -f %{_sharedstatedir}/%{name}/*.dat.log
rm -f %{_sharedstatedir}/%{name}/*.dat-journal

%postun -n libixpdimm-core -p /sbin/ldconfig
%postun -n libixpdimm-cim -p /sbin/ldconfig

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
                mofcomp -v -r -n root/intelwbem %{_sharedstatedir}/%{name}/Pegasus/mof/intelwbem.mof &> /dev/null
                mofcomp -v -r -n root/intelwbem %{_sharedstatedir}/%{name}/Pegasus/mof/profile_registration.mof &> /dev/null
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
        mofcomp -r -n root/intelwbem %{_sharedstatedir}/%{name}/Pegasus/mof/intelwbem.mof &> /dev/null
        mofcomp -v -r -n root/intelwbem %{_sharedstatedir}/%{name}/Pegasus/mof/profile_registration.mof &> /dev/null
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

%preun -n ixpdimm-monitor
%systemd_preun stop ixpdimm-monitor.service

%postun -n ixpdimm-monitor
%systemd_postun_with_restart ixpdimm-monitor.service

%post -n ixpdimm-cli -p /sbin/ldconfig
%postun -n ixpdimm-cli -p /sbin/ldconfig

%files
%doc README.md
%{_libdir}/libixpdimm.so.*
%dir %{_sharedstatedir}/%{name}
%{_sharedstatedir}/%{name}/*.pem
%config %{_sharedstatedir}/%{name}/*.dat*
%license LICENSE

%files -n %{name}-devel
%doc README.md
%{_libdir}/libixpdimm.so
%{_libdir}/libixpdimm-core.so
%{_libdir}/cmpi/libixpdimm-cim.so
%{_libdir}/libixpdimm-cli.so
%{_includedir}/nvm_types.h
%{_includedir}/nvm_management.h
%license LICENSE

%files -n libixpdimm-core
%doc README.md
%{_libdir}/libixpdimm-core.so*
%license LICENSE

%files -n libixpdimm-cim
%doc README.md
%{_libdir}/cmpi/libixpdimm-cim.so*
%dir %{_sharedstatedir}/%{name}/Pegasus
%dir %{_sharedstatedir}/%{name}/Pegasus/mof
%dir %{_sharedstatedir}/%{name}/sfcb
%{_sharedstatedir}/%{name}/sfcb/*.reg
%{_sharedstatedir}/%{name}/sfcb/*.mof
%{_sharedstatedir}/%{name}/Pegasus/mof/*.mof
%config(noreplace) %{_sysconfdir}/ld.so.conf.d/%{name}-%{_arch}.conf
%license LICENSE

%files -n ixpdimm-monitor
%{_bindir}/ixpdimm-monitor
%{_unitdir}/ixpdimm-monitor.service
%license LICENSE
%{_mandir}/man8/ixpdimm-monitor*

%files -n ixpdimm-cli
%{_bindir}/ixpdimm-cli
%{_libdir}/libixpdimm-cli.so*
%license LICENSE
%{_mandir}/man8/ixpdimm-cli*

%files -n invm-frameworks
%doc README.md
%{_libdir}/libinvm-i18n.so.*
%{_libdir}/libinvm-cli.so.*
%{_libdir}/libinvm-cim.so.*
%license LICENSE

%files -n invm-frameworks-devel
%doc README.md
%{_libdir}/libinvm-i18n.so
%{_includedir}/libinvm-i18n
%{_libdir}/libinvm-cli.so
%{_includedir}/libinvm-cli
%{_libdir}/libinvm-cim.so
%{_includedir}/libinvm-cim
%license LICENSE

%post -n invm-frameworks -p /sbin/ldconfig
%postun -n invm-frameworks -p /sbin/ldconfig

%changelog
* Mon Oct 09 2017 Juston Li <juston.li@intel.com> - 01.00.00.2345-2
- fix python requires and update changelog

* Mon Oct 09 2017 Juston Li <juston.li@intel.com> - 01.00.00.2345-1
- ixpdimm_sw release v01.00.00.2345
- added invm-frameworks release v01.00.00.2006

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 01.00.00.2264-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 01.00.00.2264-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Mon Aug 29 2016 Namratha Kothapalli <namratha.n.kothapalli@intel.com> - 01.00.00.2113-1
- Update lib version dependencies

* Thu Dec 24 2015 Nicholas Moulin <nicholas.w.moulin@intel.com> - 01.00.00.2111-1
- Initial rpm release
