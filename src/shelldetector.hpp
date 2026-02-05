// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Shell Detector Component Header
// 
// This header defines the ShellDetector class, which is responsible for
// identifying the user's current shell environment. It provides multiple
// detection methods and utilities for shell-specific operations including
// configuration file path resolution and home directory expansion.
// ------------------------------------------------------------------------------

#ifndef SHELLDETECTOR_HPP
#define SHELLDETECTOR_HPP

#include <string>
#include <string_view>

class ShellDetector {
public:
    // --------------------------------------------------------------------------
    // Shell Enumeration
    // Represents supported shell types in the application
    // --------------------------------------------------------------------------
    enum class Shell {
        BASH,     // Bourne Again SHell (Linux/macOS default)
        ZSH,      // Z Shell (macOS default since Catalina)
        FISH,     // Friendly Interactive SHell (modern alternative)
        UNKNOWN   // Could not determine shell type
    };
    
    // --------------------------------------------------------------------------
    // Primary Detection Methods
    // --------------------------------------------------------------------------
    
    // Main detection function - tries multiple strategies
    // Returns: Detected shell type, defaults to BASH if uncertain
    static Shell detectShell();
    
    // Get shell-specific configuration file path
    // Parameters: shell - Shell type to get path for
    // Returns: Absolute path to configuration file
    static std::string getConfigFilePath(Shell shell);
    
    // Convert Shell enum to human-readable name
    // Returns: String representation of shell type
    static std::string getShellName(Shell shell);
    
    // --------------------------------------------------------------------------
    // Advanced Detection Methods (can be used individually)
    // --------------------------------------------------------------------------
    
    // Detect shell from environment variables
    // Checks: SHELL, BASH_VERSION, ZSH_VERSION, FISH_VERSION
    static Shell detectFromEnvironment();
    
    // Detect shell from existing configuration files
    // Checks: ~/.bashrc, ~/.zshrc, ~/.config/fish/config.fish
    static Shell detectFromConfigFiles();
    
    // Get parent process name (for fallback detection)
    // Returns: Name of parent process, often the shell
    static std::string getParentProcess();
    
    // --------------------------------------------------------------------------
    // Utility Methods
    // --------------------------------------------------------------------------
    
    // Expand tilde (~) to home directory path
    // Parameters: path - Path potentially starting with ~
    // Returns: Expanded path with ~ replaced by home directory
    static std::string expandHome(const std::string& path);
    
private:
    // --------------------------------------------------------------------------
    // Configuration File Names (Private Constants)
    // --------------------------------------------------------------------------
    static constexpr std::string_view BASHRC = ".bashrc";
    static constexpr std::string_view ZSHRC = ".zshrc";
    static constexpr std::string_view FISH_CONFIG = ".config/fish/config.fish";
};

#endif // SHELLDETECTOR_HPP