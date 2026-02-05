// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Alias Manager Component Header
// 
// This header defines the AliasManager class, which is responsible for
// managing shell aliases in the AliaCan application. It provides functionality
// for validating, formatting, parsing, and manipulating shell aliases across
// different shell types (BASH, ZSH, FISH). The class handles shell-specific
// syntax variations and ensures alias integrity through comprehensive validation.
// ------------------------------------------------------------------------------

#ifndef ALIASMANAGER_HPP
#define ALIASMANAGER_HPP

#include <string>
#include "shelldetector.hpp"

// ------------------------------------------------------------------------------
// Structure: Alias
// Purpose: Represents a single shell alias with name and command.
// Provides equality operator for easy comparison in tests and operations.
// ------------------------------------------------------------------------------
struct Alias {
    std::string name;        // Alias identifier (e.g., "ll", "gs", "gp")
    std::string command;     // Command to execute (e.g., "ls -la", "git status")
    
    // Equality operator for comparing aliases
    bool operator==(const Alias& other) const {
        return name == other.name && command == other.command;
    }
    
    std::string description;    // Human-readable description
    bool enabled;               // Whether alias is active
    std::string created_date;   // When alias was created
    std::string last_used;      // When alias was last used
};

// ------------------------------------------------------------------------------
// Class: AliasManager
// Purpose: Central manager for all alias-related operations.
// Handles validation, formatting, parsing, and shell-specific transformations.
// ------------------------------------------------------------------------------
class AliasManager {
public:
    // --------------------------------------------------------------------------
    // Constructor & Shell Management
    // --------------------------------------------------------------------------
    
    // Constructor: Initialize with specific shell type
    explicit AliasManager(ShellDetector::Shell shell);
    
    // Get current shell type
    ShellDetector::Shell getShell() const;
    
    // Change shell type (useful for testing or shell migration)
    void setShell(ShellDetector::Shell shell);
    
    // --------------------------------------------------------------------------
    // Validation Methods (Static - Can be used without instance)
    // --------------------------------------------------------------------------
    
    // Validate alias name according to shell naming conventions
    // Returns: true if name is valid, false otherwise
    static bool validateAliasName(const std::string& name);
    
    // Validate command string for safety and syntax
    // Returns: true if command is valid, false otherwise
    static bool validateCommand(const std::string& command);
    
    // --------------------------------------------------------------------------
    // Alias Formatting & Parsing
    // --------------------------------------------------------------------------
    
    // Format alias into shell-specific syntax string
    // Returns: Formatted alias string ready for insertion into config file
    std::string formatAlias(const Alias& alias) const;
    
    // Parse a line from config file into Alias structure
    // Returns: Parsed Alias object, empty if line is not a valid alias
    static Alias parseAliasLine(const std::string& line);
    
    // Check if a line appears to be an alias definition
    // Returns: true if line starts with 'alias' keyword
    static bool isAliasLine(const std::string& line);
    
    // --------------------------------------------------------------------------
    // String Utility Methods (Static)
    // --------------------------------------------------------------------------
    
    // Extract content from quoted string
    // Parameters: str - input string, start - position of opening quote
    // Returns: Content between quotes, or empty string if malformed
    static std::string extractQuotedString(const std::string& str, size_t start);
    
    // Escape special characters in command string
    // Handles: quotes, backslashes, dollar signs
    // Returns: Escaped string safe for shell interpretation
    static std::string escapeCommand(const std::string& command);
    
    // Remove escape sequences from string
    // Handles: backslash-escaped characters
    // Returns: Unescaped string
    static std::string unescapeString(const std::string& str);
    
private:
    // Current shell type for formatting decisions
    ShellDetector::Shell currentShell;
};

#endif // ALIASMANAGER_HPP