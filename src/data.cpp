#include <data.h>

#ifdef _WIN32
const std::string homePath = []() {
    const char* envHome = std::getenv("USERPROFILE");
    if (!envHome) return std::string("");  // fallback
    return std::string(envHome);
}();
#else
const std::string homePath = "";
#endif

const std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::map<std::string, std::vector<std::string>>>>>> browsers = {
    {"Firefox", {
        {"profile", {
            {"win32", { "Mozilla\\Firefox" }},
            {"linux", { ".mozilla/firefox" }},
            {"darwin", { "Firefox" }}
        }},
        {"Stable", {
            {"win32", {
                "C:\\Program Files\\Mozilla Firefox",
                "C:\\Program Files (x86)\\Mozilla Firefox"
            }},
            {"linux", { "/usr/lib/firefox/", "/opt/firefox/", "/root/snap/firefox/" }},
            {"darwin", { "/Applications/Firefox.app/Contents/Resources" }}
        }},
        {"Developer Edition", {
            {"win32", {
                "C:\\Program Files\\Firefox Developer Edition",
                "C:\\Program Files (x86)\\Firefox Developer Edition"
            }},
            {"linux", { "/opt/firefox-developer-edition/" }},
            {"darwin", { "/Applications/Firefox Developer Edition.app/Contents/Resources" }}
        }},
        {"Nightly", {
            {"win32", {
                "C:\\Program Files\\Firefox Nightly",
                "C:\\Program Files (x86)\\Firefox Nightly"
            }},
            {"linux", { "/opt/firefox-nightly/" }},
            {"darwin", { "/Applications/Firefox Nightly.app/Contents/Resources" }}
        }}
    }},
    {"Floorp", {
        {"profile", {
            {"win32", { "Floorp" }},
            {"linux", { ".floorp" }},
            {"darwin", { "Floorp" }}
        }},
        {"Stable", {
            {"win32", {
                "C:\\Program Files\\Ablaze Floorp",
                "C:\\Program Files (x86)\\Ablaze Floorp"
            }},
            {"linux", { "/opt/floorp/" }},
            {"darwin", { "/Applications/Floorp.app/Contents/Resources" }}
        }}
    }},
    {"Mullvad", {
        {"profile", {
            {"win32", { "Mullvad\\MullvadBrowser" }},
            {"linux", { ".mullvad-browser" }},
            {"darwin", { "MullvadBrowser" }}
        }},
        {"Stable", {
            {"win32", { homePath + "\\AppData\\Local\\Mullvad\\MullvadBrowser\\Release" }},
            {"linux", { "/opt/mullvad-browser/" }},
            {"darwin", { "/Applications/Mullvad Browser.app/Contents/Resources" }}
        }},
        {"Alpha", {
            {"win32", { homePath + "\\AppData\\Local\\Mullvad\\MullvadBrowser\\Alpha" }},
            {"linux", { "/opt/mullvad-browser-alpha/" }},
            {"darwin", { "/Applications/Mullvad Browser Alpha.app/Contents/Resources" }}
        }}
    }},
    {"Waterfox", {
        {"profile", {
            {"win32", { "Waterfox" }},
            {"linux", { ".waterfox" }},
            {"darwin", { "Waterfox" }}
        }},
        {"Stable", {
            {"win32", { "C:\\Program Files\\Waterfox", "C:\\Program Files (x86)\\Waterfox" }},
            {"linux", { "/opt/waterfox/" }},
            {"darwin", { "/Applications/Waterfox.app/Contents/Resources" }}
        }}
    }},
    {"Zen", {
        {"profile", {
            {"win32", { "zen" }},
            {"linux", { ".zen" }},
            {"darwin", { "Zen" }}
        }},
        {"Beta", {
            {"win32", {
                "C:\\Program Files\\Zen Browser",
                "C:\\Program Files (x86)\\Zen Browser"
            }},
            {"linux", { "/opt/zen-browser-bin/", "/opt/zen-browser/", "/opt/zen/" }},
            {"darwin", {
                "/Applications/Zen Browser.app/contents/resources",
                "/Applications/Zen.app/Contents/Resources"
            }}
        }},
        {"Twilight", {
            {"win32", {
                "C:\\Program Files\\Zen Twilight",
                "C:\\Program Files (x86)\\Zen Twilight"
            }},
            {"linux", { "/opt/zen-twilight/", "/opt/zen-browser-twilight/" }},
            {"darwin", {
                "/Applications/Zen Browser.app/Twilight/contents/resources",
                "/Applications/Zen.app/Twilight/Contents/Resources",
                "/Applications/Twilight.app/Contents/Resources"
            }}
        }}
    }}
};

const std::string bootVersion = "0.1.1";
const std::string sineVersion = "2.3c";
const bool isCosine = true;
