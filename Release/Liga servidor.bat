@echo off

rem Defina o diretório de trabalho atual para o local do arquivo .bat
cd /d "%~dp0"

rem Defina o diretório de trabalho para DataServer e inicie DataServer.exe
cd "DataServer"
start "DataServer" "DataServer.exe"

rem Defina o diretório de trabalho para DataServer e inicie GameServer.exe
cd "..\GameServer"
start "Game Server" "GameServer.exe"

echo Iniciado com sucesso!
exit
