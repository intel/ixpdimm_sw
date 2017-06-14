REM -----------------------------
REM Create registration mofs
REM -----------------------------

set ROOT=%1
set BUILD_DIR=%2
set WBEM_PREFIX_INPUT=Intel_

echo ROOT: %ROOT%
echo BUILD_DIR: %BUILD_DIR%

del %BUILD_DIR%\profile_registration.mof 2>nul
copy "%ROOT%\src\wbem\mof\profile_registration.mof"  "%BUILD_DIR%\profile_registration.mof"

setlocal enabledelayedexpansion
for /F "tokens=*" %%i in (%BUILD_DIR%\profile_registration.mof) do (
		set str=%%i
		set str=!str:^<NVM_WBEM_PREFIX^>=%WBEM_PREFIX_INPUT%!
		echo !str! >> %BUILD_DIR%\profile_registration_new.mof
		)
move /Y %BUILD_DIR%\profile_registration_new.mof %BUILD_DIR%\profile_registration.mof

del %BUILD_DIR%\register.mof 2>nul
copy "%ROOT%\src\wbem\cimom\wmi\register.mof"  "%BUILD_DIR%\register.mof"

setlocal enabledelayedexpansion
for /F "tokens=*" %%i in (%BUILD_DIR%\register.mof) do (
		set str=%%i
		set str=!str:^<NVM_WBEM_PREFIX^>=%WBEM_PREFIX_INPUT%!
		echo !str! >> %BUILD_DIR%\register_new.mof
		)
move /Y %BUILD_DIR%\register_new.mof %BUILD_DIR%\register.mof

REM -----------------------------
REM Create class definition mofs
REM -----------------------------

REM DMTF CIM Schema
del %BUILD_DIR%\intelwbem.mof 2>nul
echo // DMTF CIM Schema 2.44.1 > %BUILD_DIR%\intelwbem.mof
type "%ROOT%\src\wbem\mof\cim_schema_2.44.1_combined.mof" >> "%BUILD_DIR%\intelwbem.mof"

REM SNIA CIM Schema

echo // SNIA CIM Schema 16Rev4-Updated >> %BUILD_DIR%\intelwbem.mof	
type "%ROOT%\src\wbem\mof\snia_mofs_16Rev4-updated.mof" >> "%BUILD_DIR%\intelwbem.mof"
REM echo >> %BUILD_DIR%\intelwbem.mof

REM Intel CIM Schema
echo // Intel CIM Schema >> %BUILD_DIR%\intelwbem.mof
setlocal enabledelayedexpansion
for /F "tokens=*" %%i in (%ROOT%\src\wbem\mof\class_def.mof) do (
 		set str=%%i
 		set str=!str:^<NVM_WBEM_PREFIX^>=%WBEM_PREFIX_INPUT%!
 		REM get rid of CMPI Specifics
 		set str=!str:^<CMPI^>CIM_AlertIndication^<^/CMPI^>=!
 		REM Keep WMI Specifics
 		set str=!str:^<WMI^>=!
 		set str=!str:^<^/WMI^>=!
 		
 		echo !str! >> %ROOT%\src\wbem\mof\class_def_new.mof
 		)
type "%ROOT%\src\wbem\mof\class_def_new.mof" >> "%BUILD_DIR%\intelwbem.mof"
del "%ROOT%\src\wbem\mof\class_def_new.mof"

del "%BUILD_DIR%\intelwbem.mof.bak" 2>nul

REM add qualifiers for linux/windows
type "%ROOT%\src\wbem\mof\qualifiers.mof" >> "%BUILD_DIR%\intelwbem_with_qualifiers.mof"
type "%BUILD_DIR%\intelwbem.mof" >> "%BUILD_DIR%\intelwbem_with_qualifiers.mof"
move /Y %BUILD_DIR%\intelwbem_with_qualifiers.mof  %BUILD_DIR%\intelwbem.mof
