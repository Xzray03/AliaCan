// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Configuration File Handler Component Header
// 
// This header defines the ConfigFileHandler class, which manages shell
// configuration files (like .bashrc, .zshrc, config.fish). It provides
// comprehensive operations for loading, adding, removing, and managing
// aliases within these configuration files with proper shell-specific
// syntax handling.
// ------------------------------------------------------------------------------

#ifndef CONFIGFILEHANDLER_HPP
#define CONFIGFILEHANDLER_HPP

#include <string>
#include <vector>
#include "aliasmanager.hpp"
#include "shelldetector.hpp"

class ConfigFileHandler {
public:
    // --------------------------------------------------------------------------
    // Constructor
    // --------------------------------------------------------------------------
    
    // Initialize with specific configuration file path and shell type
    ConfigFileHandler(const std::string& configFilePath, ShellDetector::Shell shell);
    
    // --------------------------------------------------------------------------
    // Alias Management
    // --------------------------------------------------------------------------
    
    // Load all aliases from the configuration file
    // Returns: Vector of Alias objects found in the file
    std::vector<Alias> loadAliases();
    
    // Add a new alias to the configuration file
    // Returns: true if alias was successfully added
    bool addAlias(const Alias& alias);
    
    // Remove an alias by name from the configuration file
    // Returns: true if alias was found and removed
    bool removeAlias(const std::string& aliasName);
    
    // --------------------------------------------------------------------------
    // File Operations
    // --------------------------------------------------------------------------
    
    // Get the absolute path to the configuration file
    // Expands home directory (~) and resolves shell-specific paths
    std::string getConfigFilePath() const;
    
    // Check if the configuration file exists
    bool configFileExists() const;
    
    // Read all lines from the configuration file
    // Returns: Vector of strings, each representing a line
    std::vector<std::string> readAllLines();
    
    // Write all lines to the configuration file
    // Replaces the entire file content
    // Returns: true if write was successful
    bool writeAllLines(const std::vector<std::string>& lines);
    
    // --------------------------------------------------------------------------
    // File Permissions and Error Handling
    // --------------------------------------------------------------------------
    
    // Check if file has appropriate read/write permissions
    // Returns: true if user can read and write the file
    bool checkPermissions() const;
    
    // Get the last error message for debugging
    std::string getLastError() const;
    
private:
    // --------------------------------------------------------------------------
    // Private Methods
    // --------------------------------------------------------------------------
    
    // Ensure the configuration file exists, create if it doesn't
    // Returns: true if file exists or was created successfully
    bool ensureFileExists();
    
    // Set appropriate file permissions (read/write for owner)
    // Returns: true if permissions were set successfully
    bool setFilePermissions();
    
    // --------------------------------------------------------------------------
    // Member Variables
    // --------------------------------------------------------------------------
    
    std::string configFilePath;     // Path to configuration file
    ShellDetector::Shell shell;     // Shell type for syntax handling
    std::string lastError;          // Last error message
    AliasManager aliasManager;      // Alias formatter/parser for this shell
};

#endif // CONFIGFILEHANDLER_HPP