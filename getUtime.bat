@echo off
echo WScript.Echo(new Date().getTime()); > %temp%\time.js
FOR /F %%A IN ('cscript //nologo %temp%\time.js') DO SET BUILD_TIMESTAMP=%%A
REM echo #define BUILD_TIME %BUILD_TIMESTAMP:~0,-3% > "%1\timestamp.h"
echo #define BUILD_TIME %BUILD_TIMESTAMP:~0,-3% > "timestamp.h"
del %temp%\time.js
REM @echo on
echo _
echo +++++++++++++++++++++++++
echo + BUILD_TIME %BUILD_TIMESTAMP:~0,-3% +
echo +++++++++++++++++++++++++
echo _
