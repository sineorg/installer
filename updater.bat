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

    if exist "%chromeFolder%\JS" (
        rmdir /s /q "%chromeFolder%\JS"
    )
    if exist "%chromeFolder%\locales" (
        rmdir /s /q "%chromeFolder%\locales"
    )
    if exist "%chromeFolder%\utils" (
        rmdir /s /q "%chromeFolder%\utils"
    )
    if not defined saveData if exist "%chromeFolder%\sine-mods" (
        rmdir /s /q "%chromeFolder%\sine-mods"
    )

    set "configFolder=%userprofile%\.librewolf"
    echo %chromeFolder% | find "librewolf" >nul
    if %errorlevel%==0 (
        set "isLibrewolf=true"
        mkdir "%configFolder%"
        del "%configFolder%\librewolf.overrides.cfg" >nul 2>&1
    ) else (
        set "isLibrewolf=false"
    )

    if not defined uninstall (
        curl -L "%bootloaderLink%/program.zip" -o program.zip >nul 2>&1
        curl -L "%bootloaderLink%/profile.zip" -o profile.zip >nul 2>&1

        curl -L "%sineLink%/engine.zip" -o engine.zip >nul 2>&1
        curl -L "%sineLink%/locales.zip" -o locales.zip >nul 2>&1

        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'profile.zip' '%chromeFolder%'"
        del profile.zip >nul 2>&1

        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'engine.zip' '%chromeFolder%'"
        del engine.zip >nul 2>&1

        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'locales.zip' '%chromeFolder%'"
        del locales.zip >nul 2>&1

        if "!isLibrewolf!"=="true" (
            "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
                "Expand-Archive -Force 'program.zip' '%configFolder%'"
            del program.zip
            rmdir /s /q "%configFolder%\defaults"
            ren "%configFolder%\config.js" "librewolf.overrides.cfg"
        )
    )

    if "%installBoot%"=="true" if "!isLibrewolf!"=="false" (
        echo. > "%browserPath%\.__writetest" 2>nul
        if not exist "%browserPath%\.__writetest" (
            "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -Command ^
  "Start-Process -FilePath '!scriptPath!' -ArgumentList '%browserPath%' -Verb RunAs -Wait"
            set "installBoot=false"
        ) else (
            del "%browserPath%\.__writetest" >nul 2>&1
            set "extPath=%browserPath%"
        )
    )
)

if "%installBoot%"=="true" if "!isLibrewolf!"=="false" (
    if defined uninstall (
        del "!extPath!\config.js"
        del "!extPath!\defaults\pref\config-prefs.js"
    ) else (
        "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -Command ^
            "Expand-Archive -Force 'program.zip' '!extPath!'"
        del program.zip
    )
)

if exist "%chromeFolder%\update" (
    del "%chromeFolder%\update"
)
