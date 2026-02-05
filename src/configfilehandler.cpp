// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Configuration File Handler Component Implementation
// 
// This file implements the ConfigFileHandler class, providing robust
// operations for managing shell configuration files. It handles
// file I/O, alias parsing, syntax validation, and proper file
// permissions management across different shell environments.
// ------------------------------------------------------------------------------

#include "configfilehandler.hpp"
#include <fstream>        // File stream operations
#include <filesystem>     // Filesystem path operations
#include <sys/stat.h>     // File permission handling

// Alias for convenience
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------
// Constructor
// Initializes with configuration file path and shell type
// ------------------------------------------------------------------------------
ConfigFileHandler::ConfigFileHandler(const std::string& configFilePath, 
                                     ShellDetector::Shell shell)
    : configFilePath(configFilePath), 
      shell(shell), 
      aliasManager(shell) {
    // Store the configuration file path
    // Initialize alias manager with the specified shell type
}

// ------------------------------------------------------------------------------
// Load Aliases from Configuration File
// Parses the configuration file and extracts all alias definitions
// ------------------------------------------------------------------------------
std::vector<Alias> ConfigFileHandler::loadAliases() {
    std::vector<Alias> aliases;
    
    // Check if file exists
    if (!configFileExists()) {
        lastError = "Config file does not exist: " + configFilePath;
        return aliases;  // Return empty vector
    }
    
    // Open file for reading
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        lastError = "Cannot open config file for reading: " + configFilePath;
        return aliases;
    }
    
    // Read file line by line
    std::string line;
    while (std::getline(file, line)) {
        // Check if line contains an alias definition
        if (AliasManager::isAliasLine(line)) {
            // Parse the alias line
            Alias parsed = AliasManager::parseAliasLine(line);
            
            // Add to list if parsing succeeded
            if (!parsed.name.empty()) {
                aliases.push_back(std::move(parsed));
            }
        }
    }
    
    return aliases;
}

// ------------------------------------------------------------------------------
// Add Alias to Configuration File
// Appends a new alias definition to the end of the file
// ------------------------------------------------------------------------------
bool ConfigFileHandler::addAlias(const Alias& alias) {
    // Validate alias before adding
    if (!AliasManager::validateAliasName(alias.name) || 
        !AliasManager::validateCommand(alias.command)) {
        lastError = "Invalid alias name or command";
        return false;
    }
    
    // Ensure file exists (create if necessary)
    if (!ensureFileExists()) {
        lastError = "Cannot create config file";
        return false;
    }
    
    // Open file in append mode
    std::ofstream file(configFilePath, std::ios::app);
    if (!file.is_open()) {
        lastError = "Cannot open config file for writing";
        return false;
    }
    
    // Format alias according to shell syntax and append to file
    file << '\n' << aliasManager.formatAlias(alias);
    
    // Ensure proper file permissions
    setFilePermissions();
    
    return true;
}

// ------------------------------------------------------------------------------
// Remove Alias from Configuration File
// Removes an alias definition by name, preserving other content
// ------------------------------------------------------------------------------
bool ConfigFileHandler::removeAlias(const std::string& aliasName) {
    // Check if file exists
    if (!configFileExists()) {
        lastError = "Config file does not exist";
        return false;
    }
    
    // Read all lines from file
    auto lines = readAllLines();
    if (lines.empty()) {
        lastError = "Failed to read config file";
        return false;
    }
    
    bool found = false;
    std::vector<std::string> newLines;
    newLines.reserve(lines.size());  // Pre-allocate for efficiency
    
    // Filter out the alias to be removed
    for (auto& line : lines) {
        if (AliasManager::isAliasLine(line)) {
            Alias parsed = AliasManager::parseAliasLine(line);
            if (parsed.name == aliasName) {
                found = true;      // Mark as found
                continue;          // Skip this line (don't add to newLines)
            }
        }
        newLines.push_back(std::move(line));  // Keep this line
    }
    
    // If alias wasn't found, report error
    if (!found) {
        lastError = "Alias not found: " + aliasName;
        return false;
    }
    
    // Write filtered lines back to file
    return writeAllLines(newLines);
}

