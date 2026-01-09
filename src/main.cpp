#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define NOMINMAX
#include <curl/curl.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "CascadiaCode-Regular.h"
#include "CascadiaCode-Bold.h"
#include "CascadiaCode-Light.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <array>
#include <string>

#include <algorithm>
#include <cctype>
#include <cstring>

#include <data.h>
#include <stdlib.h>
#include <cstdlib>
#include <filesystem>
#include <sys/stat.h>
#include <fstream>

#include <minizip-ng/mz.h>
#include <minizip-ng/mz_strm.h>
#include <minizip-ng/mz_strm_os.h>
#include <minizip-ng/mz_zip.h>
#include <minizip-ng/mz_zip_rw.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include <aclapi.h>
#include <tlhelp32.h>

int main(int argc, char** argv);

int WINAPI WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int
) {
    return main(__argc, __argv);
}
#else
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <sys/wait.h>
#endif

#if __APPLE__
#include <cstdio>
#include <memory>
#include <mach-o/dyld.h>
#endif

using namespace std::chrono;

enum class State
{
    START,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    LAST,
    END
};

State state = State::START;

void nextState() {
    int stateInt = static_cast<int>(state);
    state = static_cast<State>(stateInt + 1);
}

void prevState() {
    int stateInt = static_cast<int>(state);
    state = static_cast<State>(stateInt - 1);
}

std::vector<std::string> getBrowserNames() {
    std::vector<std::string> names;
    names.reserve(browsers.size());

    for (const auto& [browser, _] : browsers) {
        names.push_back(browser);
    }

    return names;
}

std::vector<std::string> getBrowserVersions(size_t browserIndex)
{
    std::vector<std::string> versionNames;

    if (browserIndex >= browsers.size())
        return versionNames;

    auto it = browsers.begin();
    std::advance(it, browserIndex);
    const auto& versionMap = it->second;

    for (const auto& [version, _] : versionMap)
    {
        if (version != "profile")
        {
            versionNames.push_back(version);
        }
    }

    return versionNames;
}

std::string getOS()
{
#if defined(_WIN32) || defined(_WIN64)
    return "win32";
#elif defined(__APPLE__) || defined(__MACH__)
    return "darwin";
#elif defined(__linux__)
    return "linux";
#else
    return "unsupported";
#endif
}

std::string getBrowserLocation(int browserIndex, int versionIndex)
{
    std::string os = getOS();
    std::vector<std::string> browserPaths = browsers[browserIndex].second[versionIndex + 1].second.find(os)->second;
    for (std::string path : browserPaths)
    {
        if (std::filesystem::exists(path))
        {
            return path;
        }
    }
    return "";
}

std::string getProfileLocation(int browserIndex)
{
    std::string os = getOS();
    std::string profilePath = browsers[browserIndex].second[0].second.find(os)->second[0];
    
    if (os == "win32")
    {
        std::filesystem::path appData = std::getenv("APPDATA");
        profilePath = (appData / profilePath / "Profiles").string();
    }
    else
    {
        std::filesystem::path home = std::getenv("HOME");
        if (os == "darwin")
        {
            profilePath = (home / "Library" / "Application Support" / profilePath / "Profiles").string();
        }
        else if (os == "linux")
        {
            profilePath = (home / profilePath).string();
        }
    }

    return profilePath;
}

std::string toLowercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return str;
}

static void framebuffer_size_callback(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}

float getCenteredText(const char* &text)
{
    float windowHeight = ImGui::GetWindowSize().x;
    ImVec2 textSize   = ImGui::CalcTextSize(text);

    float textIndentation = (windowHeight - textSize.x) / 2.0f;
    float posX = windowHeight - textIndentation;

    float availableHeight = ImGui::GetContentRegionAvail().y;
    float heightSpacing = availableHeight / 2.0f - textSize.y * 4;

    ImGui::SetCursorPosX(textIndentation);
    ImGui::SetCursorPosY(heightSpacing);

    return posX;
}

