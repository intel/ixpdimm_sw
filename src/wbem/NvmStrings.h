/*
 * Copyright (c) 2015 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains common strings for the wbem and CLI libraries.
*/


#ifndef	_WBEM_FRAMEWORK_NVM_STRINGS_H_
#define	_WBEM_FRAMEWORK_NVM_STRINGS_H_

#include <string>
#include <cr_i18n.h>


#define	XSTR(x) #x			// needed for pulling string from make input
#define	STR(x) XSTR(x)		// needed for pulling string from make input

#define	NVM_WBEM_PREFIX   STR(__WBEM_PREFIX__) //!< Prefix for WBEM classes
#define	NVM_NAMESPACE_CSTR	"root/intelwbem"


namespace wbem
{

// legal
static std::string COPYRIGHT = "Copyright";
static std::string COPYRIGHT_NOTICE = COPYRIGHT + " (c) 2013 2014, Intel Corporation.";

// Wbem exception messages
static std::string EXCEPTION_BADTARGET_MSG = N_TR("'%s' is not a valid setting for the target '%s'.");
static std::string EXCEPTION_LIBERROR_MSG = N_TR("The Native API Library returned an unknown error code: %d.");
static std::string EXCEPTION_NOTMANAGEABLE_MSG = N_TR("The device '%s' is not manageable by the management software.");

// overall namespace name
static std::string NVM_NAMESPACE = NVM_NAMESPACE_CSTR;
static std::string NVM_INTEROP_NAMESPACE = "root/interop";

// misc common strings
static const std::string MB_SUFFIX = " MB";
static const std::string NA = "N/A";
static const std::string YES = "True";
static const std::string NO = "False";
static const std::string NONE = "None";
static const std::string UNKNOWN = "Unknown";

// some common constants
static const char CURRENT_CONFIG    = 'C';
static const char GOAL_CONFIG       = 'G';
static const char MEMORY_MODE_PREFIX = 'V';
static const char APP_DIRECT_PREFIX = 'P';
static const char STORAGE_PREFIX   	= 'U';

// Attribute keys that apply to many classes
static std::string CREATIONCLASSNAME_KEY = "CreationClassName";
static std::string NAME_KEY = "Name";
static std::string DESCRIPTION_KEY = "Description";
static std::string ELEMENTNAME_KEY = "ElementName";
static std::string INSTANCEID_KEY = "InstanceID";
static std::string SYSTEMCREATIONCLASSNAME_KEY = "SystemCreationClassName";
static std::string SYSTEMNAME_KEY = "SystemName";
static std::string DEVICEID_KEY = "DeviceID";
static std::string BASEUNITS_KEY = "BaseUnits";
static std::string ENABLEDSTATE_KEY = "EnabledState";
static std::string CURRENTSTATE_KEY = "CurrentState";
static std::string HEALTHSTATE_KEY = "HealthState";
static std::string COUNT_KEY = "Count";

// Associations
static std::string ANTECEDENT_KEY = "Antecedent";
static std::string DEPENDENT_KEY = "Dependent";

// BaseServer attribute keys
static std::string OSNAME_KEY = "OsName";
static std::string OSVERSION_KEY = "OsVersion";
static std::string LOGLEVEL_KEY = "LogLevel";
static std::string DEDICATED_KEY = "Dedicated";

// NVDIMM attribute keys
static std::string TAG_KEY = "Tag";
static std::string DIMMID_KEY = "DimmID"; // UID, handle or PID based on db selection
static std::string DIMMUID_KEY = "DimmUid";
static std::string DIMMHANDLE_KEY = "DimmHandle";
static std::string MANUFACTURER_KEY = "Manufacturer";
static std::string MANUFACTURERID_KEY = "ManufacturerID";
static std::string MODEL_KEY = "Model";
static std::string CAPACITY_KEY = "Capacity";
static std::string BANKLABEL_KEY = "BankLabel";
static std::string MANAGEABILITYSTATE_KEY = "ManageabilityState";
static std::string LOCKSTATE_KEY = "LockState";
static std::string SERIALNUMBER_KEY = "SerialNumber";
static std::string VENDORID_KEY = "VendorID";
static std::string REVISIONID_KEY = "RevisionID";
static std::string SOCKETID_KEY = "SocketID";
static std::string SUBSYSTEMVENDORID_KEY = "SubsystemVendorID";
static std::string SUBSYSTEMDEVICEID_KEY = "SubsystemDeviceID";
static std::string SUBSYSTEMREVISIONID_KEY = "SubsystemRevisionID";
static std::string MANUFACTURINGINFOVALID_KEY = "ManufacturingInfoValid";
static std::string MANUFACTURINGLOCATION_KEY = "ManufacturingLocation";
static std::string MANUFACTURINGDATE_KEY = "ManufacturingDate";
static std::string MEMORYCONTROLLERID_KEY = "MemoryControllerID";
static std::string MEMCONTROLLERID_KEY = "MemControllerID";
static std::string REMOVALCONDITIONS_KEY = "RemovalConditions";
static std::string MEMORYTYPE_KEY = "MemoryType";
static std::string PHYSICALID_KEY = "PhysicalID";
static std::string FORMFACTOR_KEY = "FormFactor";
static std::string DATAWIDTH_KEY = "DataWidth";
static std::string TOTALWIDTH_KEY = "TotalWidth";
static std::string SPEED_KEY = "Speed";
static std::string MEMORYCAPACITY_KEY = "MemoryCapacity";
static std::string APP_DIRECT_CAPACITY_KEY = "AppDirectCapacity";
static std::string PARTNUMBER_KEY = "PartNumber";
static std::string DEVICELOCATOR_KEY = "DeviceLocator";
static std::string COMMUNICATIONSTATUS_KEY = "CommunicationStatus";
static std::string ISNEW_KEY = "IsNew";
static std::string FWLOGLEVEL_KEY = "FWLogLevel";
static std::string FWAPIVERSION_KEY = "FWAPIVersion";
static std::string FWVERSION_KEY = "FWVersion";
static std::string INACCESSIBLE_APP_DIRECT_CAPACITY_KEY = "InaccessibleAppDirectCapacity";
static std::string UNCONFIGUREDCAPACITY_KEY = "UnconfiguredCapacity";
static std::string POWERMANAGEMENTENABLED_KEY = "PowerManagementEnabled";
static std::string POWERLIMIT_KEY = "PowerLimit";
static std::string PEAKPOWERBUDGET_KEY = "PeakPowerBudget";
static std::string AVGPOWERBUDGET_KEY = "AvgPowerBudget";
static std::string DIESPARINGENABLED_KEY = "DieSparingEnabled";
static std::string DIESPARINGLEVEL_KEY = "DieSparingLevel";
static std::string LASTSHUTDOWNSTATUS_KEY = "LastShutdownStatus";
static std::string DIESPARESUSED_KEY = "DieSparesUsed";
static std::string FIRSTFASTREFRESH_KEY = "FirstFastRefresh";
static std::string CHANNEL_KEY = "ChannelID";
static std::string CHANNELPOS_KEY = "ChannelPos";
static std::string NODECONTROLLERID_KEY = "NodeControllerID";
static std::string CONFIGURATIONSTATUS_KEY = "ConfigurationStatus";
static std::string ARSSTATUS_KEY = "ARSStatus";
static std::string SECURITYCAPABILITIES_KEY = "SecurityCapabilities";
static std::string LASTSHUTDOWNTIME_KEY = "LastShutdownTime";
static std::string MEMORYTYPECAPABILITIES_KEY = "MemoryTypeCapabilities";
static std::string DIESPARINGCAPABLE_KEY = "DieSparingCapable";
static std::string MIXEDSKU_KEY = "MixedSKU";
static std::string SKUVIOLATION_KEY = "SKUViolation";
static std::string VIRALPOLICY_KEY = "ViralPolicy";
static std::string VIRALSTATE_KEY = "ViralState";

// RawMemory attribute keys
static std::string BLOCKSIZE_KEY = "BlockSize";
static std::string NUMBEROFBLOCKS_KEY = "NumberOfBlocks";
static std::string HEALTHSTATUS_KEY = "HealthStatus";

// MemoryController attribute keys
static std::string PROTOCOLSUPPORTED_KEY = "ProtocolSupported";

// SystemProcessor attribute keys
static std::string FAMILY_KEY = "Family";
static std::string OTHERFAMILYDESCRIPTION_KEY = "OtherFamilyDescription";
static std::string STEPPING_KEY = "Stepping";
static std::string NUMBEROFLOGICALPROCESSORS_KEY = "NumberOfLogicalProcessors";
static std::string TYPE_KEY = "Type";

// ConformsToProfile attribute keys
static std::string REGISTEREDNAME_KEY = "RegisteredName";
static std::string MANAGEDELEMENT_KEY = "ManagedElement";

// RegisteredProfile attribute keys
static std::string REGISTEREDORGANIZATION_KEY = "RegisteredOrganization";
static std::string OTHERREGISTEREDORGANIZATION_KEY = "OtherRegisteredOrganization";
static std::string REGISTEREDVERSION_KEY = "RegisteredVersion";
static std::string ADVERTISETYPES_KEY = "AdvertiseTypes";

// Version attribute keys
static std::string MAJORVERSION_KEY = "MajorVersion";
static std::string MINORVERSION_KEY = "MinorVersion";
static std::string REVISIONNUMBER_KEY = "RevisionNumber";
static std::string BUILDNUMBER_KEY = "BuildNumber";
static std::string VERSIONSTRING_KEY = "VersionString";
static std::string CLASSIFICATIONS_KEY = "Classifications";
static std::string SPECIFICATION_KEY = "Specification";
static std::string ISENTITY_KEY = "IsEntity";
static std::string FWTYPE_KEY = "FWType";
static std::string COMMITID_KEY = "CommitID";
static std::string BUILDCONFIGURATION_KEY = "BuildConfiguration";

// NumericSensorFactory Attribute keys
static std::string SENSORTYPE_KEY = "SensorType";
static std::string CURRENTREADING_KEY = "CurrentReading";
static std::string UNITMODIFIER_KEY = "UnitModifier";
static std::string OTHERSENSORTYPEDESCRIPTION_KEY = "OtherSensorTypeDescription";
static std::string LOWERTHRESHOLDNONCRITICAL_KEY = "LowerThresholdNonCritical";
static std::string UPPERTHRESHOLDNONCRITICAL_KEY = "UpperThresholdNonCritical";
static std::string LOWERTHRESHOLDCRITICAL_KEY = "LowerThresholdCritical";
static std::string UPPERTHRESHOLDCRITICAL_KEY = "UpperThresholdCritical";
static std::string LOWERTHRESHOLDFATAL_KEY = "LowerThresholdFatal";
static std::string UPPERTHRESHOLDFATAL_KEY = "UpperThresholdFatal";
static std::string SETTABLETHRESHOLDS_KEY = "SettableThresholds";
static std::string SUPPORTEDTHRESHOLDS_KEY = "SupportedThresholds";
static std::string POSSIBLESTATES_KEY = "PossibleStates";

// Erasure Capabilities attribute keys
static std::string ERASUREMETHODS_KEY = "ErasureMethods";
static std::string DEFAULTERASUREMETHOD_KEY = "DefaultErasureMethod";
static std::string CANERASEONRETURNTOSTORAGEPOOL_KEY = "CanEraseOnReturnToStoragePool";
static std::string ELEMENTTYPESSUPPORTED_KEY = "ElementTypesSupported";

// Metrics
const static std::string ID_KEY = "Id";
const static std::string UNITS_KEY = "Units";
const static std::string DATATYPE_KEY = "DataType";
const static std::string ISCONTINUOUS_KEY = "IsContinuous";
const static std::string TIMESCOPE_KEY = "TimeScope";
const static std::string MEASUREDELEMENTNAME_KEY = "MeasuredElementName";
const static std::string METRICVALUE_KEY = "MetricValue";
const static std::string METRICDEFINITION_ID_KEY = "MetricDefinitionId";
const static std::string MEASUREDELEMENT_NAME_KEY = "MeasuredElementName";
const static std::string BYTESREAD_KEY = "BytesRead";
const static std::string BYTESWRITTEN_KEY = "BytesWritten";
const static std::string HOSTWRITECOMMANDS_KEY = "HostWrites";
const static std::string HOSTREADREQUESTS_KEY = "HostReads";
const static std::string BLOCKWRITECOMMANDS_KEY = "BlockWrites";
const static std::string BLOCKREADREQUESTS_KEY = "BlockReads";

// MemoryConfigurationCapabilities
static std::string SUPPORTEDSYNCHRONOUSOPERATIONS_KEY = "SupportedSynchronousOperations";
static std::string SUPPORTEDASYNCHRONOUSOPERATIONS_KEY = "SupportedAsynchronousOperations";

// MemoryCapabilities
static std::string MEMORYMODES_KEY = "MemoryModes";
static std::string REPLICATIONSUPPORT_KEY = "ReplicationSupport";
static std::string RELIABILITYSUPPORT_KEY = "ReliabilitySupport";
static std::string ALIGNMENT_KEY = "Alignment";
static std::string CHANNELINTERLEAVESUPPORT_KEY = "ChannelInterleaveSupport";
static std::string CHANNELINTERLEAVEWAYSUPPORT_KEY = "ChannelInterleaveWaySupport";
static std::string MEMORYCONTROLLERINTERLEAVESUPPORT_KEY = "MemoryControllerInterleaveSupport";
static std::string MEMORYMODEALIGNMENT_KEY = "MemoryModeAlignment";
static std::string APPDIRECTALIGNMENT_KEY = "AppDirectAlignment";
static std::string PLATFORMCONFIGSUPPORTED_KEY = "PlatformConfigSupported";
static std::string PLATFORMRUNTIMESUPPORTED_KEY = "PlatformRuntimeSupported";
static std::string CURRENTVOLATILEMODE_KEY = "CurrentVolatileMode";
static std::string CURRENTAPPDIRECTMODE_KEY = "CurrentAppDirectMode";
static std::string SUPPORTEDAPP_DIRECT_SETTINGS_KEY = "SupportedAppDirectSettings";
static std::string RECOMMENDEDAPP_DIRECT_SETTINGS_KEY = "RecommendedAppDirectSettings";

// MemoryResources
static std::string PRIMORDIAL_KEY = "Primordial";
static std::string RESOURCETYPE_KEY = "ResourceType";
static std::string RESERVED_KEY = "Reserved";
static std::string INACCESSIBLECAPACITY_KEY = "InaccessibleCapacity";
static std::string RESERVEDCAPACITY_KEY = "ReservedCapacity";

// FWEventLog
static std::string MAXNUMBEROFRECORDS_KEY = "MaxNumberOfRecords";
static std::string CURRENTNUMBEROFRECORDS_KEY = "CurrentNumberOfRecords";
static std::string OVERWRITEPOLICY_KEY = "OverwritePolicy";

// AppDirectMemoryPool
static std::string POOLID_KEY = "PoolID";
static std::string TOTALMANAGEDSPACE_KEY = "TotalManagedSpace";
static std::string REMAININGMANAGEDSPACE_KEY = "RemainingManagedSpace";

// PoolView
static std::string POOLTYPE_KEY = "PoolType";
static std::string FREECAPACITY_KEY = "FreeCapacity";
static std::string ENCRYPTIONCAPABLE_KEY = "EncryptionCapable";
static std::string ENCRYPTIONENABLED_KEY = "EncryptionEnabled";
static std::string ERASECAPABLE_KEY = "EraseCapable";
static std::string APPDIRECTNAMESPACE_MAX_SIZE_KEY = "AppDirectNamespaceMaxSize";
static std::string APPDIRECTNAMESPACE_MIN_SIZE_KEY = "AppDirectNamespaceMinSize";
static std::string APPDIRECTNAMESPACE_COUNT_KEY = "AppDirectNamespaceCount";
static std::string STORAGENAMESPACE_MAX_SIZE_KEY = "StorageNamespaceMaxSize";
static std::string STORAGENAMESPACE_MIN_SIZE_KEY = "StorageNamespaceMinSize";
static std::string STORAGENAMESPACE_COUNT_KEY = "StorageNamespaceCount";
static std::string APP_DIRECT_SETTINGS_KEY = "AppDirectSettings";

// VolatileMemory
static std::string VOLATILE_KEY = "Volatile";
static std::string MEMORYSIZE_KEY = "MemorySize";

// MemoryPoolConfigurationServiceFactory
static std::string INTERLEAVESIZES_KEY = "InterleaveSizes";
static std::string PACKAGEREDUNDANCY_KEY = "PackageRedundancy";

// MemoryConfiguration
static std::string CHANGEABLETYPE_KEY = "ChangeableType";
static std::string STATUS_KEY = "Status";

// MemoryAllocationSetting
static std::string PARENT_KEY = "Parent";
static std::string CHANNELINTERLEAVESIZE_KEY = "ChannelInterleaveSize";
static std::string CHANNELCOUNT_KEY = "ChannelCount";
static std::string CONTROLLERINTERLEAVESIZE_KEY = "ControllerInterleaveSize";
static std::string NEWMEMORYONLY_KEY = "NewMemoryOnly";

// CLI only
static std::string INTERLEAVEINDEXES_KEY = "InterleaveIndexes";
static std::string INTERLEAVEFORMATS_KEY = "InterleaveFormats";

// MemoryAllocationSettings for CLI
static std::string APPDIRECT1SIZE_KEY = "AppDirect1Size";
static std::string APPDIRECT1INDEX_KEY = "AppDirect1Index";
static std::string APPDIRECT2SIZE_KEY = "AppDirect2Size";
static std::string APPDIRECT2INDEX_KEY = "AppDirect2Index";
static std::string APPDIRECT1SETTINGS_KEY = "AppDirect1Settings";
static std::string APPDIRECT2SETTINGS_KEY = "AppDirect2Settings";
static std::string STORAGECAPACITY_KEY = "StorageCapacity";

// DiagnosticCompletionRecord
static std::string SERVICENAME_KEY = "ServiceName";
static std::string MANAGEDELEMENTNAME_KEY = "ManagedElementName";
static std::string CREATIONTIMESTAMP_KEY = "CreationTimeStamp";
static std::string ERRORCODE_KEY = "ErrorCode";
static std::string COMPLETIONSTATE_KEY = "CompletionState";

// SanitizeJob
static std::string OPERATIONALSTATUS_KEY = "OperationalStatus"; // also used by RawMemory
static std::string JOBSTATE_KEY = "JobState";
static std::string PERCENTCOMPLETE_KEY = "PercentComplete";
static std::string DELETEONCOMPLETION_KEY = "DeleteOnCompletion";
static std::string TIMEBEFOREREMOVAL_KEY = "TimeBeforeRemoval";
static std::string NVM_JOB_TYPE_SANITIZE_NAME = "Sanitize Job";
static std::string NVM_STATUS_OK = "OK";
static std::string JOBTABLENAME = "JobTable";

// NVDIMMLogEntry
static std::string PERCEIVEDSEVERITY_KEY = "PerceivedSeverity";
static std::string LOGINSTANCEID_KEY = "LogInstanceID";
static std::string MESSAGEID_KEY = "MessageID";
static std::string MESSAGE_KEY = "Message";
static std::string MESSAGEARGS_KEY = "MessageArguments";
static std::string ACTIONREQUIRED_KEY = "ActionRequired";

// NamespaceSettings
static std::string NAMESPACEID_KEY = "NamespaceID";
static std::string ALLOCATIONUNITS_KEY = "AllocationUnits";
static std::string RESERVATION_KEY = "Reservation";
static std::string OPTIMIZE_KEY = "Optimize";
static std::string SECURITYGOAL_KEY = "SecurityGoal";
static std::string CONFIGGOALTABLENAME = "ConfigGoal";
static std::string DELETECONFIGGOALTABLENAME = "DeleteConfigGoalTable";
static std::string INITIALSTATE_KEY = "InitialState";

// AppDirectMemoryCapabilities
static std::string MAXNAMESPACES_KEY = "MaxNamespaces";
static std::string SECURITYFEATURES_KEY = "SecurityFeatures";
static std::string ACCESSGRANULARITY_KEY = "AccessGranularity";
static std::string MEMORYARCHITECTURE_KEY = "MemoryArchitecture";
static std::string REPLICATION_KEY = "Replication";

// Events
static std::string ACTIONREQUIREDEVENTS_KEY = "ActionRequiredEvents";

// AppDirectMemory
static std::string PROCESSORAFFINITY_KEY = "ProcessorAffinity";

// Indications
static std::string TIMECREATED_KEY = "TIME_CREATED";
static std::string TARGETINSTANCE_KEY = "TargetInstance";
static std::string CHANGEDPROPERTYNAMES_KEY = "ChangedPropertyNames";
static std::string PREVIOUSINSTANCE_KEY = "PreviousInstance";
static std::string INDICATIONTIME_KEY = "IndicationTime";
static std::string SOURCEINSTANCEMODELPATH_KEY = "SourceInstanceModelPath";
static std::string SOURCEINSTANCE_KEY = "SourceInstance";
static std::string SOURCENAMESPACE_KEY = "SourceNamespace";
static std::string QUERYLANGUAGE_KEY = "QueryLanguage";
static std::string QUERY_KEY = "Query";
static std::string INDIVIDUALSUBSCRIPTIONSUPPORTED_KEY = "IndividualSubscriptionSupported";

// NVDIMMEvent Indication
static std::string ALERTINGELEMENTFORMAT_KEY = "AlertingElementFormat";
static std::string ALERTTYPE_KEY = "AlertType";
static std::string ALERTINGMANAGEDELEMENT_KEY = "AlertingManagedElement";

// SystemCapabilities
static std::string MEMORYMODESSUPPORTED_KEY = "ModesSupported";
static std::string MINNAMESPACESIZE_KEY = "MinNamespaceSize";
static std::string BLOCKSIZES_KEY = "BlockSizes";
static std::string APP_DIRECT_MEMORY_MIRROR_SUPPORT_KEY = "AppDirectMirrorSupported";
static std::string DIMMSPARESUPPORT_KEY = "DimmSpareSupported";
static std::string APP_DIRECT_MEMORY_MIGRATION_SUPPORT_KEY = "AppDirectMigrationSupported";
static std::string RENAMENAMESPACESUPPORT_KEY = "RenameNamespaceSupported";
static std::string ENABLENAMESPACESUPPORT_KEY = "EnableNamespaceSupported";
static std::string DISABLENAMESPACESUPPORT_KEY = "DisableNamespaceSupported";
static std::string GROWAPPDIRECTNAMESPACESUPPORT_KEY = "GrowAppDirectNamespaceSupported";
static std::string SHRINKAPPDIRECTNAMESPACESUPPORT_KEY = "ShrinkAppDirectNamespaceSupported";
static std::string GROWSTORAGENAMESPACESUPPORT_KEY = "GrowStorageNamespaceSupported";
static std::string SHRINKSTORAGENAMESPACESUPPORT_KEY = "ShrinkStorageNamespaceSupported";
static std::string INITIATESCRUBSUPPORT_KEY = "InitiateScrubSupported";
static std::string MEMORYPAGEALLOCATIONCAPABLE_KEY = "MemoryPageAllocationCapable";
static std::string MEMORYPAGEALLOCATION_KEY = "MemoryPageAllocation";

// HealthSensorView
static std::string MEDIACURRENTTEMPERATURE_KEY = "CurrentMediaTemperature";
static std::string MEDIATEMPERATUREUNITS_KEY = "MediaTemperatureUnits";
static std::string MEDIATEMPERATUREUNITMODIFIER_KEY = "MediaTemperatureUnitModifier";
static std::string MEDIATEMPERATURE_UPPERNONCRITICAL_THRESHOLD_KEY = "MediaTemperatureUpperNonCriticalThreshold";
static std::string MEDIATEMPERATURE_LOWERCRITICAL_THRESHOLD_KEY = "MediaTemperatureLowerCriticalThreshold";
static std::string MEDIATEMPERATURE_UPPERCRITICAL_THRESHOLD_KEY = "MediaTemperatureUpperCriticalThreshold";
static std::string MEDIATEMPERATURE_UPPERFATAL_THRESHOLD_KEY = "MediaTemperatureUpperFatalThreshold";
static std::string MEDIATEMPERATURESTATE_KEY = "MediaTemperatureState";
static std::string CURRENTSPARELEVEL_KEY = "CurrentSpareLevel";
static std::string SPAREUNITS_KEY = "SpareUnits";
static std::string SPAREUNITMODIFIER_KEY = "SpareUnitModifier";
static std::string SPARETHRESHOLD_KEY = "SpareThreshold";
static std::string SPARESTATE_KEY = "SpareState";
static std::string CURRENTWEARLEVEL_KEY = "CurrentWearLevel";
static std::string WEARUNITS_KEY = "WearUnits";
static std::string WEARUNITMODIFIER_KEY = "WearUnitModifier";
static std::string WEARSTATE_KEY = "WearState";
static std::string POWERONTIME_KEY = "PowerOnTime";
static std::string POWERONUNITS_KEY = "PowerOnUnits";
static std::string POWERONMODIFIER_KEY = "PowerOnModifier";
static std::string POWERONSTATE_KEY = "PowerOnState";
static std::string UPTIME_KEY = "UpTime";
static std::string UPTIMEUNITS_KEY = "UpTimeUnits";
static std::string UPTIMEMODIFIER_KEY = "UpTimeModifier";
static std::string UPTIMESTATE_KEY = "UpTimeState";
static std::string POWERCYCLECOUNT_KEY = "PowerCycleCount";
static std::string POWERCYCLEUNITS_KEY = "PowerCycleUnits";
static std::string POWERCYCLEMODIFIER_KEY = "PowerCycleModifier";
static std::string POWERCYCLESTATE_KEY = "PowerCycleState";
static std::string UNSAFESHUTDOWNS_KEY = "UnsafeShutdowns";
static std::string UNSAFESHUTDOWNUNITS_KEY = "UnsafeShutdownUnits";
static std::string UNSAFESHUTDOWNMODIFIER_KEY = "UnsafeShutdownModifier";
static std::string UNSAFESHUTDOWNSTATE_KEY = "UnsafeShutdownState";
static std::string MEDIAERRORSUNCORRECTABLE_KEY = "MediaErrorsUncorrectable";
static std::string MEDIAERRORSUNCORRECTABLEUNITS_KEY = "MediaErrorsUncorrectableUnits";
static std::string MEDIAERRORSUNCORRECTABLEMODIFIER_KEY = "MediaErrorsUncorrectableModifier";
static std::string MEDIAERRORSUNCORRECTABLESTATE_KEY = "MediaErrorsUncorrectableState";
static std::string POWERLIMITED_KEY = "PowerLimited";
static std::string POWERLIMITEDUNITS_KEY = "PowerLimitedUnits";
static std::string POWERLIMITEDMODIFIER_KEY = "PowerLimitedModifier";
static std::string POWERLIMITEDSTATE_KEY = "PowerLimitedState";
static std::string MEDIAERRORSCORRECTED_KEY = "MediaErrorsCorrected";
static std::string MEDIAERRORSCORRECTEDUNITS_KEY = "MediaErrorsCorrectedUnits";
static std::string MEDIAERRORSCORRECTEDMODIFIER_KEY = "MediaErrorsCorrectedModifier";
static std::string MEDIAERRORSCORRECTEDSTATE_KEY = "MediaErrorsCorrectedState";
static std::string CONTROLLERCURRENTTEMPERATURE_KEY = "CurrentControllerTemperature";
static std::string CONTROLLERTEMPERATUREUNITS_KEY = "ControllerTemperatureUnits";
static std::string CONTROLLERTEMPERATUREUNITMODIFIER_KEY = "ControllerTemperatureUnitModifier";
static std::string CONTROLLERTEMPERATURE_NONCRITICAL_THRESHOLD_KEY = "ControllerTemperatureNonCriticalThreshold";
static std::string CONTROLLERTEMPERATURE_FATAL_THRESHOLD_KEY = "ControllerTemperatureFatalThreshold";
static std::string CONTROLLERTEMPERATURESTATE_KEY = "ControllerTemperatureState";

// Namespace
static std::string BLOCKCOUNT_KEY = "BlockCount";
static std::string ENABLED_KEY = "Enabled";

// Firmware
static std::string ACTIVEFWVERSION_KEY = "ActiveFWVersion";
static std::string ACTIVEFWTYPE_KEY = "ActiveFWType";
static std::string ACTIVEFWCOMMITID_KEY = "ActiveFWCommitID";
static std::string ACTIVEBUILDCONFIGURATION_KEY = "ActiveFWBuildConfiguration";
static std::string STAGEDFWVERSION_KEY = "StagedFWVersion";
static std::string STAGEDFWTYPE_KEY = "StagedFWType";

// ElementSoftwareIdentity
static std::string ELEMENTSOFTWARESTATUS_KEY = "ElementSoftwareStatus";

} // wbem
#endif // _WBEM_FRAMEWORK_NVM_STRINGS_H_
