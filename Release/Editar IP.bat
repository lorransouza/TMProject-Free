@echo off
setlocal enabledelayedexpansion

set "novoIP=127.0.0.1"
set "serveripPorta=8281"
set "dataserverPorta=7514"

rem Obtenha o diretório onde o .bat está localizado
for %%i in ("%~dp0") do set "pastaAtual=%%~fi"

rem Alterar o serverlist.txt
(
    echo 0 0 %novoIP%
    echo 0 1 %novoIP%
    echo 1 0 %novoIP%
    echo 1 1 %novoIP%
) > "%pastaAtual%\DataServer\serverlist.txt"

rem Alterar o serverip.txt
(
    echo Porta=%serveripPorta%
    echo IP=%novoIP%
) > "%pastaAtual%\GameServer\serverip.txt"

rem Alterar o dataserver.txt
(
    echo Porta=%dataserverPorta%
    echo IP=%novoIP%
) > "%pastaAtual%\GameServer\dataserver.txt"

echo Arquivos alterados com sucesso!
pause

