ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS = \
	-I $(top_srcdir)/src \
	-I $(top_srcdir)/src/lib \
	-I $(top_srcdir)/src/common \
	-D__VERSION_MAJOR__=$(PACKAGE_VERSION_MAJOR) \
	-D__VERSION_MINOR__=$(PACKAGE_VERSION_MINOR) \
	-D__VERSION_HOTFIX__=$(PACKAGE_VERSION_MICRO) \
	-D__VERSION_BUILDNUM__=$(PACKAGE_VERSION_BUILD)

AM_CXXFLAGS = \
	-std=gnu++98

bin_PROGRAMS = \
	ixpdimm-monitor \
	ixpdimm-cli \
	ixpdimm-create-db

lib_LTLIBRARIES = \
	libixpdimm-cim.la \
	libixpdimm-cli.la \
	libixpdimm-core.la \
	libixpdimm.la

noinst_LTLIBRARIES = \
	libixpdimm-common.la

libixpdimm_common_la_SOURCES = \
	src/common/encrypt/encrypt.c \
	src/common/encrypt/encrypt.h \
	src/common/file_ops/file_ops.c \
	src/common/guid/guid.c \
	src/common/guid/guid.h \
	src/common/persistence/config_settings.h \
	src/common/persistence/csv_log.c \
	src/common/persistence/csv_log.h \
	src/common/persistence/event.c \
	src/common/persistence/event.h \
	src/common/persistence/lib_persistence.c \
	src/common/persistence/lib_persistence.h \
	src/common/persistence/logging.c \
	src/common/persistence/logging.h \
	src/common/persistence/schema.c \
	src/common/persistence/schema.h \
	src/common/smbios/smbios.c \
	src/common/smbios/smbios.h \
	src/common/smbios/smbios_types.h \
	src/common/string/revision.c \
	src/common/string/revision.h \
	src/common/string/s_str.c \
	src/common/string/s_str.h \
	src/common/string/unicode_utilities.c \
	src/common/string/unicode_utilities.h \
	src/common/string/x_str.c \
	src/common/string/x_str.h \
	src/common/system/jedec_manufacturer.c \
	src/common/system/jedec_manufacturer.h \
	src/common/system/system.c \
	src/common/system/system.h \
	src/common/time/time_utilities.c \
	src/common/time/time_utilities.h \
	src/common/uid/uid.c \
	src/common/uid/uid.h \
	src/acpi/acpi.c \
	src/acpi/acpi.h \
	src/acpi/nfit.c \
	src/acpi/nfit_dimm.c \
	src/acpi/nfit.h \
	src/acpi/nfit_interleave.c \
	src/acpi/nfit_parser.c \
	src/acpi/nfit_parser.h \
	src/acpi/nfit_printer.c \
	src/acpi/nfit_printer.h \
	src/acpi/nfit_tables.h

libixpdimm_common_la_CFLAGS = \
	$(OPENSSL_CFLAGS) $(LIBCRYPTO_CFLAGS)

if LINUX
libixpdimm_common_la_SOURCES += \
	src/acpi/lnx_acpi.c \
	src/common/file_ops/lnx_file_ops.c \
	src/common/os/lnx_os.c
libixpdimm_common_la_CFLAGS += \
	-D__PRODUCT_DATADIR__=\"$(pkgdatadir)\"
endif
if WINDOWS
libixpdimm_common_la_SOURCES += \
	src/acpi/win_acpi.c \
	src/common/file_ops/win_file_ops.c \
	src/common/os/win_os.c
endif

libixpdimm_common_la_LDFLAGS = \
	$(OPENSSL_LIBS) $(LIBCRYPTO_LIBS)

