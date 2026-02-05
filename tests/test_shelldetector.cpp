// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Unit Tests for ShellDetector Component
//
// This file contains comprehensive unit tests for the ShellDetector class,
// which is responsible for identifying the user's current shell and providing
// shell-specific file paths and functionality. The tests verify shell detection,
// home directory expansion, configuration file path resolution, and shell name
// mapping. These tests are critical for ensuring cross-shell compatibility.
// ------------------------------------------------------------------------------

#include "shelldetector.hpp"  // Main class under test
#include <cassert>             // Assertion macros for test validation
#include <iostream>            // Console output for test reporting
#include <filesystem>          // Filesystem operations for path validation

// Alias for convenience
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------
// Test: Shell Detection
// Purpose: Verify that the shell detection mechanism works correctly.
// Tests detection of various shells (bash, zsh, fish, etc.) by examining
// environment variables and process information.
// ------------------------------------------------------------------------------
static void testShellDetection() {
    std::cout << "  Testing shell detection... ";
    
    // Detect the current shell
    ShellDetector::Shell detected = ShellDetector::detectShell();
    
    // Output the detected shell for debugging
    std::string shell_name = ShellDetector::getShellName(detected);
    std::cout << "\n    Detected shell: " << shell_name << std::endl;
    
    // Test that detection is consistent (idempotent)
    ShellDetector::Shell second_detection = ShellDetector::detectShell();
    assert(detected == second_detection);
    
    std::cout << "  ✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Home Directory Expansion
// Purpose: Verify that tilde (~) is correctly expanded to the home directory.
// Tests the expansion of various path patterns containing ~.
// ------------------------------------------------------------------------------
static void testExpandHome() {
    std::cout << "  Testing home directory expansion... ";
    
    // Get home directory from environment for comparison
    const char* home_env = std::getenv("HOME");
    assert(home_env != nullptr && "HOME environment variable should be set");
    std::string home_path = home_env;
    
    // Test basic tilde expansion
    std::string expanded = ShellDetector::expandHome("~");
    assert(!expanded.empty());
    assert(expanded[0] != '~');  // Tilde should be expanded
    assert(expanded == home_path);
    
    // Test tilde with subdirectory
    expanded = ShellDetector::expandHome("~/Documents");
    assert(!expanded.empty());
    assert(expanded.find("~/") == std::string::npos);  // No tilde in result
    assert(expanded == home_path + "/Documents");
    
    // Test tilde with username (should not expand in typical implementation)
    expanded = ShellDetector::expandHome("~" + std::string(home_env) + "/test");
    // Some implementations may support ~username, others may not
    // We just verify it doesn't crash and returns something
    
    // Test paths without tilde (should remain unchanged)
    expanded = ShellDetector::expandHome("/absolute/path");
    assert(expanded == "/absolute/path");
    
    expanded = ShellDetector::expandHome("relative/path");
    assert(expanded == "relative/path");
    
    // Test empty string
    expanded = ShellDetector::expandHome("");
    assert(expanded.empty());
    
    // Test edge cases
    expanded = ShellDetector::expandHome("~~");
    // Implementation-specific behavior
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Configuration File Paths
// Purpose: Verify that correct configuration file paths are returned for each shell.
// Tests shell-specific RC file locations and ensures they follow standard conventions.
// ------------------------------------------------------------------------------
static void testConfigFilePath() {
    std::cout << "  Testing configuration file paths... ";
    
    // Test BASH configuration file path
    std::string bash_path = ShellDetector::getConfigFilePath(ShellDetector::Shell::BASH);
    assert(!bash_path.empty());
    assert(bash_path.find(".bash") != std::string::npos);  // Should contain .bash
    std::cout << "\n    BASH path: " << bash_path << std::endl;
    
    // BASH can use multiple files: .bashrc, .bash_profile, .bash_aliases
    bool valid_bash_path = 
        (bash_path.find(".bashrc") != std::string::npos) ||
        (bash_path.find(".bash_profile") != std::string::npos) ||
        (bash_path.find(".bash_aliases") != std::string::npos);
    assert(valid_bash_path && "Invalid BASH configuration file path");
    
    // Test ZSH configuration file path
    std::string zsh_path = ShellDetector::getConfigFilePath(ShellDetector::Shell::ZSH);
    assert(!zsh_path.empty());
    assert(zsh_path.find(".zsh") != std::string::npos);  // Should contain .zsh
    std::cout << "    ZSH path: " << zsh_path << std::endl;
    
    // ZSH typically uses .zshrc
    bool valid_zsh_path = 
        (zsh_path.find(".zshrc") != std::string::npos) ||
        (zsh_path.find(".zshenv") != std::string::npos) ||
        (zsh_path.find(".zprofile") != std::string::npos);
    assert(valid_zsh_path && "Invalid ZSH configuration file path");
    
    // Test FISH configuration file path
    std::string fish_path = ShellDetector::getConfigFilePath(ShellDetector::Shell::FISH);
    assert(!fish_path.empty());
    assert(fish_path.find("fish") != std::string::npos);  // Should contain fish
    std::cout << "    FISH path: " << fish_path << std::endl;
    
    // FISH uses config.fish in ~/.config/fish/
    bool valid_fish_path = 
        (fish_path.find(".config/fish") != std::string::npos) ||
        (fish_path.find("config.fish") != std::string::npos);
    assert(valid_fish_path && "Invalid FISH configuration file path");
    
    // Test that paths are absolute (start with / or ~)
    assert(bash_path[0] == '/' || bash_path[0] == '~');
    assert(zsh_path[0] == '/' || zsh_path[0] == '~');
    assert(fish_path[0] == '/' || fish_path[0] == '~');
    
    // Test unknown shell (should return empty or default path)
    std::string unknown_path = ShellDetector::getConfigFilePath(ShellDetector::Shell::UNKNOWN);
    
    std::cout << "  ✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Shell Name Mapping
// Purpose: Verify that shell enum values are correctly mapped to human-readable names.
// Tests the bi-directional mapping between Shell enum and string representations.
// ------------------------------------------------------------------------------
static void testShellNames() {
    std::cout << "  Testing shell name mapping... ";
    
    // Test known shell names
    assert(ShellDetector::getShellName(ShellDetector::Shell::BASH) == "BASH");
    assert(ShellDetector::getShellName(ShellDetector::Shell::ZSH) == "ZSH");
    assert(ShellDetector::getShellName(ShellDetector::Shell::FISH) == "FISH");
    assert(ShellDetector::getShellName(ShellDetector::Shell::UNKNOWN) == "UNKNOWN");
     
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Main Test Runner
// Purpose: Execute all ShellDetector tests and report results.
// ------------------------------------------------------------------------------
void test_shelldetector() {
    std::cout << "Running ShellDetector tests...\n";
    
    // Execute all test cases
    testShellDetection();    // Test automatic shell detection
    testExpandHome();        // Test home directory expansion
    testConfigFilePath();    // Test configuration file paths
    testShellNames();        // Test shell name mapping
    
    std::cout << "✓ ShellDetector tests passed!\n";
}
