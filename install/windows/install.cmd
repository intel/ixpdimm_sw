REM setup for install
set CIM_LIB_SONAME=libixpdimm-cim
echo CIM_LIB_SONAME:%CIM_LIB_SONAME%
set ROOT=.
set BUILD_DIR=%1
set BUILD_SIM=%2
set NAMESPACES=interop root\interop root\PG_Interop

cd %BUILD_DIR% 
REM register the DLL
regsvr32 /s %CIM_LIB_SONAME%.dll 
REM compile and load the MOF file
mofcomp -N:root\intelwbem intelwbem.mof
REM registration MOF file
mofcomp -N:root\intelwbem register.mof 

REM register with default namespace
for %%a in (%NAMESPACES%) do (
	powershell gwmi -namespace %%a -list 2>nul
	if %ERRORLEVEL% == 0 ( 
		mofcomp.exe -N:%%a profile_registration.mof
	)
)
if /I "%BUILD_SIM%" EQU "1"
	REM Setup simulator
	del cim_system.db; create_simulator.exe --path `pwd`; move apss.dat.smoke apss.dat
endif