libixpdimm_cim_la_SOURCES = \
	external/rapidxml-1.13/rapidxml.hpp \
	external/rapidxml-1.13/rapidxml_iterators.hpp \
	external/rapidxml-1.13/rapidxml_print.hpp \
	external/rapidxml-1.13/rapidxml_utils.hpp \
	src/common/common_types.h \
	src/common/cr_i18n.h \
	src/common/os/os_adapter.h \
	src/common/file_ops/file_ops_adapter.h \
	src/wbem/indication/InstIndicationFactory.cpp \
	src/wbem/indication/NVDIMMEventFactory.cpp \
	src/wbem/indication/InstIndicationFactory.h \
	src/wbem/indication/StaticFilterFactory.h \
	src/wbem/indication/StaticFilterFactory.cpp \
	src/wbem/indication/NVDIMMEventFactory.h \
	src/wbem/indication/NvmIndicationFactory.h \
	src/wbem/indication/NvmIndicationFactory.cpp \
	src/wbem/cimom/cmpi/IntelCmpiProvider.cpp \
	src/wbem/cimom/cmpi/sfcb_register.reg \
	src/wbem/mem_config/MemoryAllocationSettingsFactory.h \
	src/wbem/mem_config/MemoryConfigurationFactory.h \
	src/wbem/mem_config/PoolViewFactory.cpp \
	src/wbem/mem_config/MemoryResourcesFactory.h \
	src/wbem/mem_config/MemoryConfigurationCapabilitiesFactory.h \
	src/wbem/mem_config/MemoryConfigurationFactory.cpp \
	src/wbem/mem_config/MemoryConfigurationCapabilitiesFactory.cpp \
	src/wbem/mem_config/InterleaveSet.cpp \
	src/wbem/mem_config/MemoryConfigurationServiceFactory.h \
	src/wbem/mem_config/MemoryResourcesFactory.cpp \
	src/wbem/mem_config/InterleaveSet.h \
	src/wbem/mem_config/MemoryCapabilitiesFactory.h \
	src/wbem/mem_config/PoolViewFactory.h \
	src/wbem/mem_config/MemoryConfigurationServiceFactory.cpp \
	src/wbem/mem_config/MemoryAllocationSettingsFactory.cpp \
	src/wbem/mem_config/MemoryCapabilitiesFactory.cpp \
	src/wbem/performance/NVDIMMPerformanceViewFactory.cpp \
	src/wbem/performance/PerformanceMetricServiceFactory.cpp \
	src/wbem/performance/PerformanceMetricCommon.h \
	src/wbem/performance/PerformanceMetricServiceCapabilitiesFactory.cpp \
	src/wbem/performance/NVDIMMPerformanceViewFactory.h \
	src/wbem/performance/PerformanceMetricDefinitionFactory.h \
	src/wbem/performance/PerformanceMetricServiceCapabilitiesFactory.h \
	src/wbem/performance/PerformanceMetricDefinitionFactory.cpp \
	src/wbem/performance/PerformanceMetricFactory.cpp \
	src/wbem/performance/PerformanceMetricFactory.h \
	src/wbem/performance/PerformanceMetricServiceFactory.h \
	src/wbem/physical_asset/NVDIMMFactory.cpp \
	src/wbem/physical_asset/NVDIMMFactory.h \
	src/wbem/physical_asset/MemoryTopologyViewFactory.cpp \
	src/wbem/physical_asset/MemoryTopologyViewFactory.h \
	src/wbem/erasure/ErasureCapabilitiesFactory.h \
	src/wbem/erasure/ErasureServiceFactory.cpp \
	src/wbem/erasure/ErasureServiceFactory.h \
	src/wbem/erasure/ErasureCapabilitiesFactory.cpp \
	src/wbem/pegasus/PegasusRegisteredProfileFactory.h \
	src/wbem/memory/PersistentMemoryFactory.cpp \
	src/wbem/memory/SystemProcessorFactory.cpp \
	src/wbem/memory/VolatileMemoryFactory.cpp \
	src/wbem/memory/PersistentMemoryFactory.h \
	src/wbem/memory/RawMemoryFactory.cpp \
	src/wbem/memory/MemoryControllerFactory.h \
	src/wbem/memory/RawMemoryFactory.h \
	src/wbem/memory/MemoryControllerFactory.cpp \
	src/wbem/memory/VolatileMemoryFactory.h \
	src/wbem/memory/SystemProcessorFactory.h \
	src/wbem/software/AvailableFWFactory.cpp \
	src/wbem/software/NVDIMMSoftwareInstallationServiceFactory.h \
	src/wbem/software/NVDIMMSoftwareInstallationServiceFactory.cpp \
	src/wbem/software/HostSoftwareFactory.h \
	src/wbem/software/ElementSoftwareIdentityFactory.h \
	src/wbem/software/ManagementSoftwareIdentityFactory.cpp \
	src/wbem/software/NVDIMMCollectionFactory.h \
	src/wbem/software/NVDIMMFWVersionFactory.h \
	src/wbem/software/NVDIMMFWVersionFactory.cpp \
	src/wbem/software/NVDIMMDriverIdentityFactory.h \
	src/wbem/software/HostSoftwareFactory.cpp \
	src/wbem/software/NVDIMMDriverIdentityFactory.cpp \
	src/wbem/software/ElementSoftwareIdentityFactory.cpp \
	src/wbem/software/AvailableFWFactory.h \
	src/wbem/software/ManagementSoftwareIdentityFactory.h \
	src/wbem/software/NVDIMMCollectionFactory.cpp \
	src/wbem/NvmWbem.cpp \
	src/wbem/pmem_config/PersistentMemoryCapabilitiesFactory.h \
	src/wbem/pmem_config/PersistentConfigurationCapabilitiesFactory.cpp \
	src/wbem/pmem_config/PersistentMemoryNamespaceFactory.cpp \
	src/wbem/pmem_config/PersistentConfigurationCapabilitiesFactory.h \
	src/wbem/pmem_config/NamespaceViewFactory.h \
	src/wbem/pmem_config/NamespaceViewFactory.cpp \
	src/wbem/pmem_config/PersistentMemoryPoolFactory.h \
	src/wbem/pmem_config/NamespaceSettingsFactory.cpp \
	src/wbem/pmem_config/PersistentMemoryServiceFactory.cpp \
	src/wbem/pmem_config/PersistentMemoryPoolFactory.cpp \
	src/wbem/pmem_config/PersistentMemoryServiceFactory.h \
	src/wbem/pmem_config/PersistentMemoryCapabilitiesFactory.cpp \
	src/wbem/pmem_config/NamespaceSettingsFactory.h \
	src/wbem/pmem_config/PersistentMemoryNamespaceFactory.h \
	src/wbem/profile/RegisteredProfileFactory.cpp \
	src/wbem/profile/RegisteredProfileFactory.h \
	src/wbem/support/DiagnosticCompletionRecordFactory.cpp \
	src/wbem/support/SupportDataServiceFactory.cpp \
	src/wbem/support/DiagnosticIdentityFactory.h \
	src/wbem/support/EventLogFilter.cpp \
	src/wbem/support/SanitizeJobFactory.cpp \
	src/wbem/support/FWEventLogFactory.h \
	src/wbem/support/SanitizeJobFactory.h \
	src/wbem/support/EventLogFilter.h \
	src/wbem/support/DiagnosticLogFactory.cpp \
	src/wbem/support/NVDIMMSensorFactory.h \
	src/wbem/support/FWEventLogFactory.cpp \
	src/wbem/support/NVDIMMSensorFactory.cpp \
	src/wbem/support/NVDIMMLogEntryFactory.cpp \
	src/wbem/support/NVDIMMSensorViewFactory.cpp \
	src/wbem/support/NVDIMMDiagnosticFactory.cpp \
	src/wbem/support/NVDIMMEventLogFactory.cpp \
	src/wbem/support/DiagnosticCompletionRecordFactory.h \
	src/wbem/support/NVDIMMLogEntryFactory.h \
	src/wbem/support/NVDIMMEventLogFactory.h \
	src/wbem/support/OpaqueSupportDataFactory.h \
	src/wbem/support/DiagnosticLogFactory.h \
	src/wbem/support/OpaqueSupportDataFactory.cpp \
	src/wbem/support/NVDIMMDiagnosticFactory.h \
	src/wbem/support/NVDIMMSensorViewFactory.h \
	src/wbem/support/DiagnosticIdentityFactory.cpp \
	src/wbem/support/SupportDataServiceFactory.h \
	src/wbem/exception/NvmExceptionBadFilter.cpp \
	src/wbem/exception/NvmExceptionDiagnosticError.h \
	src/wbem/exception/NvmExceptionDiagnosticError.cpp \
	src/wbem/exception/NvmExceptionBadTarget.h \
	src/wbem/exception/NvmExceptionLibError.h \
	src/wbem/exception/NvmExceptionBadTarget.cpp \
	src/wbem/exception/NvmExceptionInvalidPoolConfig.cpp \
	src/wbem/exception/NvmExceptionLibError.cpp \
	src/wbem/exception/NvmExceptionUndoModifyFailed.h \
	src/wbem/exception/NvmExceptionNotManageable.h \
	src/wbem/exception/NvmExceptionUndoModifyFailed.cpp \
	src/wbem/exception/NvmExceptionNotManageable.cpp \
	src/wbem/exception/NvmExceptionInvalidPoolConfig.h \
	src/wbem/exception/NvmExceptionBadFilter.h \
	src/wbem/server/ServerChassisFactory.cpp \
	src/wbem/server/BaseServerFactory.cpp \
	src/wbem/server/BaseServerFactory.h \
	src/wbem/server/SystemCapabilitiesFactory.cpp \
	src/wbem/server/SystemCapabilitiesFactory.h \
	src/wbem/server/ServerChassisFactory.h \
	src/wbem/framework_interface/NvmInstanceFactory.cpp \
	src/wbem/framework_interface/NvmAssociationFactory.h \
	src/wbem/framework_interface/FrameworkExtensions.h \
	src/wbem/framework_interface/NvmProviderFactory.cpp \
	src/wbem/framework_interface/NvmIndicationService.h \
	src/wbem/framework_interface/NvmAssociationFactory.cpp \
	src/wbem/framework_interface/NvmProviderFactory.h \
	src/wbem/framework_interface/FrameworkExtensions.cpp \
	src/wbem/framework_interface/NvmIndicationService.cpp \
	src/wbem/framework_interface/NvmInstanceFactory.h \
	src/wbem/lib_interface/NvmApi.cpp \
	src/wbem/lib_interface/NvmApi.h \
	src/wbem/lib_interface/NvmContext.h \
	src/wbem/NvmStrings.h \
	src/monitor/EventMonitor.cpp \
	src/monitor/EventMonitor.h \
	src/monitor/NvmMonitorBase.cpp \
	src/monitor/NvmMonitorBase.h \
	src/monitor/PerformanceMonitor.cpp \
	src/monitor/PerformanceMonitor.h