void colorFade(float timeDiff, const std::array<int, 2>& thresholds = {0, 0}, const std::array<int, 2>& smoothers = {300, 300})
{
    float alpha = 1.0f;
    if (timeDiff < thresholds[0])
    {
        alpha = timeDiff / smoothers[0];
    }
    else if (timeDiff > thresholds[1] && thresholds[1] != 0)
    {
        alpha = 1.0f - ((timeDiff - thresholds[1]) / smoothers[1]);
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
}

void renderHeader(ImFont* &font, const float timeDiff)
{
    const char* headerText = "Sine";
    colorFade(timeDiff, {500, 0}, {300, 0});
    ImGui::PushFont(font);
    ImGui::Text(headerText);
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::Separator();
}

void renderStepHeader(const char* stepHeader, ImFont* &font, const float timeDiff)
{
    colorFade(timeDiff, {500, 0}, {300, 0});
    ImGui::PushFont(font);
    ImGui::Text(stepHeader);
    ImGui::PopFont();
    ImGui::PopStyleColor();
}

void renderOptions(const std::vector<std::string>& optionsVector, int& selectedOption, ImFont* &font)
{
    ImGui::PushFont(font);
    ImGui::Spacing();

    for (size_t i = 0; i < optionsVector.size(); ++i)
    {
        if (ImGui::RadioButton(optionsVector[i].c_str(), selectedOption == i))
        {
            selectedOption = i;
        }
    }

    ImGui::PopFont();
}

void renderFooter(ImFont* &font, float uiScale, ImVec2 windowSize, bool hideEnd = false, bool backBtnDisabled = false)
{
    bool finishBtn = false;

    if (state == State::ONE)
    {
        backBtnDisabled = true;
    }
    else if (state == State::LAST)
    {
        backBtnDisabled = true;
        finishBtn = true;
    }

    float buttonWidth = 120 * uiScale, buttonHeight = 28 * uiScale;
    float margin = 10 * uiScale;

    float bottomY = windowSize.y - buttonHeight - margin;

    ImGui::PushFont(font);

    // Bottom-left button
    ImGui::SetCursorPos(ImVec2(windowSize.x - buttonWidth * 2 - margin * 2, bottomY));
    ImGui::BeginDisabled(backBtnDisabled);
    if (ImGui::Button("Back", ImVec2(buttonWidth, buttonHeight)))
    {
        prevState();
    }
    ImGui::EndDisabled();

    // Bottom-right button
    ImGui::SetCursorPos(ImVec2(windowSize.x - buttonWidth - margin, bottomY));
    ImGui::BeginDisabled(hideEnd);
    if (ImGui::Button(finishBtn ? "Finish" : "Next", ImVec2(buttonWidth, buttonHeight)))
    {
        nextState();
    }
    ImGui::EndDisabled();

    ImGui::PopFont();
}

bool fixFilePerms(const std::string& filepath) {
#ifdef _WIN32
    // Remove read-only attribute
    DWORD attrs = GetFileAttributesA(filepath.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES)
    {
        SetFileAttributesA(filepath.c_str(), attrs & ~FILE_ATTRIBUTE_READONLY);
    }

    // Get current user SID
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        return false;
    }

    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
    TOKEN_USER* pTokenUser = (TOKEN_USER*)malloc(dwSize);

    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize))
    {
        free(pTokenUser);
        CloseHandle(hToken);
        return false;
    }

    // Set owner to current user
    DWORD result = SetNamedSecurityInfoA(
        (LPSTR)filepath.c_str(),
        SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION,
        pTokenUser->User.Sid,
        NULL, NULL, NULL
    );

    free(pTokenUser);
    CloseHandle(hToken);

    return (result == ERROR_SUCCESS);

