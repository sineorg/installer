#!/usr/bin/env bash

set -e

# Navigate to the directory where the script lives
cd "$(dirname "$0")"

# --------------------
# Argument parsing
# --------------------
browserPath=""
chromeFolder=""
installBoot=true
saveData=false

ORIGINAL_ARGS=("$@")

while [[ $# -gt 0 ]]; do
  case "$1" in
    -s)
      saveData=true
      ;;
    --browser)
      browserPath="$2"
      shift
      ;;
    --profile)
      chromeFolder="$2/chrome"
      shift
      ;;
    --no-boot)
      installBoot=false
      ;;
    --bootloader)
      bootloaderVersion="$2"
      shift
      ;;
    --version)
      sineVersion="$2"
      shift
      ;;
  esac

  shift
done

# --------------------
# Versions / links
# --------------------
bootloaderLink="https://github.com/sineorg/bootloader/releases/download/v$bootloaderVersion"
sineLink="https://github.com/CosmoCreeper/Sine/releases/download/v$sineVersion"

# --------------------
# Admin / root check
# --------------------
if [[ "$(id -u)" -ne 0 ]]; then
  # Cleanup
  rm -rf \
    "$chromeFolder/JS" \
    "$chromeFolder/locales" \
    "$chromeFolder/utils"
  if ! $saveData; then
    rm -rf "$chromeFolder/sine-mods"
  fi

  # Downloads
  curl -fsSL "$bootloaderLink/program.zip" -o program.zip
  curl -fsSL "$bootloaderLink/profile.zip" -o profile.zip

  curl -fsSL "$sineLink/engine.zip" -o engine.zip
  curl -fsSL "$sineLink/locales.zip" -o locales.zip

  # Extract
  unzip -oq profile.zip -d "$chromeFolder"
  rm -f profile.zip

  unzip -oq engine.zip -d "$chromeFolder"
  rm -f engine.zip

  unzip -oq locales.zip -d "$chromeFolder"
  rm -f locales.zip

  # Write test (permission check)
  if $installBoot; then
    if ! touch "$browserPath/.__writetest" 2>/dev/null; then
      pkexec "$0" "${ORIGINAL_ARGS[@]}"
      exit
    else
      rm -f "$browserPath/.__writetest"
    fi
  else
    rm -f "$chromeFolder/update"
  fi
fi

# --------------------
# Bootloader install
# --------------------
if $installBoot; then
  unzip -oq program.zip -d "$browserPath"
  rm -f program.zip
  rm -f "$chromeFolder/update"
fi
