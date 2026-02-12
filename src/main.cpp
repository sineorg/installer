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
#include <thread>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include <aclapi.h>
#include <tlhelp32.h>
#include <sddl.h>

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
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/file.h>
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

struct ProcessHandle
{
#ifdef _WIN32
    HANDLE handle = nullptr;
#else
    pid_t pid = -1;
#endif

    bool valid() const
    {
#ifdef _WIN32
        return handle != nullptr;
#else
        return pid > 0;
#endif
    }

    // Wait for process to finish (blocking)
    void wait()
    {
#ifdef _WIN32
        if (handle)
        {
            WaitForSingleObject(handle, INFINITE);
            CloseHandle(handle);
            handle = nullptr;
        }
#else
        if (pid > 0)
        {
            int status;
            waitpid(pid, &status, 0);
            pid = -1;
        }
#endif
    }
};

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
#elif defined(__linux__) || defined(__linux)
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
            if (path.contains("/snap/"))
            {
                return "/root/snap/firefox";
            }
            return path;
        }
    }
    return "";
}

std::string getProfileLocation(int browserIndex) {
    std::string os = getOS();
    std::vector<std::string> profilePaths = browsers[browserIndex].second[0].second.find(os)->second;

    std::filesystem::path home;
    if (os == "win32")
    {
        home = std::getenv("APPDATA");
    }
    else
    {
        home = std::getenv("HOME");
    }

    for (std::string profilePath : profilePaths)
    {
        if (os == "win32")
        {
            profilePath = (home / profilePath / "Profiles").string();
        }
        else
        {
            if (os == "darwin")
            {
                profilePath = (home / "Library" / "Application Support" / profilePath / "Profiles").string();
            }
            else if (os == "linux")
            {
                profilePath = (home / profilePath).string();
            }
        }

        if (std::filesystem::exists(profilePath))
        {
            return profilePath;
        }
    }

    return "";
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

std::string getDownloadsFolder()
{
#ifdef _WIN32
    const char* userProfile = std::getenv("USERPROFILE");
    return std::string(userProfile) + "\\Downloads\\";
#else
    const char* home = std::getenv("HOME");
    return std::string(home) + "/Downloads/";
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
    if (!curl) {
        std::cerr << "Failed to init curl: " << std::endl;
        return false;
    }

    std::ofstream file(outputPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << outputPath << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    file.close();

    return (res == CURLE_OK);
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

#ifdef _WIN32
std::wstring s2ws(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}
#endif


ProcessHandle launchProcess(
    const std::string& targetPath,
    const std::string& browserPath,
    const std::string& profilePath,
    bool shouldSaveData,
    bool shouldUninstall,
    bool reinstallBoot
)
{
    ProcessHandle ph;

#ifdef _WIN32
    std::string args;
    if (shouldSaveData) args += "-s ";
    if (shouldUninstall) args += "-u ";
    args += "--browser \"" + browserPath + "\" ";
    args += "--profile \"" + profilePath + "\" ";
    if (!reinstallBoot) args += "--no-boot ";
    args += "--bootloader \"" + bootVersion + "\" ";
    args += "--version \"" + sineVersion + "\"";

    std::string cmdLine = "\"" + targetPath + "\" " + args;

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessA(
            nullptr,
            cmdLine.data(),
            nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW,
            nullptr, nullptr,
            &si, &pi
        ))
    {
        CloseHandle(pi.hThread);
        ph.handle = pi.hProcess;
    }
#else
    pid_t pid = fork();
    if (pid < 0)
        return ph;
    
    if (pid == 0) // child
    {
        std::vector<std::string> args;
        args.push_back("gnome-terminal");  // or "xterm" if gnome-terminal is missing
        args.push_back("--");
        args.push_back("bash");
        args.push_back("-c");
    
        // Build the command string
        std::string cmd = targetPath;
        if (shouldSaveData) cmd += " -s";
        if (shouldUninstall) cmd += " -u";
        cmd += " --browser " + browserPath;
        cmd += " --profile " + profilePath;
        if (!reinstallBoot) cmd += " --no-boot";
        cmd += " --bootloader " + bootVersion;
        cmd += " --version " + sineVersion;
    
        // Keep terminal alive forever
        cmd += "; while true; do sleep 60; done";
    
        args.push_back(cmd);
    
        // Convert args to char* array for execvp
        std::vector<char*> argv;
        for (auto& s : args)
            argv.push_back(const_cast<char*>(s.c_str()));
        argv.push_back(nullptr);
    
        execvp("gnome-terminal", argv.data());
        perror("execvp failed"); // log if terminal fails to launch
        _exit(1);
    }
    
    // parent process
    ph.pid = pid;
#endif

    return ph;
}

void installSine(
    ImFont*& titleFont, ImFont*& mediumFont, ImFont*& lightFont,
    float timeDiff, bool shouldUninstall,
    std::string& profilePath, bool reinstallBoot,
    std::string& browserPathStr, int selectedBrowser,
    int& installStep, bool shouldSaveData,
    float uiScale, ImGuiIO& io, ProcessHandle& processHandle
)
{
    renderHeader(titleFont, timeDiff);

    if (!std::filesystem::exists(std::filesystem::path(profilePath) / "chrome"))
    {
        std::filesystem::create_directory(std::filesystem::path(profilePath) / "chrome");
    }

    const std::string updaterName = std::string("updater.") + (getOS() == "win32" ? "bat" : "sh");
    const std::string filePath = getDownloadsFolder() + updaterName;

    std::vector<const char*> steps;
    steps.push_back("Launching manager...");
    if (shouldUninstall)
    {
        steps.insert(steps.end(), {
            "Cleaning up your browser...",
            "Cleaning up your profile..."
        });
    }
    else
    {
        if (reinstallBoot)
        {
            steps.insert(steps.end(), {
                "Downloading program.zip...",
                "Cleaning up your browser...",
                "Configuring your browser..."
            });
        }

        steps.insert(steps.end(), {
            "Downloading profile.zip...",
            "Downloading engine.zip...",
            "Downloading locales.zip...",
            "Configuring your profile..."
        });
    }
    if (!shouldSaveData)
    {
        steps.push_back("Removing mods...");
    }
    steps.push_back("Clearing startup cache...");
    if (!shouldUninstall)
    {
        steps.push_back("Clearing up...");
    }
    steps.push_back("Finished.");

    bool browserOpen = isProcessRunning(toLowercase(browsers[selectedBrowser].first) + (getOS() == "win32" ? ".exe" : ""));
    if (browserOpen)
    {
        renderStepHeader("Please close your browser before installing.", mediumFont, timeDiff);
        ImGui::PushFont(lightFont);
        ImGui::Text("Listening for browser to be closed...");
        ImGui::PopFont();
    }
    else
    {
        renderStepHeader(steps[installStep], mediumFont, timeDiff);
        const float totalWidth = ImGui::GetContentRegionAvail().x;
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::ProgressBar((installStep + 1) / (float)steps.size(), ImVec2(totalWidth * 0.6f, 30));
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        if (steps[installStep] == "Clearing startup cache...")
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
        else if (steps[installStep - 1] == "Launching manager...")
        {
            downloadFile("https://github.com/CosmoCreeper/Sine/releases/download/v" + sineVersion + "/" + updaterName, filePath);
            processHandle = launchProcess(filePath, browserPathStr, profilePath, shouldSaveData, shouldUninstall, reinstallBoot);
        }
        else if (steps[installStep] == "Finished.")
        {
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::PushFont(lightFont);
            ImGui::Text("If Sine does not appear in the settings page, you may need to clear startup cache");
            ImGui::Text("(visit about:support and click 'Clear Startup Cache', you must do this on Linux).");
            ImGui::PopFont();
        }

        std::this_thread::sleep_for(750ms);

        if (installStep < steps.size() - 1)
        {
            installStep += 1;
            if (installStep == steps.size() - 1)
            {
                processHandle.wait();
                std::remove(filePath.c_str());
            }
        }
    }

    renderFooter(mediumFont, uiScale, io.DisplaySize, steps.size() != installStep + 1);
}

int main(int argc, char* argv[])
{
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
    int installStep = 0;
    ProcessHandle processHandle;

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

    if (!browserPathStr.empty() && !profilePath.empty())
    {
        state = State::SIX;
    }

    std::error_code ec;

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
            else {
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
            if (getOS() != "linux")
            {
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
            if (std::filesystem::exists(std::filesystem::path(profilePath) / "chrome" / "JS", ec))
            {
                renderHeader(titleFont, timeDiff);
                renderStepHeader("Old Sine installation detected:", mediumFont, timeDiff);
                ImGui::PushFont(bodyFont);
                if (std::filesystem::exists(std::filesystem::path(profilePath) / "chrome" / "sine-mods", ec))
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
            // Install Sine.
            installSine(
                titleFont, mediumFont, lightFont,
                timeDiff, shouldUninstall,
                profilePath, reinstallBoot,
                browserPathStr, selectedBrowser,
                installStep, shouldSaveData,
                uiScale, io, processHandle
            );
        }
        else if (state == State::LAST)
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