#else
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0)
        return false;

    if (S_ISDIR(st.st_mode))
    {
        // Directories: 755
        if (chmod(filepath.c_str(),
                  S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
            return false;
    }
    else
    {
        // Files: 644
        if (chmod(filepath.c_str(),
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0)
            return false;
    }

    return false;
#endif
}

std::string shellEscape(const std::string& input)
{
    // POSIX shell-safe escaping using single quotes
    // abc'def → 'abc'"'"'def'
    std::string out;
    out.reserve(input.size() + 2);

    out.push_back('\'');
    for (char c : input)
    {
        if (c == '\'')
            out += "'\"'\"'";
        else
            out.push_back(c);
    }
    out.push_back('\'');

    return out;
}

bool requestAdmin(const std::string& browserPath, const std::string& profilePath, bool shouldSaveData, bool shouldUninstall, bool reinstallBoot, bool showExitScreen)
{
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    std::string parameters = "--browser \"" + browserPath + "\" "
        "--profile \"" + profilePath + "\" "
        + (shouldSaveData ? "-s " : "")
        + (shouldUninstall ? "-u" : "")
        + (reinstallBoot ? "" : "--no-boot")
        + (showExitScreen ? "" : "--update");

    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.lpVerb = "runas";  // request admin
    sei.lpFile = path;
    sei.nShow = SW_NORMAL;
    sei.lpParameters = parameters.c_str();

    if (!ShellExecuteExA(&sei))
    {
        std::cerr << "Failed to request admin privileges.\n";
        return false;
    }

    return true;

#elif defined(__linux__)
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len <= 0)
        return false;

    exePath[len] = '\0';

    // Get required environment variables from the current user session
    const char* display = std::getenv("DISPLAY");
    const char* xauth = std::getenv("XAUTHORITY");
    const char* home = std::getenv("HOME");

    std::string cmd = "pkexec --disable-internal-agent env ";

    // Pass display variables to the root environment
    if (const char* display = std::getenv("DISPLAY"))
        cmd += "DISPLAY=" + std::string(display) + " ";
    if (const char* xauth = std::getenv("XAUTHORITY"))
        cmd += "XAUTHORITY=" + std::string(xauth) + " ";
    if (const char* home = std::getenv("HOME"))
	cmd += "HOME=" + std::string(home) + " ";

    // Now add the program path and its arguments
    cmd += shellEscape(exePath);
    cmd += " --helper --browser " + shellEscape(browserPath);
    cmd += " --profile " + shellEscape(profilePath);

    if (shouldUninstall)
        cmd += " -u";

    int result = system(cmd.c_str());
    return WIFEXITED(result) && WEXITSTATUS(result) == 0;

#elif defined(__APPLE__)
    char exePath[PATH_MAX];
    uint32_t size = sizeof(exePath);

    if (_NSGetExecutablePath(exePath, &size) != 0)
        return false;

    std::string args;
    args += "\"" + shellEscape(exePath) + "\"";
    args += " --browser \"" + shellEscape(browserPath) + "\"";
    args += " --profile \"" + shellEscape(profilePath) + "\"";

    if (shouldSaveData)   args += " -s";
    if (shouldUninstall) args += " -u";
    if (!reinstallBoot)  args += " --no-boot";
    if (!showExitScreen) args += " --update";

    std::string script =
        "do shell script \"" + shellEscape(args) +
        "\" with administrator privileges";

    std::string cmd =
        "osascript -e \"" + shellEscape(script) + "\"";

    int result = system(cmd.c_str());

    if (result == 0)
    {
        _exit(0);
    }

    return false;

#else
    std::cerr << "Unsupported OS.\n";
    return false;
#endif
}

bool isUserAdmin()
{
#ifdef _WIN32
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;

    // Create a SID for the administrators group
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &administratorsGroup))
    {
        // Check if the token contains the admin SID
        if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin))
            isAdmin = FALSE;

        FreeSid(administratorsGroup);
    }

    return isAdmin;

#else
    // POSIX (Linux/macOS)
    return geteuid() == 0;
#endif
}

bool canWriteToFolder(const std::filesystem::path& folder)
{
    try
    {
        if (!std::filesystem::exists(folder))
        {
            std::cerr << "Folder does not exist.\n";
            return false;
        }

        std::filesystem::path testFile = folder / "temp_write_test.tmp";

        std::ofstream ofs(testFile);
        if (!ofs)
        {
            // Could not open file for writing
            return false;
        }

        ofs << "test"; // Try writing something
        ofs.close();

        std::filesystem::remove(testFile); // Clean up
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::string getDownloadsFolder()
{
#ifdef _WIN32
    PWSTR path = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, NULL, &path)))
    {
        char downloadsPath[MAX_PATH];
        wcstombs(downloadsPath, path, MAX_PATH);
        CoTaskMemFree(path);
        return std::string(downloadsPath);
    }
    return "C:\\Users\\Default\\Downloads";
#else
    const char* home = std::getenv("HOME");
    return std::string(home ? home : "/tmp") + "/Downloads";
#endif
}

