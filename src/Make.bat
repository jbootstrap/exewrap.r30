@ECHO OFF

SETLOCAL
SET VCDIR=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC

IF "%1" == "clean" (
call "%VCDIR%\vcvarsall.bat"
nmake /nologo clean
IF ERRORLEVEL 1 GOTO ERR
goto SUCCESS
)

call "%VCDIR%\vcvarsall.bat" x86
nmake /nologo IMAGE_X86
IF ERRORLEVEL 1 GOTO ERR

call "%VCDIR%\vcvarsall.bat" x64
nmake /nologo IMAGE_X64
IF ERRORLEVEL 1 GOTO ERR

call "%VCDIR%\vcvarsall.bat" x86
nmake /nologo EXEWRAP_X86
nmake /nologo JREMIN_X86
IF ERRORLEVEL 1 GOTO ERR

call "%VCDIR%\vcvarsall.bat" x64
nmake /nologo EXEWRAP_X64
nmake /nologo JREMIN_X64
IF ERRORLEVEL 1 GOTO ERR

:SUCCESS
echo ����ɏI�����܂����B
goto END

:ERR
echo �G���[���������܂����B
goto END

:END

ENDLOCAL