if LINUX
libixpdimm_cim_la_SOURCES += \
	src/wbem/cimom/cmpi/cmpiMonitor.cpp \
	src/wbem/cimom/cmpi/cmpiMonitor.h
endif
if WINDOWS
libixpdimm_cim_la_SOURCES += \
	src/wbem/cimom/wmi/DllMain.cpp
endif

libixpdimm_cim_la_CXXFLAGS = \
	$(AM_CPPFLAGS) \
	$(AM_CXXFLAGS) \
	-I $(top_srcdir)/src/wbem \
	-I $(top_srcdir)/external/rapidxml-1.13 \
	$(INVM_CIM_CFLAGS)

if LINUX
libixpdimm_cim_la_CXXFLAGS += \
	-DCMPI_PLATFORM_LINUX_GENERIC_GNU=1
endif
if WINDOWS
libixpdimm_cim_la_CXXFLAGS += \
	-DCMPI_PLATFORM_WIN32_IX86_MSVC=1
endif

libixpdimm_cim_la_LDFLAGS = \
	-version-info 1:0:0 \
	$(OPENSSL_LIBS) $(LIBCRYPTO_LIBS) \
	$(INVM_CIM_LIBS)

libixpdimm_core_la_SOURCES = \
	src/core/NvmLibrary.h \
	src/core/NvmLibrary.cpp \
	src/core/StringList.h \
	src/core/Collection.h \
	src/core/StringList.cpp \
	src/core/LibWrapper.cpp \
	src/core/logs/Log.h \
	src/core/logs/LogService.cpp \
	src/core/logs/Log.cpp \
	src/core/logs/LogService.h \
	src/core/Collection.cpp \
	src/core/Result.h \
	src/core/memory_allocator/LayoutStepCheckDriverSupportsStorage.h \
	src/core/memory_allocator/LayoutStepCheckRequestLayoutDeviation.cpp \
	src/core/memory_allocator/RuleNamespacesExist.h \
	src/core/memory_allocator/LayoutStepCheckCurrentVolatileMode.cpp \
	src/core/memory_allocator/LayoutBuilder.cpp \
	src/core/memory_allocator/LayoutStepCheckRequestLayoutDeviation.h \
	src/core/memory_allocator/RuleNamespacesExist.cpp \
	src/core/memory_allocator/RuleNoDimms.cpp \
	src/core/memory_allocator/MemoryAllocator.cpp \
	src/core/memory_allocator/ReserveDimmSelector.h \
	src/core/memory_allocator/LayoutStepAppDirectSettingsNotRecommended.h \
	src/core/memory_allocator/PostLayoutAddressDecoderLimitCheck.cpp \
	src/core/memory_allocator/RequestRule.h \
	src/core/memory_allocator/LayoutStepReserveDimm.cpp \
	src/core/memory_allocator/LayoutStepCheckDriverSupportsAppDirect.cpp \
	src/core/memory_allocator/RuleProvisionCapacityNotSupported.h \
	src/core/memory_allocator/InterleaveableDimmSetBuilder.h \
	src/core/memory_allocator/MemoryAllocationUtil.h \
	src/core/memory_allocator/LayoutStepCheckDriverSupportsAppDirect.h \
	src/core/memory_allocator/LayoutStepCheckAsymmetricalPopulation.h \
	src/core/memory_allocator/LayoutBuilder.h \
	src/core/memory_allocator/LayoutStepLimitTotalMappedMemory.cpp \
	src/core/memory_allocator/InterleaveableDimmSetBuilder.cpp \
	src/core/memory_allocator/LayoutStepAppDirect.h \
	src/core/memory_allocator/RuleMemoryModeCapacityNotSupported.cpp \
	src/core/memory_allocator/LayoutStepMemory.h \
	src/core/memory_allocator/MemoryAllocationRequestBuilder.h \
	src/core/memory_allocator/LayoutStep.h \
	src/core/memory_allocator/RuleProvisionCapacityNotSupported.cpp \
	src/core/memory_allocator/RuleAppDirectNotSupported.cpp \
	src/core/memory_allocator/RuleDimmListInvalid.cpp \
	src/core/memory_allocator/PostLayoutAddressDecoderLimitCheck.h \
	src/core/memory_allocator/RulePartialSocketConfigured.cpp \
	src/core/memory_allocator/RuleMirroredAppDirectNotSupported.h \
	src/core/memory_allocator/ReserveDimmSelector.cpp \
	src/core/memory_allocator/RuleRejectLockedDimms.h \
	src/core/memory_allocator/LayoutStepReserveDimm.h \
	src/core/memory_allocator/LayoutStepStorage.cpp \
	src/core/memory_allocator/LayoutStepLimitTotalMappedMemory.h \
	src/core/memory_allocator/RuleStorageCapacityNotSupported.cpp \
	src/core/memory_allocator/RuleAppDirectNotSupported.h \
	src/core/memory_allocator/RuleDimmListInvalid.h \
	src/core/memory_allocator/RuleDimmHasConfigGoal.cpp \
	src/core/memory_allocator/RuleRejectLockedDimms.cpp \
	src/core/memory_allocator/PostLayoutCheck.h \
	src/core/memory_allocator/RuleReserveDimmPropertyInvalid.cpp \
	src/core/memory_allocator/LayoutStepStorage.h \
	src/core/memory_allocator/MemoryAllocator.h \
	src/core/memory_allocator/MemoryAllocationTypes.h \
	src/core/memory_allocator/LayoutStepAppDirect.cpp \
	src/core/memory_allocator/RulePartialSocketConfigured.h \
	src/core/memory_allocator/LayoutStepMemory.cpp \
	src/core/memory_allocator/MemoryAllocationUtil.cpp \
	src/core/memory_allocator/MemoryAllocationRequest.cpp \
	src/core/memory_allocator/LayoutStepCheckCurrentVolatileMode.h \
	src/core/memory_allocator/LayoutStep.cpp \
	src/core/memory_allocator/RuleMirroredAppDirectNotSupported.cpp \
	src/core/memory_allocator/RuleStorageCapacityNotSupported.h \
	src/core/memory_allocator/RuleReserveDimmPropertyInvalid.h \
	src/core/memory_allocator/RuleMemoryModeCapacityNotSupported.h \
	src/core/memory_allocator/LayoutStepCheckDriverSupportsStorage.cpp \
	src/core/memory_allocator/LayoutStepAppDirectSettingsNotRecommended.cpp \
	src/core/memory_allocator/MemoryAllocationRequest.h \
	src/core/memory_allocator/RuleDimmHasConfigGoal.h \
	src/core/memory_allocator/RuleNoDimms.h \
	src/core/memory_allocator/MemoryAllocationRequestBuilder.cpp \
	src/core/memory_allocator/LayoutStepCheckAsymmetricalPopulation.cpp \
	src/core/exceptions/InvalidArgumentException.cpp \
	src/core/exceptions/InvalidArgumentException.h \
	src/core/exceptions/LibraryException.h \
	src/core/exceptions/LibraryException.cpp \
	src/core/exceptions/NoMemoryException.cpp \
	src/core/exceptions/NoMemoryException.h \
	src/core/exceptions/NvmExceptionBadRequest.h \
	src/core/Helper.h \
	src/core/system/SystemMemoryResources.h \
	src/core/system/SoftwareInfo.h \
	src/core/system/SystemMemoryResources.cpp \
	src/core/system/SystemService.h \
	src/core/system/SystemInfo.cpp \
	src/core/system/SystemService.cpp \
	src/core/system/SystemInfo.h \
	src/core/system/SoftwareInfo.cpp \
	src/core/Helper.cpp \
	src/core/device/DeviceService.h \
	src/core/device/DeviceFirmwareService.cpp \
	src/core/device/DeviceFirmwareInfoCollection.h \
	src/core/device/TopologyService.h \
	src/core/device/Topology.cpp \
	src/core/device/Device.cpp \
	src/core/device/DeviceHelper.cpp \
	src/core/device/DeviceFirmwareInfoCollection.cpp \
	src/core/device/DeviceHelper.h \
	src/core/device/DeviceFirmwareInfo.h \
	src/core/device/DeviceFirmwareInfo.cpp \
	src/core/device/DeviceService.cpp \
	src/core/device/Device.h \
	src/core/device/TopologyService.cpp \
	src/core/device/DeviceFirmwareService.h \
	src/core/device/Topology.h \
	src/core/NvmCore.cpp \
	src/core/configuration/MemoryAllocationGoalService.cpp \
	src/core/configuration/MemoryAllocationGoalCollection.cpp \
	src/core/configuration/MemoryAllocationGoal.cpp \
	src/core/configuration/MemoryAllocationGoalService.h \
	src/core/configuration/MemoryAllocationGoal.h \
	src/core/configuration/MemoryAllocationGoalCollection.h \
	src/core/LibWrapper.h \
	src/core/firmware_interface/FwCommands.cpp \
	src/core/firmware_interface/FwCommandsWrapper.cpp \
	src/core/firmware_interface/FwCommandsWrapper.h \
	src/core/firmware_interface/FwCommands.h \
	src/driver_interface/passthrough.c \
	src/driver_interface/passthrough.h \
	src/firmware_interface/fis_commands.h \
	src/firmware_interface/fis_commands.c \
	src/firmware_interface/fis_parser.h \
	src/firmware_interface/fis_parser.c \
	src/firmware_interface/fw_commands.c \
	src/firmware_interface/fw_command_printer.h \
	src/firmware_interface/fw_command_printer.c \
	src/firmware_interface/fw_command_controller.h \
	src/firmware_interface/fw_command_controller.c \
	src/firmware_interface/fw_command_dump.h \
	src/firmware_interface/fw_command_dump.c \
	src/firmware_interface/fwcmd_args.h \
	src/firmware_interface/fwcmd_args.c