size_t writeData(void* ptr, size_t size, size_t nmemb, void* stream)
{
    std::ofstream* out = static_cast<std::ofstream*>(stream);
    out->write(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

bool downloadFile(const std::string& url, const std::string& outputPath)
{
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    file.close();

    if (res == CURLE_OK)
    {
        fixFilePerms(outputPath);
    }

    return (res == CURLE_OK);
}

void extractZip(const std::string& zipPath, const std::string& outputDir)
{
    std::filesystem::create_directories(outputDir);
    fixFilePerms(outputDir);

    void* reader = mz_zip_reader_create();
    mz_zip_reader_open_file(reader, zipPath.c_str());
    mz_zip_reader_goto_first_entry(reader);

    do
    {
        mz_zip_file* file_info = nullptr;
        mz_zip_reader_entry_get_info(reader, &file_info);

        std::string outPath = outputDir + "/" + file_info->filename;

        if (mz_zip_reader_entry_is_dir(reader) == MZ_OK)
        {
            std::filesystem::create_directories(outPath);
            fixFilePerms(outPath);
        }
        else
        {
            std::filesystem::create_directories(
                std::filesystem::path(outPath).parent_path());

            fixFilePerms(std::filesystem::path(outPath).parent_path().string());

            // Read entry data into buffer
            mz_zip_reader_entry_open(reader);

            std::vector<uint8_t> buffer(file_info->uncompressed_size);
            int32_t bytes_read = mz_zip_reader_entry_read(reader, buffer.data(), buffer.size());

            mz_zip_reader_entry_close(reader);

            // Write buffer to file
            std::ofstream outFile(outPath, std::ios::binary);
            outFile.write(reinterpret_cast<char*>(buffer.data()), bytes_read);
            outFile.close();

            fixFilePerms(outPath);
        }
    } while (mz_zip_reader_goto_next_entry(reader) == MZ_OK);

    mz_zip_reader_close(reader);
    mz_zip_reader_delete(&reader);
}

bool isProcessRunning(const std::string& processName) {
#ifdef _WIN32
    // Windows implementation
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return false;

#elif __linux__
    // Linux implementation
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string cmdlinePath = entry.path().string() + "/cmdline";
            std::ifstream cmdline(cmdlinePath);
            std::string content;
            std::getline(cmdline, content, '\0');
            if (content.find(processName) != std::string::npos) {
                return true;
            }
        }
    }
    return false;

#elif __APPLE__
    // macOS implementation
    std::string command = "pgrep -x " + processName;
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        return false;
    }

    return fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr;

#else
    // Unsupported platform
    return false;
#endif
}

void removeDir(std::string path)
{
    if (std::filesystem::exists(path))
    {
        std::filesystem::remove_all(path);
    }
}

int main(int argc, char* argv[])
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    const int windowHeight = 900;
    const int windowWidth = 600;
    GLFWwindow* window = glfwCreateWindow(windowHeight, windowWidth, "Sine Installer", nullptr, nullptr);
    if (!window) return -1;

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);

    int w, h;
    glfwGetWindowSize(window, &w, &h);
    int posX = (mode->width - w) / 2;
    int posY = (mode->height - h) / 2;
    glfwSetWindowPos(window, posX, posY);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    float uiScale = std::max(xscale, yscale);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(uiScale);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    
