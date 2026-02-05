// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Shell Detector Component Implementation
// 
// This file implements comprehensive shell detection using multiple strategies:
// 1. Environment variable analysis
// 2. Configuration file presence
// 3. Parent process inspection
// It provides robust fallback mechanisms and utilities for shell-specific paths.
// ------------------------------------------------------------------------------

#include "shelldetector.hpp"
#include <cstdlib>       // For std::getenv
#include <utility>       // For std::pair
#include <filesystem>    // For filesystem operations
#include <fstream>       // For file reading
#include <iostream>      // For debugging output
#include <algorithm>     // For string searching
#include <vector>        // For collections
#include <unistd.h>      // For getuid, getppid
#include <pwd.h>         // For getpwuid
#include <string_view>   // For efficient string views

// Alias for convenience
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------
// Main Shell Detection Function
// Uses multiple detection strategies with fallback order:
// 1. Environment variable detection (fastest, most reliable)
// 2. Configuration file detection (checks existing configs)
// 3. Parent process detection (checks running processes)
// 4. Default to BASH (safest fallback)
// ------------------------------------------------------------------------------
ShellDetector::Shell ShellDetector::detectShell() {
    // Strategy 1: Check environment variables (fastest)
    if (auto shell = detectFromEnvironment(); shell != Shell::UNKNOWN) {
        return shell;
    }
    
    // Strategy 2: Check for existing configuration files
    if (auto shell = detectFromConfigFiles(); shell != Shell::UNKNOWN) {
        return shell;
    }
    
    // Strategy 3: Check parent process (if available on Unix-like systems)
    std::string parent = getParentProcess();
    if (!parent.empty()) {
        // Case-insensitive substring matching
        std::string parentLower = parent;
        std::transform(parentLower.begin(), parentLower.end(), 
                      parentLower.begin(), ::tolower);
        
        if (parentLower.find("zsh") != std::string::npos) return Shell::ZSH;
        if (parentLower.find("bash") != std::string::npos) return Shell::BASH;
        if (parentLower.find("fish") != std::string::npos) return Shell::FISH;
    }
    
    // Strategy 4: Default to BASH (most widely available)
    // Could also check for SHELL env variable without shell name
    if (const char* shellEnv = std::getenv("SHELL"); shellEnv != nullptr) {
        // If SHELL is set but we couldn't identify it, still return BASH as fallback
        std::cout << "Warning: Using BASH fallback (detected SHELL=" 
                  << shellEnv << ")" << std::endl;
    }
    
    return Shell::BASH;
}

// ------------------------------------------------------------------------------
// Environment Variable Detection
// Checks shell-specific environment variables for detection
// ------------------------------------------------------------------------------
ShellDetector::Shell ShellDetector::detectFromEnvironment() {
    // Check for shell-specific version variables (most reliable)
    if (const char* bashVer = std::getenv("BASH_VERSION"); bashVer != nullptr) {
        return Shell::BASH;
    }
    
    if (const char* zshVer = std::getenv("ZSH_VERSION"); zshVer != nullptr) {
        return Shell::ZSH;
    }
    
    if (const char* fishVer = std::getenv("FISH_VERSION"); fishVer != nullptr) {
        return Shell::FISH;
    }
    
    // Fallback: Check SHELL environment variable
    if (const char* shellEnv = std::getenv("SHELL"); shellEnv != nullptr) {
        std::string_view shellPath(shellEnv);
        
        // Extract basename from path (handle paths like /bin/bash, /usr/local/bin/zsh)
        size_t lastSlash = shellPath.find_last_of('/');
        std::string_view shellName = (lastSlash != std::string_view::npos) 
                                   ? shellPath.substr(lastSlash + 1) 
                                   : shellPath;
        
        // Check for shell names (case-insensitive)
        // Convert to lowercase for comparison
        std::string shellNameLower(shellName);
        std::transform(shellNameLower.begin(), shellNameLower.end(),
                      shellNameLower.begin(), ::tolower);
        
        if (shellNameLower.find("zsh") != std::string::npos) return Shell::ZSH;
        if (shellNameLower.find("bash") != std::string::npos) return Shell::BASH;
        if (shellNameLower.find("fish") != std::string::npos) return Shell::FISH;
    }
    
    return Shell::UNKNOWN;
}

// ------------------------------------------------------------------------------
// Configuration File Detection
// Detects shell based on which configuration files exist
// Useful when shell is started without version variables
// ------------------------------------------------------------------------------
ShellDetector::Shell ShellDetector::detectFromConfigFiles() {
    std::string home = expandHome("~");
    
    // List of shells and their configuration files to check
    // Ordered by popularity/reliability
    constexpr std::pair<Shell, std::string_view> configs[] = {
        {Shell::BASH, BASHRC},
        {Shell::ZSH, ZSHRC},
        {Shell::FISH, FISH_CONFIG}
    };
    
    for (const auto& [shell, config] : configs) {
        std::string configPath = home + "/" + std::string(config);
        
        // Check if config file exists and is readable
        if (fs::exists(configPath)) {
            return shell;
        }
    }
    
    return Shell::UNKNOWN;
}

