set CIM_LIB_SONAME=libixpdimm-cim
echo CIM_LIB_SONAME:%CIM_LIB_SONAME%
set ROOT=.
set BUILD_DIR=%1
set BUILD_SIM=%2
set NAMESPACES=interop root\interop root\PG_Interop

cd %BUILD_DIR% 
regsvr32 /s /u %CIM_LIB_SONAME%.dll
powershell gwmi -N:root -Query \"SELECT * from __Namespace WHERE Name = \'intelwbem\'\" "|" rwmi 
	
REM unregister from default namespace
for %%a in (%NAMESPACES%) do (
	powershell gwmi -namespace %%a -list 2>nul
	if %ERRORLEVEL% == 0 ( 
		mofcomp.exe -N:%%a profile_registration.mof
		powershell rwmi -N:%%a -class Intel_ElementConformsToProfile; \
		powershell rwmi -N:%%a -class intel_registeredprofile; \
			break; \
	)
)
	
REM might need to stop the WMI Service after registering the DLL and running a query against it.  
REM Unregistering the dll doesn't work most of the time.  The following command will stop the WMI service. (must be admin)
REM net stop "Windows Management Instrumentation"