#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 330");
#endif

    // === Embedded fonts ===
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;

    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;

    ImFont* mediumFont = io.Fonts->AddFontFromMemoryTTF(
        CascadiaCode_Regular_ttf,
        CascadiaCode_Regular_ttf_len,
        22.0f * uiScale,
        &cfg
    );

    ImFont* bodyFont = io.Fonts->AddFontFromMemoryTTF(
        CascadiaCode_Regular_ttf,
        CascadiaCode_Regular_ttf_len,
        18.0f * uiScale,
        &cfg
    );

    ImFont* lightFont = io.Fonts->AddFontFromMemoryTTF(
        CascadiaCode_Light_ttf,
        CascadiaCode_Light_ttf_len,
        14.0f * uiScale,
        &cfg
    );

    ImFont* titleFont = io.Fonts->AddFontFromMemoryTTF(
        CascadiaCode_Bold_ttf,
        CascadiaCode_Bold_ttf_len,
        36.0f * uiScale,
        &cfg
    );

    auto begin = high_resolution_clock::now();
    int selectedBrowser = 0;
    int selectedVersion = 0;
    int selectedProfile = 0;
    char browserPath[128] = "";
    char profileFolderPath[128] = "";
    char reason[128] = "";
    bool showHiddenProfiles = false;
    bool shouldReset = true;
    std::string browserPathStr;
    std::string profilePath;
    bool reinstallBoot = true;
    bool shouldSaveData = false;
    bool shouldUninstall = false;
    int shouldNotify = 0;
    bool isAdmin = isUserAdmin();
    bool shouldTryAdmin = true;
    int installStep = 0;
    int needsAdmin = -1;

    bool showExitScreen = true;

    const std::string downloadsFolder = getDownloadsFolder();

    const std::string bootloaderReleases = "https://github.com/sineorg/bootloader/releases/download/v";
    const std::string sineReleases = "https://github.com/CosmoCreeper/Sine/releases/download/v";

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--browser" && i + 1 < argc)
        {
            browserPathStr = argv[i + 1];
            ++i;
        }
        else if (arg == "--profile" && i + 1 < argc)
        {
            profilePath = argv[i + 1];
            ++i;
        }
        else if (arg == "--save" || arg == "-s")
        {
            shouldSaveData = true;
        }
        else if (arg == "--uninstall" || arg == "-u")
        {
            shouldUninstall = true;
        }
        else if (arg == "--no-boot")
        {
            reinstallBoot = false;
        }
        else if (arg == "--update")
        {
            showExitScreen = false;
        }
    }

    if (!browserPathStr.empty() && !profilePath.empty())
    {
        state = State::SIX;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Separator] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Main", nullptr, flags);

        auto end = high_resolution_clock::now();
        float timeDiff = duration_cast<std::chrono::milliseconds>(end - begin).count();

        if (state == State::START)
        {
            const char* transitionHeader = "your gateway to the internet:";
            float textPosX = getCenteredText(transitionHeader);
            colorFade(timeDiff, {500, 1500}, {300, 300});
            ImGui::PushTextWrapPos(textPosX);
            ImGui::PushFont(titleFont);
            ImGui::Text(transitionHeader);
            ImGui::PopFont();
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            if (timeDiff >= 2000)
            {
                state = State::ONE;
                begin = high_resolution_clock::now();
            }
        }
        else if (state == State::ONE)
        {
            renderHeader(titleFont, timeDiff);
            renderStepHeader("Pick your browser", mediumFont, timeDiff);
            renderOptions(getBrowserNames(), selectedBrowser, bodyFont);
            renderFooter(mediumFont, uiScale, io.DisplaySize);
        }
        else if (state == State::TWO)
        {
            const auto browserVersions = getBrowserVersions(selectedBrowser);
            if (browserVersions.size() == 1)
            {
                state = State::THREE;
            }
            else
            {
                renderHeader(titleFont, timeDiff);
                renderStepHeader("Pick your browser version", mediumFont, timeDiff);
                renderOptions(browserVersions, selectedVersion, bodyFont);
                renderFooter(mediumFont, uiScale, io.DisplaySize);
            }
        }
        else if (state == State::THREE)
        {
            renderHeader(titleFont, timeDiff);

            bool hasError = false;
            
            renderStepHeader("Confirm your browser location", mediumFont, timeDiff);
            const std::string autoBrowserPath = getBrowserLocation(selectedBrowser, selectedVersion);
            if (browserPath[0] == '\0')
            {
                memset(browserPath, 0, sizeof(browserPath));
                strncpy(browserPath, autoBrowserPath.c_str(), sizeof(browserPath) - 1);
            }
            ImGui::PushFont(bodyFont);
            ImGui::InputText("##browser", browserPath, IM_ARRAYSIZE(browserPath));
            ImGui::PopFont();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::PushFont(bodyFont);
            struct stat browserBuffer;
            if (stat(browserPath, &browserBuffer) == 0)
            {
                std::string browserDataStr = (std::filesystem::path(browserPath) / "browser").string();
                char browserData[128];
                strncpy(browserData, browserDataStr.c_str(), sizeof(browserData) - 1);
                browserData[sizeof(browserData) - 1] = '\0';

                if ((browserBuffer.st_mode & S_IFMT) == S_IFREG)
                {
                    ImGui::Text("Path should be a folder, not a file.");
                    hasError = true;
                }
                else if (stat(browserData, &browserBuffer) != 0 && getOS() != "darwin")
                {
                    ImGui::Text("Path should contain browser-like contents.");
                    hasError = true;
                }
            }
            else
            {
                ImGui::Text("Path does not exist.");
                hasError = true;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            renderStepHeader("Confirm your profile location", mediumFont, timeDiff);
            const std::string autoProfilePath = getProfileLocation(selectedBrowser);
            if (profileFolderPath[0] == '\0')
            {
                memset(profileFolderPath, 0, sizeof(profileFolderPath));
                strncpy(profileFolderPath, autoProfilePath.c_str(), sizeof(profileFolderPath) - 1);
            }
            ImGui::PushFont(bodyFont);
            ImGui::InputText("##profile", profileFolderPath, IM_ARRAYSIZE(profileFolderPath));
            ImGui::PopFont();
            browserPathStr = browserPath;

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::PushFont(bodyFont);
            if (stat(profileFolderPath, &browserBuffer) == 0)
            {
                std::string browserDataStr = (std::filesystem::path(profileFolderPath) / "Profiles").string();
                char browserData[128];
                strncpy(browserData, browserDataStr.c_str(), sizeof(browserData) - 1);
                browserData[sizeof(browserData) - 1] = '\0';

                if ((browserBuffer.st_mode & S_IFMT) == S_IFREG)
                {
                    ImGui::Text("Path should be a folder, not a file.");
                    hasError = true;
                }
                else if (stat(browserData, &browserBuffer) == 0)
                {
                    ImGui::Text("Path should be the profiles folder, not contain it.");
                    hasError = true;
                }
            }
            else
            {
                ImGui::Text("Path does not exist.");
                hasError = true;
            }
            ImGui::PopFont();
            ImGui::PopStyleColor();
            
            renderFooter(mediumFont, uiScale, io.DisplaySize, hasError);
        }
        else if (state == State::FOUR)
        {
            renderHeader(titleFont, timeDiff);

            renderStepHeader("Choose your profile", mediumFont, timeDiff);

            std::vector<std::string> profiles;
            for (const auto& entry : std::filesystem::directory_iterator(profileFolderPath))
            {
                if (std::filesystem::is_directory(entry.status()))
                {
                    std::string dirName = entry.path().filename().string();

                    if (showHiddenProfiles)
                    {
                        profiles.push_back(dirName);
                        continue;
                    }

                    for (const auto& subEntry : std::filesystem::directory_iterator(entry.path()))
                    {
                        if (subEntry.path().filename() == "prefs.js")
                        {
                            profiles.push_back(dirName);
                            break;
                        }
                    }
                }
            }

            renderOptions(profiles, selectedProfile, bodyFont);

            profilePath = (std::filesystem::path(profileFolderPath) / profiles[selectedProfile]).string();

            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            ImGui::PushFont(bodyFont);
            ImGui::Checkbox("Show unused profiles", &showHiddenProfiles);
            ImGui::PopFont();

            renderFooter(mediumFont, uiScale, io.DisplaySize);
        }
        else if (state == State::FIVE)
        {
            if (std::filesystem::exists(std::filesystem::path(profilePath) / "chrome" / "JS"))
            {
                renderHeader(titleFont, timeDiff);
                renderStepHeader("Old Sine installation detected:", mediumFont, timeDiff);
                ImGui::PushFont(bodyFont);
                if (std::filesystem::exists(std::filesystem::path(profilePath) / "chrome" / "sine-mods"))
                {
                    ImGui::Checkbox("Save old mods", &shouldSaveData);
                }
                ImGui::Checkbox("Reinstall bootloader", &reinstallBoot);
                ImGui::Checkbox("Uninstall Sine (will not reinstall)", &shouldUninstall);
                ImGui::PopFont();
                if (shouldUninstall)
                {
                    ImGui::PushFont(bodyFont);
                    ImGui::BeginDisabled(shouldNotify == 4);
                    ImGui::Button("Tell us why");
                    ImGui::EndDisabled();
                    ImGui::PopFont();
                    if (ImGui::IsItemDeactivated())
                    {
                        shouldNotify += 1;
                    }

                    if (shouldNotify > 0)
                    {
                        ImGui::PushFont(lightFont);
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

                        ImGui::Text("jk :) We don't collect telemetry, but we will miss you.");
                        if (shouldNotify > 1)
                        {
                            ImGui::Text("lol, no need to tell us why, we don't collect telemetry.");
                            if (shouldNotify > 2)
                            {
                                ImGui::Text("Okay, you can stop now. :(");
                                if (shouldNotify > 3)
                                {
                                    ImGui::Text("Alright, here's a text field, just put whatever you want in it.");
                                    ImGui::PopStyleColor();
                                    ImGui::InputText("##joke", reason, IM_ARRAYSIZE(reason));
                                }
                            }
                        }

                        if (shouldNotify != 4)
                        {
                            ImGui::PopStyleColor();
                        }
                        ImGui::PopFont();
                    }
                }
                renderFooter(mediumFont, uiScale, io.DisplaySize);
            }
            else
            {
                state = State::SIX;
            }
        }
        else if (state == State::SIX)
        {
            renderHeader(titleFont, timeDiff);
            
            if (!std::filesystem::exists(std::filesystem::path(profilePath) / "chrome"))
            {
                std::filesystem::create_directory(std::filesystem::path(profilePath) / "chrome");
            }

            std::vector<char*> steps;
            if (shouldUninstall)
            {
                steps.insert(steps.end(), {
                    "Cleaning up your browser...",
                    "Cleaning up your profile...",
                    "Removing mods...",
                    "Clearing startup cache..."
                });
            }
            else
            {
                if (reinstallBoot)
                {
                    steps.insert(steps.end(), {
                        "Downloading program.zip...",
                        "Configuring your browser..."
                    });
                }

                steps.insert(steps.end(), {
                    "Downloading profile.zip...",
                    "Downloading engine.zip...",
                    "Downloading locales.zip...",
                    "Cleaning up your profile...",
                    "Configuring your profile...",
                    "Removing mods...",
                    "Clearing startup cache...",
                    "Cleaning up...",
                });
            }
            steps.insert(steps.end(), { "Finished." });

            if (needsAdmin == -1)
            {
                needsAdmin = ((reinstallBoot || shouldUninstall) && !canWriteToFolder(browserPathStr)) || !canWriteToFolder(profilePath) ? 1 : 0;
            }

            bool hasPerms = isAdmin || !needsAdmin;
            bool browserOpen = isProcessRunning(toLowercase(browsers[selectedBrowser].first) + (getOS() == "win32" ? ".exe" : ""));

            if (hasPerms && (!browserOpen || !showExitScreen))
            {
                renderStepHeader(steps[installStep], mediumFont, timeDiff);
                const float totalWidth = ImGui::GetContentRegionAvail().x;
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::ProgressBar((installStep + 1) / (float)steps.size(), ImVec2(totalWidth * 0.6f, 30));
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();

                if (strstr(steps[installStep], "program.zip") != nullptr)
                {
                    const std::string fileName = "/program.zip";
                    downloadFile(bootloaderReleases + bootVersion + fileName, downloadsFolder + fileName);
                }
                else if (strstr(steps[installStep], "Configuring your browser") != nullptr)
                {
                    extractZip(downloadsFolder + "/program.zip", browserPathStr);
                }
                else if (strstr(steps[installStep], "profile.zip") != nullptr)
                {
                    const std::string fileName = "/profile.zip";
                    downloadFile(bootloaderReleases + bootVersion + fileName, downloadsFolder + fileName);
                }
                else if (strstr(steps[installStep], "engine.zip") != nullptr)
                {
                    const std::string fileName = "/engine.zip";
                    downloadFile(sineReleases + sineVersion + fileName, downloadsFolder + fileName);
                }
                else if (strstr(steps[installStep], "locales.zip") != nullptr)
                {
                    const std::string fileName = "/locales.zip";
                    downloadFile(sineReleases + sineVersion + fileName, downloadsFolder + fileName);
                }
                else if (strstr(steps[installStep], "Configuring your profile") != nullptr)
                {
                    extractZip(downloadsFolder + "/profile.zip", profilePath + "/chrome");
                    extractZip(downloadsFolder + "/engine.zip", profilePath + "/chrome");
                    extractZip(downloadsFolder + "/locales.zip", profilePath + "/chrome");

                    std::ofstream file(profilePath + "/prefs.js", std::ios::app);
                    file <<
                        ("user_pref(\"sine.is-cosine\", " + std::string(isCosine ? "true" : "false") + ");") << std::endl <<
                        ("user_pref(\"sine.version\", \"" + sineVersion + "\");") << std::endl <<
                        ("user_pref(\"sine.latest-version\", \"" + sineVersion + "\");") << std::endl;
                    file.close();
                }
                else if (strstr(steps[installStep], "Cleaning up your browser") != nullptr)
                {
                    std::filesystem::remove(browserPathStr + "/defaults/pref/config-prefs.js");
                    std::filesystem::remove(browserPathStr + "config.js");
                }
                else if (strstr(steps[installStep], "Cleaning up your profile") != nullptr)
                {
                    removeDir(profilePath + "/chrome/JS");
                    removeDir(profilePath + "/chrome/utils");
                    removeDir(profilePath + "/chrome/locales");
                }
                else if (
                    !shouldSaveData && strstr(steps[installStep], "Removing mods") != nullptr &&
                    std::filesystem::exists(std::filesystem::path(profilePath) / "chrome" / "sine-mods")
                )
                {
                    std::filesystem::remove_all(profilePath + "/chrome/sine-mods");
                }
                else if (strstr(steps[installStep], "Clearing startup cache") != nullptr)
                {
                    if (getOS() == "win32")
                    {
                        size_t pos = profilePath.find("Roaming");
                        removeDir(profilePath.replace(pos, 7, "Local") + "/startupCache");
                    }
                    else if (getOS() == "darwin")
                    {
                        size_t pos = profilePath.find("Application Support");
                        removeDir(profilePath.replace(pos, 19, "Caches") + "/startupCache");
                    }
                }
                else if (strstr(steps[installStep], "Cleaning up") != nullptr)
                {
                    std::filesystem::remove(downloadsFolder + "/program.zip");
                    std::filesystem::remove(downloadsFolder + "/profile.zip");
                    std::filesystem::remove(downloadsFolder + "/engine.zip");
                    std::filesystem::remove(downloadsFolder + "/locales.zip");
                }
                else
                {
                    ImGui::Dummy(ImVec2(0.0f, 20.0f));
                    ImGui::PushFont(lightFont);
                    ImGui::Text("If Sine does not appear in the settings page, you may need to clear startup cache");
                    ImGui::Text("(visit about:support and click 'Clear Startup Cache', you must do this on Linux).");
                    ImGui::PopFont();
                }

                if (installStep != steps.size() - 1)
                {
                    installStep += 1;
                }
            }
            else if (browserOpen && showExitScreen)
            {
                renderStepHeader("Please close your browser before installing.", mediumFont, timeDiff);
                ImGui::PushFont(lightFont);
                ImGui::Text("Listening for browser to be closed...");
                ImGui::PopFont();
            }
            else if (shouldTryAdmin)
            {
                shouldTryAdmin = false;
                if (requestAdmin(browserPathStr, profilePath, shouldSaveData, shouldUninstall, reinstallBoot, showExitScreen))
                {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }
            else
            {
                renderStepHeader("Failed to gain required privileges.", mediumFont, timeDiff);
                if (ImGui::Button("Retry"))
                {
                    shouldTryAdmin = true;
                }
            }

            renderFooter(mediumFont, uiScale, io.DisplaySize, (!hasPerms && !shouldTryAdmin) || steps.size() != installStep + 1, hasPerms);

            if (steps.size() == installStep + 1 && !showExitScreen)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
        else if (state == State::LAST && showExitScreen)
        {
            if (shouldReset)
            {
                begin = high_resolution_clock::now();
                shouldReset = false;
            }

            const char* transitionHeader = "meet your new internet.";
            float textPosX = getCenteredText(transitionHeader);
            colorFade(timeDiff, { 500, 0 }, { 300, 0 });
            ImGui::PushTextWrapPos(textPosX);
            ImGui::PushFont(titleFont);
            ImGui::Text(transitionHeader);
            ImGui::PopFont();
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            bool transitionFinished = false;
            if (timeDiff >= 1500)
            {
                transitionFinished = true;
            }

            renderFooter(mediumFont, uiScale, io.DisplaySize, !transitionFinished);
        }
        else
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::End();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
