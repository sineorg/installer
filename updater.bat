@echo off
setlocal enabledelayedexpansion

pause

cd /d "%~dp0"

pause

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

shift
goto parsePaths

:doneParsing

pause

if not defined installBoot set "installBoot=true"

pause

set "bootloaderLink=https://github.com/sineorg/bootloader/releases/download/v%bootloaderVersion%"
set "sineLink=https://github.com/CosmoCreeper/Sine/releases/download/v%sineVersion%"

pause

net session >nul 2>&1
if errorlevel 1 (
    echo Welcome to Sine's official updater!
    echo Updating to v%sineVersion%...
    echo Do not cancel the installer. Doing so will crash your Sine installation.

    pause

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

    pause

    if not defined uninstall (
        curl -L "%bootloaderLink%/program.zip" -o program.zip >nul 2>&1
        curl -L "%bootloaderLink%/profile.zip" -o profile.zip >nul 2>&1

        curl -L "%sineLink%/engine.zip" -o engine.zip >nul 2>&1
        curl -L "%sineLink%/locales.zip" -o locales.zip >nul 2>&1

        powershell -NoProfile -Command ^
            "Expand-Archive -Force 'profile.zip' '%chromeFolder%'"
        del profile.zip >nul 2>&1

        powershell -NoProfile -Command ^
            "Expand-Archive -Force 'engine.zip' '%chromeFolder%'"
        del engine.zip >nul 2>&1

        powershell -NoProfile -Command ^
            "Expand-Archive -Force 'locales.zip' '%chromeFolder%'"
        del locales.zip >nul 2>&1
    )

    pause
    if "%installBoot%"=="true" (
        set "batchPath=%~dp0updater.bat"
        echo. > "%browserPath%\.__writetest" 2>nul
        pause
        if not exist "%browserPath%\.__writetest" (
            pause
            powershell -Command "Start-Process -FilePath '%batchPath%' -ArgumentList '%*' -Verb RunAs -Wait"
            pause
            exit /b
        ) else (
            del "%browserPath%\.__writetest" >nul 2>&1
        )
    ) else (
        if exist "%chromeFolder%\update" (
            del "%chromeFolder%\update"
        )
    )
)

pause

if "%installBoot%"=="true" (
    if "%uninstall%"=="true" (
        pause
        del "%browserPath%\config.js"
        del "%browserPath%\defaults\pref\config-prefs.js"
    ) else (
        pause
        powershell -NoProfile -Command ^
            "Expand-Archive -Force 'program.zip' '%browserPath%'"
        pause
        del program.zip
    )

    pause

    if exist "%chromeFolder%\update" (
        del "%chromeFolder%\update"
    )
)