if LINUX
libixpdimm_core_la_SOURCES += \
	src/driver_interface/lnx_passthrough.c
endif
if WINDOWS
libixpdimm_core_la_SOURCES += \
	src/driver_interface/win_passthrough.c
endif

libixpdimm_core_la_DEPENDENCIES = \
	libixpdimm-common.la

libixpdimm_core_la_LDFLAGS = \
	-version-info 1:0:0 \
	libixpdimm-common.la

libixpdimm_cli_la_SOURCES = \
	src/cli/features/core/WbemToCli_utilities.h \
	src/cli/features/core/VerboseFeatureBase.h \
	src/cli/features/core/NamespaceFeature_Goals.cpp \
	src/cli/features/core/CreateGoalCommand.h \
	src/cli/features/core/SystemFeature.h \
	src/cli/features/core/ValidationFeature.cpp \
	src/cli/features/core/ShowVersionCommand.h \
	src/cli/features/core/ShowMemoryResourcesCommand.h \
	src/cli/features/core/ShowGoalCommand.cpp \
	src/cli/features/core/NamespaceFeature_Namespaces.cpp \
	src/cli/features/core/VerboseFeatureBase.cpp \
	src/cli/features/core/ShowCommandUtilities.cpp \
	src/cli/features/core/WbemToCli.h \
	src/cli/features/core/DimmProviderAdapter.cpp \
	src/cli/features/core/ShowDevicePcdCommand.h \
	src/cli/features/core/ShowTopologyCommand.cpp \
	src/cli/features/core/NamespaceFeature.h \
	src/cli/features/core/ShowLogCommand.h \
	src/cli/features/core/FieldSupportFeature.cpp \
	src/cli/features/core/ValidationFeature.h \
	src/cli/features/core/CreateGoalCommand.cpp \
	src/cli/features/core/ShowGoalCommand.h \
	src/cli/features/core/ShowHostServerCommand.h \
	src/cli/features/core/WbemToCli.cpp \
	src/cli/features/core/SensorFeature.h \
	src/cli/features/core/NvmCliCore.cpp \
	src/cli/features/core/framework/CliHelper.h \
	src/cli/features/core/framework/PropertyDefinitionList.cpp \
	src/cli/features/core/framework/CliHelper.cpp \
	src/cli/features/core/framework/CommandBase.h \
	src/cli/features/core/framework/UnitsOption.h \
	src/cli/features/core/framework/CommandBase.cpp \
	src/cli/features/core/framework/PropertyDefinitionList.h \
	src/cli/features/core/framework/ConsoleAdapter.cpp \
	src/cli/features/core/framework/DisplayOptions.cpp \
	src/cli/features/core/framework/PropertyDefinitionBase.h \
	src/cli/features/core/framework/ConsoleAdapter.h \
	src/cli/features/core/framework/PropertyDefinitionBase.cpp \
	src/cli/features/core/framework/YesNoPrompt.cpp \
	src/cli/features/core/framework/DisplayOptions.h \
	src/cli/features/core/framework/YesNoPrompt.h \
	src/cli/features/core/framework/UnitsOption.cpp \
	src/cli/features/core/ShowLogCommand.cpp \
	src/cli/features/core/NamespaceFeature.cpp \
	src/cli/features/core/SystemFeature.cpp \
	src/cli/features/core/ShowCommandPropertyUtilities.h \
	src/cli/features/core/MemoryProperty.cpp \
	src/cli/features/core/SimulatorFeature.cpp \
	src/cli/features/core/ShowDevicePcdCommand.cpp \
	src/cli/features/core/DimmProviderAdapter.h \
	src/cli/features/core/ShowHostServerCommand.cpp \
	src/cli/features/core/ShowDeviceCommand.cpp \
	src/cli/features/core/WbemToCli_utilities.cpp \
	src/cli/features/core/ShowTopologyCommand.h \
	src/cli/features/core/ShowDeviceCommand.h \
	src/cli/features/core/ShowMemoryResourcesCommand.cpp \
	src/cli/features/core/VerboseController.cpp \
	src/cli/features/core/NvmCliCore.h \
	src/cli/features/core/MemoryProperty.h \
	src/cli/features/core/ShowCommandUtilities.h \
	src/cli/features/core/FieldSupportFeature.h \
	src/cli/features/core/VerboseController.h \
	src/cli/features/core/CommandParts.h \
	src/cli/features/core/SensorFeature.cpp \
	src/cli/features/core/ShowVersionCommand.cpp \
	src/cli/features/core/SimulatorFeature.h \
	src/firmware_interface/fis_commands.h \
	src/firmware_interface/fw_commands.h

