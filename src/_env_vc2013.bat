set SCRIPT=%~0
for /f "delims=\ tokens=*" %%z in ("%SCRIPT%") do (
  set SCRIPT_DRIVE=%%~dz
  set SCRIPT_PATH=%%~pz
  set SCRIPT_CURRENT_DIR=%%~dpz
)

SETLOCAL

SET VCDIR=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC
call "%VCDIR%\vcvarsall.bat" x86
rem call "%VCDIR%\vcvarsall.bat" x64

cd /d %SCRIPT_CURRENT_DIR%
rmdir /s /q build_vc2013
mkdir build_vc2013
cd build_vc2013
cmake -G "Visual Studio 12 2013" -DCMAKE_CXX_FLAGS_RELEASE="/MT" -DCMAKE_CXX_FLAGS_DEBUG="/MTd" ..

:SUCCESS
echo 正常に終了しました。
goto END

:ERR
echo エラーが発生しました。
goto END

:END

ENDLOCAL

PAUSE
