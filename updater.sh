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
uninstall=false

ORIGINAL_ARGS=("$@")

while [[ $# -gt 0 ]]; do
  case "$1" in
    -s)
      saveData=true
      ;;
    -u)
      uninstall=true
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

download_file() {
    local url=$1
    local dest=$2
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$url" -o "$dest"
    elif command -v wget >/dev/null 2>&1; then
        wget -q -O "$dest" "$url"
    else
        echo "Error: Neither curl nor wget found. Please install one."
        exit 1
    fi
}

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

  # Librewolf detection and cleanup
  configFolder="~/.librewolf"
  # Convert to lowercase then check if Librewolf is mentioned
  if [[ "${chromeFolder,,}" == *"librewolf"* ]]; then
    isLibrewolf="true"
    mkdir -p "$configFolder"
    rm -f "$configFolder/librewolf.overrides.cfg"
  else
    isLibrewolf="false"
  fi

  if ! $uninstall; then
    # Downloads
    download_file "$bootloaderLink/program.zip" "program.zip"
    download_file "$bootloaderLink/profile.zip" "profile.zip"

    download_file "$sineLink/engine.zip" "engine.zip"
    download_file "$sineLink/locales.zip" "locales.zip"

    # Extract
    unzip -oq profile.zip -d "$chromeFolder"
    rm -f profile.zip

    unzip -oq engine.zip -d "$chromeFolder"
    rm -f engine.zip

    unzip -oq locales.zip -d "$chromeFolder"
    rm -f locales.zip

    if $isLibrewolf; then
      unzip -oq program.zip -d "$configFolder"
      rm -f program.zip
      rm -rf "$configFolder/defaults"
      mv "$configFolder/config.js" "$configFolder/librewolf.overrides.cfg"
    fi
  fi

  # Write test (permission check)
  if $installBoot && ! $isLibrewolf; then
    if ! touch "$browserPath/.__writetest" 2>/dev/null; then
      if [[ "$OSTYPE" == "darwin"* ]]; then
        cmd="$0"
        for arg in "${ORIGINAL_ARGS[@]}"; do
          cmd+=" $(printf '%q' "$arg")"
        done

        escaped_cmd=$(printf '%s' "$cmd" | sed 's/"/\\"/g')
        osascript -e "do shell script \"$escaped_cmd\" with administrator privileges"
      else
        pkexec "$0" "${ORIGINAL_ARGS[@]}"
      fi
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
  if $uninstall; then
    rm -f "$browserPath/config.js"
    rm -f "$browserPath/defaults/pref/config-prefs.js"
  else
    unzip -oq program.zip -d "$browserPath"
    rm -f program.zip
  fi

  rm -f "$chromeFolder/update"
fi
