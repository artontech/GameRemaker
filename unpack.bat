@ECHO OFF
cd /d %~dp0
".\build\Debug\GameRemaker.exe" unpack -i %1
pause