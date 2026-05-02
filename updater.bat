@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

set "scriptPath=%~f0"

:parsePaths
if "%~1"=="" goto doneParsing

if "%~1"=="-s" (
    set "saveData=true"
    shift
    goto parsePaths
)

if "%~1"=="-u" (
    set "uninstall=true"
    shift
    goto parsePaths
)

if "%~1"=="--browser" (
    set "browserPath=%~2"
    shift
    shift
    goto parsePaths
)

if "%~1"=="--profile" (
    set "chromeFolder=%~2\chrome"
    shift
    shift
    goto parsePaths
)

if "%~1"=="--no-boot" (
    set "installBoot=false"
    shift
    goto parsePaths
)

if "%~1"=="--bootloader" (
    set "bootloaderVersion=%~2"
    shift
    shift
    goto parsePaths
)

if "%~1"=="--version" (
    set "sineVersion=%~2"
    shift
    shift
    goto parsePaths
)

if not defined extPath (
    set "extPath=%~1"
) else (
    set "extPath=!extPath! %~1"
)
shift
goto parsePaths
:doneParsing

if not defined installBoot set "installBoot=true"

set "bootloaderLink=https://github.com/sineorg/bootloader/releases/download/v%bootloaderVersion%"
set "sineLink=https://github.com/CosmoCreeper/Sine/releases/download/v%sineVersion%"

if not defined extPath (
    echo Welcome to Sine's official updater!
    echo Updating to v%sineVersion%...
    echo Do not cancel the installer. Doing so will crash your Sine installation.

    echo Update triggered with: v%sineVersion% >> "installer.log"
    echo Using bootloader version %bootloaderVersion% >> "installer.log"
    echo Using browser path: %browserPath% >> "installer.log"

    if exist "%chromeFolder%\JS" (
        rmdir /s /q "%chromeFolder%\JS" 2>>installer.log
    )
    if exist "%chromeFolder%\utils" (
        rmdir /s /q "%chromeFolder%\utils" 2>>installer.log
    )
    if not defined saveData if exist "%chromeFolder%\sine-mods" (
        rmdir /s /q "%chromeFolder%\sine-mods" 2>>installer.log
    )

    set "configFolder=%userprofile%\.librewolf"
    echo(!chromeFolder! | find /i "librewolf" >nul
    if !errorlevel! equ 0 (
        set "isLibrewolf=true"
        mkdir "!configFolder!" 2>>installer.log
        del "!configFolder!\librewolf.overrides.cfg" 2>>installer.log
    ) else (
        set "isLibrewolf=false"
    )

    echo Is Librewolf? !isLibrewolf! >> "installer.log"

    if not defined uninstall (
        curl -L "%bootloaderLink%/program.zip" -o program.zip 2>>installer.log
        curl -L "%bootloaderLink%/profile.zip" -o profile.zip 2>>installer.log

        curl -L "%sineLink%/engine.zip" -o engine.zip 2>>installer.log

        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'profile.zip' '%chromeFolder%'" 2>>installer.log
        del profile.zip 2>>installer.log

        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'engine.zip' '%chromeFolder%'" 2>>installer.log
        del engine.zip 2>>installer.log

        if "!isLibrewolf!"=="true" (
            "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
                "Expand-Archive -Force 'program.zip' '!configFolder!'" 2>>installer.log
            del program.zip 2>>installer.log
            rmdir /s /q "!configFolder!\defaults" 2>>installer.log
            ren "!configFolder!\config.js" "librewolf.overrides.cfg" 2>>installer.log
        )
    )

    if "%installBoot%"=="true" if "!isLibrewolf!"=="false" (
        echo. > "%browserPath%\.__writetest" 2>nul
        if not exist "%browserPath%\.__writetest" (
            set "args=%browserPath%"
            if defined uninstall set "args=-u %args%"
            "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -Command ^
                "Start-Process -FilePath '!scriptPath!' -ArgumentList '!args!' -Verb RunAs -Wait" 2>>installer.log
            set "installBoot=false"
        ) else (
            del "%browserPath%\.__writetest" >nul 2>&1
            set "extPath=%browserPath%"
            set "isNested=false"
        )
    )
)

if not defined isLibrewolf set "isLibrewolf=false"
if not defined isNested set "isNested=true"

if "%installBoot%"=="true" if "!isLibrewolf!"=="false" (
    if defined uninstall (
        del "!extPath!\config.js" 2>>bootloader.log
        del "!extPath!\defaults\pref\config-prefs.js" 2>>bootloader.log
    ) else (
        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'program.zip' '!extPath!'" 2>>bootloader.log

        if !isNested!=="true" (
            exit
        )
    )
)

del program.zip 2>>installer.log

if exist "%chromeFolder%\update" (
    del "%chromeFolder%\update" 2>>installer.log
)
