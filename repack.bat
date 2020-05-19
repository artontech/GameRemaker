@ECHO OFF
cd /d %~dp0
"build\Debug\GameRemaker.exe" repack -i %1 -o ./data2.win
pause