@echo off
rem ----------------------------------------------------------------------------

rem ---- Test Configuration Name
if %1 == Debug          goto CONFIGNAME_OK
if %1 == Release        goto CONFIGNAME_OK
if %1 == RelWithDebInfo goto CONFIGNAME_OK
echo Error during PreBuild step: Invalid Configuration - %1
goto END
:CONFIGNAME_OK

rem ---------------------------------------
rem ---- CONFIGURATIONS
rem ---------------------------------------

set DOT_NET_INSTALL_PATH=%~2
set FREEEMG_SDK_DIR=%~3
set Device_DIR=%~4
set PLATFORM=%~5

echo DOT_NET_INSTALL_PATH %DOT_NET_INSTALL_PATH%
echo FREEEMG_SDK_DIR %FREEEMG_SDK_DIR%
echo Device_DIR %Device_DIR%
echo PLATFORM %PLATFORM%

set TARGET_ROOT=%PROJECT_ROOT%..
set IMPORT_ROOT=%FREEEMG_SDK_DIR%\Bin

set IMPORT_NAME1=bts.biodaq.drivers
set IMPORT_NAME2=bts.biodaq.core

rem ---------------------------------------
rem ---- FOLDERS SETUP
rem ---------------------------------------

if %1 == Debug          set CONFIG_FOLDER=Debug
if %1 == Release        set CONFIG_FOLDER=Release
if %1 == RelWithDebInfo set CONFIG_FOLDER=Release

set SOURCE_FOLDER=%Device_DIR%\src
set IMPORT_FOLDER=%IMPORT_ROOT%\%CONFIG_FOLDER%\%PLATFORM%
set REGASM_FOLDER=%DOT_NET_INSTALL_PATH%

rem ---------------------------------------
rem ---- ALL CONFIGURATIONS
rem ---------------------------------------

echo Registering BioDaq libraries %IMPORT_NAME1%.dll and %IMPORT_NAME2%.dll
%DOT_NET_INSTALL_PATH%\RegAsm /codebase "%IMPORT_FOLDER%\%IMPORT_NAME1%.dll" /tlb: "%SOURCE_FOLDER%\%IMPORT_NAME1%.tlb"
%DOT_NET_INSTALL_PATH%\RegAsm /codebase "%IMPORT_FOLDER%\%IMPORT_NAME2%.dll" /tlb: "%SOURCE_FOLDER%\%IMPORT_NAME2%.tlb"

echo Copying type library mscorlib.tlb
call :FCOPY "%IMPORT_FOLDER%\mscorlib.tlb"            "%SOURCE_FOLDER%"

goto END

rem ---------------------------------------
rem ---- SUBROUTINES
rem ----

:FCOPY

echo Copying %1 to %2
copy %1 %2 > Nul
if ERRORLEVEL 1 echo Error copying %1 to %2 
goto :EOF 


:END
rem ---------------------------------------
rem                END
rem ---------------------------------------

echo PreBuild done.