// ------------------------------------------------------------------------------
// Parent Process Detection (Linux/Unix specific)
// Reads /proc/<pid>/comm to get parent process name
// Useful when running from scripts or subshells
// ------------------------------------------------------------------------------
std::string ShellDetector::getParentProcess() {
    // Get parent process ID
    pid_t parentPid = getppid();
    
    // Read process name from /proc (Linux-specific)
    std::string procPath = "/proc/" + std::to_string(parentPid) + "/comm";
    
    std::ifstream procFile(procPath);
    if (procFile.is_open()) {
        std::string processName;
        
        // Read process name (single line in comm file)
        if (std::getline(procFile, processName)) {
            // Remove trailing newline if present
            if (!processName.empty() && processName.back() == '\n') {
                processName.pop_back();
            }
            return processName;
        }
    }
    
    // Fallback for systems without /proc (macOS, BSD)
    // Could use ps command: ps -p <pid> -o comm=
    std::string psCommand = "ps -p " + std::to_string(parentPid) + " -o comm= 2>/dev/null";
    
    FILE* pipe = popen(psCommand.c_str(), "r");
    if (pipe) {
        char buffer[128];
        std::string result;
        
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        
        pclose(pipe);
        
        // Remove trailing whitespace
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        
        return result;
    }
    
    return "";
}

// ------------------------------------------------------------------------------
// Home Directory Expansion
// Expands tilde (~) to absolute home directory path
// Supports: ~, ~/path, ~username (with limitations)
// ------------------------------------------------------------------------------
std::string ShellDetector::expandHome(const std::string& path) {
    // Return unchanged if path doesn't start with tilde
    if (path.empty() || path[0] != '~') {
        return path;
    }
    
    // Get home directory
    const char* homeDir = nullptr;
    
    // Try environment variable first (respects user overrides)
    homeDir = std::getenv("HOME");
    
    // Fallback to password database
    if (homeDir == nullptr) {
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) {
            homeDir = pw->pw_dir;
        } else {
            // Last resort: return path unchanged
            return path;
        }
    }
    
    // Handle ~ alone
    if (path.length() == 1) {
        return std::string(homeDir);
    }
    
    // Handle ~username (limited support - requires getpwnam)
    if (path[1] != '/') {
        // Extract username after tilde
        size_t slashPos = path.find('/');
        std::string username = (slashPos == std::string::npos) 
                             ? path.substr(1) 
                             : path.substr(1, slashPos - 1);
        
        // Look up user's home directory
        struct passwd* userPw = getpwnam(username.c_str());
        if (userPw != nullptr) {
            homeDir = userPw->pw_dir;
            // Append remaining path if any
            if (slashPos != std::string::npos) {
                return std::string(homeDir) + path.substr(slashPos);
            } else {
                return std::string(homeDir);
            }
        } else {
            // User not found, return path unchanged
            return path;
        }
    }
    
    // Handle ~/path (most common case)
    return std::string(homeDir) + path.substr(1);
}

// ------------------------------------------------------------------------------
// Get Configuration File Path
// Returns absolute path to shell-specific configuration file
// ------------------------------------------------------------------------------
std::string ShellDetector::getConfigFilePath(Shell shell) {
    std::string home = expandHome("~");
    
    switch (shell) {
        case Shell::BASH:
            // BASH can use multiple files, default to .bashrc
            // Could check for .bash_profile or .bash_aliases
            return home + "/" + std::string(BASHRC);
            
        case Shell::ZSH:
            // ZSH typically uses .zshrc
            return home + "/" + std::string(ZSHRC);
            
        case Shell::FISH:
            // FISH uses config.fish in .config/fish directory
            return home + "/" + std::string(FISH_CONFIG);
            
        case Shell::UNKNOWN:
            return "";  // Empty string for unknown shell
            
        default:
            // Should never reach here, but include for completeness
            return "";
    }
}

// ------------------------------------------------------------------------------
// Get Shell Name as String
// Converts Shell enum to human-readable name
// ------------------------------------------------------------------------------
std::string ShellDetector::getShellName(Shell shell) {
    switch (shell) {
        case Shell::BASH:
            return "BASH";
        case Shell::ZSH:
            return "ZSH";
        case Shell::FISH:
            return "FISH";
        case Shell::UNKNOWN:
            return "UNKNOWN";
        default:
            // Should never reach here with exhaustive switch
            return "UNKNOWN";
    }
}