libixpdimm_cli_la_CFLAGS = \
	$(AM_CPPFLAGS) \
	-I $(top_srcdir)/src/wbem \
	-I $(top_srcdir)/src/acpi

libixpdimm_cli_la_CXXFLAGS = \
	$(AM_CPPFLAGS) \
	$(AM_CXXFLAGS) \
	-I $(top_srcdir)/src/wbem \
	-I $(top_srcdir)/src/acpi

libixpdimm_cli_la_LDFLAGS = \
	-version-info 1:0:0

libixpdimm_la_SOURCES = \
	external/fw_headers/fw_header.h \
	src/lib/adapter_types.h \
	src/lib/capabilities.c \
	src/lib/capabilities.h \
	src/lib/cleanup_support_events.c \
	src/lib/cleanup_support_events.h \
	src/lib/config_goal.c \
	src/lib/config_goal.h \
	src/lib/config_goal_utilities.c \
	src/lib/config_goal_utilities.h \
	src/lib/device_adapter.h \
	src/lib/device.c \
	src/lib/device_fw.c \
	src/lib/device_fw.h \
	src/lib/device_utilities.c \
	src/lib/device_utilities.h \
	src/lib/diagnostic.c \
	src/lib/diagnostic_fw.c \
	src/lib/diagnostic.h \
	src/lib/diagnostic_nfit.c \
	src/lib/diagnostic_platform_config_best_practices.c \
	src/lib/diagnostic_platform_config.c \
	src/lib/diagnostic_quick.c \
	src/lib/diagnostic_security.c \
	src/lib/error_injection.c \
	src/lib/event_field_metadata.c \
	src/lib/event_field_metadata.h \
	src/lib/fis_types.h \
	src/lib/job.c \
	src/lib/lib.c \
	src/lib/LogEnterExit.h \
	src/lib/monitor.c \
	src/lib/monitor.h \
	src/lib/namespace.c \
	src/lib/namespace_labels.c \
	src/lib/namespace_labels.h \
	src/lib/namespace_utils.c \
	src/lib/namespace_utils.h \
	src/lib/nfit_utilities.c \
	src/lib/nfit_utilities.h \
	src/lib/nvm_context.c \
	src/lib/nvm_context.h \
	src/lib/nvm_logging.c \
	src/lib/nvm_management.c \
	src/lib/nvm_management.h \
	src/lib/nvm_types.h \
	src/lib/platform_capabilities.c \
	src/lib/platform_capabilities_db.c \
	src/lib/platform_capabilities_db.h \
	src/lib/platform_capabilities.h \
	src/lib/platform_config_data.c \
	src/lib/platform_config_data_db.c \
	src/lib/platform_config_data_db.h \
	src/lib/platform_config_data.h \
	src/lib/pool.c \
	src/lib/pool_utilities.c \
	src/lib/pool_utilities.h \
	src/lib/save_state.c \
	src/lib/security.c \
	src/lib/sensor.c \
	src/lib/smbios_utilities.c \
	src/lib/smbios_utilities.h \
	src/lib/support.c \
	src/lib/support.h \
	src/lib/system.c \
	src/lib/system.h \
	src/lib/system_utilities.c \
	src/lib/system_utilities.h \
	src/lib/utility.h