// ------------------------------------------------------------------------------
// Get Configuration File Path
// Returns shell-specific default paths if not explicitly set
// ------------------------------------------------------------------------------
std::string ConfigFileHandler::getConfigFilePath() const {
    switch (shell) {
        case ShellDetector::Shell::BASH:
            return ShellDetector::expandHome("~/.bashrc");
            
        case ShellDetector::Shell::ZSH:
            return ShellDetector::expandHome("~/.zshrc");
            
        case ShellDetector::Shell::FISH:
            return ShellDetector::expandHome("~/.config/fish/config.fish");
            
        default:
            return configFilePath;  // Return explicitly set path
    }
}

// ------------------------------------------------------------------------------
// Check if Configuration File Exists
// ------------------------------------------------------------------------------
bool ConfigFileHandler::configFileExists() const {
    return fs::exists(configFilePath);
}

// ------------------------------------------------------------------------------
// Read All Lines from Configuration File
// ------------------------------------------------------------------------------
std::vector<std::string> ConfigFileHandler::readAllLines() {
    std::vector<std::string> lines;
    
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        return lines;  // Return empty vector
    }
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(std::move(line));
    }
    
    return lines;
}

// ------------------------------------------------------------------------------
// Write All Lines to Configuration File
// Replaces entire file content
// ------------------------------------------------------------------------------
bool ConfigFileHandler::writeAllLines(const std::vector<std::string>& lines) {
    std::ofstream file(configFilePath, std::ios::trunc);
    if (!file.is_open()) {
        lastError = "Cannot open file for writing";
        return false;
    }
    
    // Write all lines, adding newline between them
    for (size_t i = 0; i < lines.size(); ++i) {
        file << lines[i];
        if (i < lines.size() - 1) {
            file << '\n';  // No newline after last line
        }
    }
    
    // Ensure proper file permissions
    setFilePermissions();
    
    return true;
}

// ------------------------------------------------------------------------------
// Check File Permissions
// Verifies that user has read and write permissions
// ------------------------------------------------------------------------------
bool ConfigFileHandler::checkPermissions() const {
    struct stat sb;
    
    // Get file status
    if (stat(configFilePath.c_str(), &sb) != 0) {
        return false;  // Can't stat file
    }
    
    // Check if user has read and write permissions
    bool canRead = (sb.st_mode & S_IRUSR) != 0;
    bool canWrite = (sb.st_mode & S_IWUSR) != 0;
    
    return canRead && canWrite;
}

// ------------------------------------------------------------------------------
// Get Last Error Message
// ------------------------------------------------------------------------------
std::string ConfigFileHandler::getLastError() const {
    return lastError;
}

// ------------------------------------------------------------------------------
// Ensure File Exists
// Creates the file if it doesn't exist
// ------------------------------------------------------------------------------
bool ConfigFileHandler::ensureFileExists() {
    if (fs::exists(configFilePath)) {
        return true;  // File already exists
    }
    
    // Create empty file
    std::ofstream file(configFilePath);
    if (!file.is_open()) {
        return false;  // Failed to create file
    }
    
    // Set appropriate permissions
    setFilePermissions();
    
    return true;
}

// ------------------------------------------------------------------------------
// Set File Permissions
// Sets permissions to: owner read/write, group read, others read
// (Unix permissions: 644 or rw-r--r--)
// ------------------------------------------------------------------------------
bool ConfigFileHandler::setFilePermissions() {
    // Set permissions: owner can read/write, group/others can only read
    int result = chmod(configFilePath.c_str(), 
                      S_IRUSR | S_IWUSR |   // User read/write
                      S_IRGRP |             // Group read
                      S_IROTH);             // Others read
    
    return result == 0;  // Return true if chmod succeeded
}