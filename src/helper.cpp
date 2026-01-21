// sine-installer-helper: Privileged helper for file operations
// This binary handles operations that require root privileges.
// It is called by the main GUI installer via pkexec or sudo.

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "data.h"

namespace fs = std::filesystem;

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  --install              Install Sine\n"
              << "  --uninstall            Uninstall Sine\n"
              << "  --browser-dir=PATH     Browser installation directory\n"
              << "  --profile-dir=PATH     Profile directory\n"
              << "  --downloads-dir=PATH   Downloads directory (for zip files)\n"
              << "  --save-data            Keep user data during uninstall\n"
              << "  --no-boot              Skip bootloader installation\n"
              << std::endl;
}

bool removeDir(const std::string& path) {
    try {
        if (fs::exists(path)) {
            fs::remove_all(path);
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error removing directory " << path << ": " << e.what() << std::endl;
        return false;
    }
}

bool runCommand(const std::string& cmd) {
    int result = system(cmd.c_str());
    return result == 0;
}

bool extractZip(const std::string& zipPath, const std::string& destDir) {
    fs::create_directories(destDir);
    std::string cmd = "unzip -q -o \"" + zipPath + "\" -d \"" + destDir + "\"";
    return runCommand(cmd);
}

int doInstall(const std::string& browserDir, const std::string& profileDir, 
              const std::string& downloadsDir, bool reinstallBoot, bool saveData) {
    
    std::cout << "Installing Sine..." << std::endl;
    std::cout << "Browser dir: " << browserDir << std::endl;
    std::cout << "Profile dir: " << profileDir << std::endl;
    std::cout << "Downloads dir: " << downloadsDir << std::endl;

    // Ensure chrome directory exists
    fs::path chromeDir = fs::path(profileDir) / "chrome";
    if (!fs::exists(chromeDir)) {
        fs::create_directories(chromeDir);
    }

    // Configure browser (extract program.zip)
    if (reinstallBoot) {
        std::string programZip = downloadsDir + "/program.zip";
        if (fs::exists(programZip)) {
            std::cout << "Configuring browser..." << std::endl;
            if (!extractZip(programZip, browserDir)) {
                std::cerr << "Failed to extract program.zip" << std::endl;
                return 1;
            }
        }
    }

    // Clean up old profile files
    std::cout << "Cleaning up old profile..." << std::endl;
    removeDir(profileDir + "/chrome/JS");
    removeDir(profileDir + "/chrome/utils");
    removeDir(profileDir + "/chrome/locales");

    // Extract profile files
    std::string profileZip = downloadsDir + "/profile.zip";
    if (fs::exists(profileZip)) {
        std::cout << "Configuring profile..." << std::endl;
        if (!extractZip(profileZip, chromeDir.string())) {
            std::cerr << "Failed to extract profile.zip" << std::endl;
            return 1;
        }
    }

    // Extract engine
    std::string engineZip = downloadsDir + "/engine.zip";
    if (fs::exists(engineZip)) {
        std::cout << "Installing engine..." << std::endl;
        if (!extractZip(engineZip, chromeDir.string())) {
            std::cerr << "Failed to extract engine.zip" << std::endl;
            return 1;
        }
    }

    // Extract locales
    std::string localesZip = downloadsDir + "/locales.zip";
    if (fs::exists(localesZip)) {
        std::cout << "Installing locales..." << std::endl;
        if (!extractZip(localesZip, chromeDir.string())) {
            std::cerr << "Failed to extract locales.zip" << std::endl;
            return 1;
        }
    }

    // Write prefs.js
    std::cout << "Writing preferences..." << std::endl;
    std::ofstream prefsFile(profileDir + "/prefs.js", std::ios::app);
    if (prefsFile.is_open()) {
        prefsFile << "user_pref(\"sine.is-cosine\", " << (isCosine ? "true" : "false") << ");\n";
        prefsFile << "user_pref(\"sine.version\", \"" << sineVersion << "\");\n";
        prefsFile << "user_pref(\"sine.latest-version\", \"" << sineVersion << "\");\n";
        prefsFile.close();
    }

    // Remove mods if not saving data
    if (!saveData) {
        fs::path modsDir = fs::path(profileDir) / "chrome" / "sine-mods";
        if (fs::exists(modsDir)) {
            std::cout << "Removing mods..." << std::endl;
            removeDir(modsDir.string());
        }
    }

    std::cout << "Installation complete." << std::endl;
    return 0;
}

int doUninstall(const std::string& browserDir, const std::string& profileDir, bool saveData) {
    std::cout << "Uninstalling Sine..." << std::endl;

    // Clean up browser
    std::cout << "Cleaning up browser..." << std::endl;
    fs::remove(fs::path(browserDir) / "defaults" / "pref" / "config-prefs.js");
    fs::remove(fs::path(browserDir) / "config.js");

    // Clean up profile
    std::cout << "Cleaning up profile..." << std::endl;
    removeDir(profileDir + "/chrome/JS");
    removeDir(profileDir + "/chrome/utils");
    removeDir(profileDir + "/chrome/locales");

    // Remove mods if not saving data
    if (!saveData) {
        fs::path modsDir = fs::path(profileDir) / "chrome" / "sine-mods";
        if (fs::exists(modsDir)) {
            std::cout << "Removing mods..." << std::endl;
            removeDir(modsDir.string());
        }
    }

    std::cout << "Uninstall complete." << std::endl;
    return 0;
}

int main(int argc, char** argv) {
#ifndef _WIN32
    if (geteuid() != 0) {
        std::cerr << "Warning: This helper should be run as root for full functionality.\n";
    }
#endif

    std::string browserDir;
    std::string profileDir;
    std::string downloadsDir;
    bool doInstallFlag = false;
    bool doUninstallFlag = false;
    bool saveData = false;
    bool reinstallBoot = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--install") {
            doInstallFlag = true;
        } else if (arg == "--uninstall") {
            doUninstallFlag = true;
        } else if (arg == "--save-data") {
            saveData = true;
        } else if (arg == "--no-boot") {
            reinstallBoot = false;
        } else if (arg.rfind("--browser-dir=", 0) == 0) {
            browserDir = arg.substr(strlen("--browser-dir="));
        } else if (arg.rfind("--profile-dir=", 0) == 0) {
            profileDir = arg.substr(strlen("--profile-dir="));
        } else if (arg.rfind("--downloads-dir=", 0) == 0) {
            downloadsDir = arg.substr(strlen("--downloads-dir="));
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }

    if (browserDir.empty() || profileDir.empty()) {
        std::cerr << "Error: --browser-dir and --profile-dir are required.\n";
        printUsage(argv[0]);
        return 1;
    }

    if (!doInstallFlag && !doUninstallFlag) {
        std::cerr << "Error: Must specify --install or --uninstall.\n";
        printUsage(argv[0]);
        return 1;
    }

    if (doInstallFlag && downloadsDir.empty()) {
        std::cerr << "Error: --downloads-dir is required for installation.\n";
        return 1;
    }

    if (doInstallFlag) {
        return doInstall(browserDir, profileDir, downloadsDir, reinstallBoot, saveData);
    } else if (doUninstallFlag) {
        return doUninstall(browserDir, profileDir, saveData);
    }

    return 0;
}