if LINUX
libixpdimm_la_SOURCES += \
	src/lib/lnx_adapter.c \
	src/lib/lnx_adapter.h \
	src/lib/lnx_adapter_namespaces.c \
	src/lib/lnx_adapter_passthrough.c \
	src/lib/lnx_adapter_pools.c \
	src/lib/lnx_system.c
endif
if WINDOWS
libixpdimm_la_SOURCES += \
	src/lib/win_adapter.c \
	src/lib/win_leg_adapter.c \
	src/lib/win_leg_adapter.h \
	src/lib/win_leg_adapter_namespaces.c \
	src/lib/win_leg_adapter_passthrough.c \
	src/lib/win_leg_adapter_shared.c \
	src/lib/win_leg_adapter_shared.h \
	src/lib/win_scm2_adapter.c \
	src/lib/win_scm2_adapter.h \
	src/lib/win_scm2_capabilities.c \
	src/lib/win_scm2_capabilities.h \
	src/lib/win_scm2_ioctl.c \
	src/lib/win_scm2_ioctl_driver_version.c \
	src/lib/win_scm2_ioctl_driver_version.h \
	src/lib/win_scm2_ioctl.h \
	src/lib/win_scm2_ioctl_passthrough.c \
	src/lib/win_scm2_ioctl_passthrough.h \
	src/lib/win_scm2_namespaces.c \
	src/lib/win_scm2_passthrough.c \
	src/lib/win_scm2_passthrough.h \
	src/lib/win_scm2_version_info.c \
	src/lib/win_scm2_version_info.h \
	src/lib/win_system.c
endif

libixpdimm_la_CFLAGS = \
	$(AM_CPPFLAGS) \
	-I $(top_srcdir)/external/fw_headers \
	-I $(top_srcdir)/src/acpi

libixpdimm_la_DEPENDENCIES = \
	libixpdimm-common.la

libixpdimm_la_LDFLAGS = \
	-version-info 1:0:0 \
	libixpdimm-common.la

ixpdimm_monitor_SOURCES =

if LINUX
ixpdimm_monitor_SOURCES += \
	src/monitor/lnx_main.cpp
endif
if WINDOWS
ixpdimm_monitor_SOURCES += \
	src/monitor/win_service.cpp \
	src/monitor/win_main.cpp \
	src/monitor/win_service.h
endif

ixpdimm_monitor_CFLAGS = \
	$(SQLITE3_CFLAGS) \
	$(NDCTL_CFLAGS) \
	$(ZLIB_CFLAGS)

ixpdimm_monitor_LDADD = \
	libixpdimm-common.la \
	$(SQLITE3_LIBS) \
	$(NDCTL_LIBS) \
	$(ZLIB_LIBS) \
	-lixpdimm-core \
	-lixpdimm-cim \
	-lixpdimm \
	-lpthread \
	-ldl \
	-lnuma

ixpdimm_monitor_DEPENDENCIES = \
	libixpdimm-common.la \
	libixpdimm-core.la \
	libixpdimm-cim.la \
	libixpdimm.la

ixpdimm_cli_SOURCES = \
	src/cli/nvmcli.cpp

ixpdimm_cli_CXXFLAGS = \
	$(AM_CPPFLAGS) \
	$(AM_CXXFLAGS) \
	$(INVM_CLI_CFLAGS) \
	-I $(top_srcdir)/src/wbem

ixpdimm_cli_LDADD = \
	libixpdimm-common.la \
	$(INVM_CLI_LIBS) \
	$(SQLITE3_LIBS) \
	$(NDCTL_LIBS) \
	$(ZLIB_LIBS) \
	-lixpdimm-cli \
	-lixpdimm-core \
	-lixpdimm-cim \
	-lixpdimm \
	-lpthread \
	-ldl \
	-lnuma

ixpdimm_cli_DEPENDENCIES = \
	libixpdimm-common.la \
	libixpdimm-core.la \
	libixpdimm-cim.la \
	libixpdimm.la

ixpdimm_create_db_SOURCES = \
	./src/lib/create_config_db/main.c

ixpdimm_create_db_LDADD = \
	libixpdimm-common.la \
	$(SQLITE3_LIBS)

ixpdimm_create_db_DEPENDENCIES = \
	libixpdimm-common.la


EXTRA_DIST = \
	README.md \
	LICENSE

