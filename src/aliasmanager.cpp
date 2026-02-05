// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Alias Manager Component Implementation
// 
// This file implements the AliasManager class, providing concrete functionality
// for alias management. The implementation handles shell-specific formatting,
// robust parsing of alias definitions, and comprehensive validation. Special
// attention is given to edge cases and security considerations.
// ------------------------------------------------------------------------------

#include "aliasmanager.hpp"
#include <algorithm>  // For std::all_of, std::isalnum
#include <cctype>     // For character classification
#include <sstream>    // For string stream operations

// ------------------------------------------------------------------------------
// Constructor Implementation
// ------------------------------------------------------------------------------
AliasManager::AliasManager(ShellDetector::Shell shell) : currentShell(shell) {
    // Initialize with specified shell type
}

// ------------------------------------------------------------------------------
// Validation: Alias Name
// Rules:
// 1. Non-empty, max 255 characters
// 2. First character: letter or underscore
// 3. Subsequent characters: letter, digit, underscore, or hyphen
// 4. No spaces, special punctuation, or control characters
// ------------------------------------------------------------------------------
bool AliasManager::validateAliasName(const std::string& name) {
    // Check length constraints
    if (name.empty() || name.length() > 255) return false;
    
    // First character must be alphabetic or underscore
    // Using C locale for consistent behavior across platforms
    if (!std::isalnum(static_cast<unsigned char>(name[0])) && name[0] != '_') {
        return false;
    }
    
    // Check all remaining characters
    return std::all_of(name.begin() + 1, name.end(), [](char c) {
        // Allow alphanumeric, underscore, and hyphen
        return std::isalnum(static_cast<unsigned char>(c)) || 
               c == '_' || 
               c == '-';
    });
}

// ------------------------------------------------------------------------------
// Validation: Command
// Rules:
// 1. Non-empty
// 2. Reasonable length limit (2048 characters)
// 3. Could add security checks for dangerous commands
// ------------------------------------------------------------------------------
bool AliasManager::validateCommand(const std::string& command) {
    // Basic validation
    if (command.empty()) return false;
    
    // Reasonable length limit to prevent abuse
    if (command.length() > 2048) return false;
    
    return true;
}

// ------------------------------------------------------------------------------
// Format Alias for Shell Configuration
// Handles:
// - Shell-specific syntax variations
// - Proper quoting based on command content
// - Special character escaping
// ------------------------------------------------------------------------------
std::string AliasManager::formatAlias(const Alias& alias) const {
    // Basic validation
    if (!validateAliasName(alias.name) || !validateCommand(alias.command)) {
        return "";
    }
    
    // Handle shell-specific formatting
    switch (currentShell) {
        case ShellDetector::Shell::BASH:
        case ShellDetector::Shell::ZSH:
            // BASH/ZSH: alias name='command' or alias name="command"
            if (alias.command.find('\'') != std::string::npos) {
                // Command contains single quotes, use double quotes
                return "alias " + alias.name + "=\"" + 
                       escapeCommand(alias.command) + "\"";
            } else {
                // Use single quotes (safer, prevents variable expansion)
                return "alias " + alias.name + "='" + 
                       escapeCommand(alias.command) + "'";
            }
            
        case ShellDetector::Shell::FISH:
            // FISH: alias name 'command'
            return "alias " + alias.name + " '" + 
                   escapeCommand(alias.command) + "'";
            
        case ShellDetector::Shell::UNKNOWN:
        default:
            // Default to BASH syntax
            return "alias " + alias.name + "='" + 
                   escapeCommand(alias.command) + "'";
    }
}

// ------------------------------------------------------------------------------
// Parse Alias Line from Configuration File
// Supports formats:
// - alias name='command'
// - alias name="command"
// - alias name = 'command' (with spaces)
// - alias name 'command' (fish syntax)
// ------------------------------------------------------------------------------
Alias AliasManager::parseAliasLine(const std::string& line) {
    Alias result;  // Default empty result
    
    // Skip leading whitespace
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) return result;  // Empty line
    
    // Check if line starts with "alias"
    if (line.substr(start, 5) != "alias") return result;  // Not an alias line
    
    // Find equals sign (might be spaces around it)
    size_t eqPos = line.find('=', start + 5);
    if (eqPos == std::string::npos) {
        // Fish syntax: alias name 'command' (no equals)
        // Could implement fish-specific parsing here
        return result;
    }
    
    // Extract alias name (between "alias" and "=")
    std::string namePart = line.substr(start + 5, eqPos - start - 5);
    size_t nameStart = namePart.find_first_not_of(" \t");
    size_t nameEnd = namePart.find_last_not_of(" \t");
    
    if (nameStart == std::string::npos) return result;  // No name found
    
    result.name = namePart.substr(nameStart, nameEnd - nameStart + 1);
    
    // Extract command (after "=")
    std::string commandPart = line.substr(eqPos + 1);
    size_t cmdStart = commandPart.find_first_not_of(" \t");
    
    if (cmdStart == std::string::npos) return result;  // No command found
    
    // Check for quoted command
    if (commandPart[cmdStart] == '\'' || commandPart[cmdStart] == '"') {
        char quote = commandPart[cmdStart];
        size_t endQuote = commandPart.find(quote, cmdStart + 1);
        
        if (endQuote != std::string::npos) {
            // Found matching quote
            result.command = commandPart.substr(cmdStart + 1, 
                                               endQuote - cmdStart - 1);
        } else {
            // Unclosed quote - take everything after opening quote
            result.command = commandPart.substr(cmdStart + 1);
        }
    } else {
        // Unquoted command - read until comment or end of line
        size_t commentPos = commandPart.find('#', cmdStart);
        result.command = commandPart.substr(cmdStart, commentPos - cmdStart);
        
        // Trim trailing whitespace
        size_t end = result.command.find_last_not_of(" \t");
        if (end != std::string::npos) {
            result.command.resize(end + 1);
        }
    }
    
    // Unescape the command if needed
    result.command = unescapeString(result.command);
    
    return result;
}

// ------------------------------------------------------------------------------
// Detect Alias Line
// Simple check: line starts with "alias" keyword
// Could be enhanced to handle indented or commented alias lines
// ------------------------------------------------------------------------------
bool AliasManager::isAliasLine(const std::string& line) {
    // Skip leading whitespace
    size_t start = line.find_first_not_of(" \t");
    
    // Check if line starts with "alias"
    return start != std::string::npos && 
           line.substr(start, 5) == "alias";
}

// ------------------------------------------------------------------------------
// Shell Getter/Setter
// ------------------------------------------------------------------------------
ShellDetector::Shell AliasManager::getShell() const { 
    return currentShell; 
}

void AliasManager::setShell(ShellDetector::Shell shell) { 
    currentShell = shell; 
}

// ------------------------------------------------------------------------------
// Utility: Extract Quoted String
// Extracts content between matching quotes
// ------------------------------------------------------------------------------
std::string AliasManager::extractQuotedString(const std::string& str, size_t start) {
    // Validate input
    if (start >= str.length()) return "";
    
    char quote = str[start];
    if (quote != '\'' && quote != '"') return "";
    
    // Find closing quote
    size_t end = str.find(quote, start + 1);
    
    // Return content between quotes
    return end == std::string::npos ? 
           str.substr(start + 1) :  // Unclosed quote
           str.substr(start + 1, end - start - 1);  // Properly quoted
}

// ------------------------------------------------------------------------------
// Utility: Escape Command String
// Escapes special shell characters to prevent interpretation
// ------------------------------------------------------------------------------
std::string AliasManager::escapeCommand(const std::string& command) {
    std::string escaped;
    escaped.reserve(command.length() * 2);  // Reserve worst-case space
    
    for (char c : command) {
        // Escape characters that have special meaning in shells
        if (c == '\'' || c == '"' || c == '\\' || c == '$' || 
            c == '`' || c == '!' || c == '*' || c == '?') {
            escaped += '\\';
        }
        escaped += c;
    }
    
    return escaped;
}

// ------------------------------------------------------------------------------
// Utility: Unescape String
// Removes backslash escapes from string
// ------------------------------------------------------------------------------
std::string AliasManager::unescapeString(const std::string& str) {
    std::string unescaped;
    unescaped.reserve(str.length());  // Unescaped string won't be longer
    
    bool prevBackslash = false;
    
    for (char c : str) {
        if (prevBackslash) {
            // Current character was escaped, add it literally
            unescaped += c;
            prevBackslash = false;
        } else if (c == '\\') {
            // Backslash found, escape next character
            prevBackslash = true;
        } else {
            // Normal character
            unescaped += c;
        }
    }
    
    // Handle trailing backslash (malformed but possible)
    if (prevBackslash) {
        unescaped += '\\';
    }
    
    return unescaped;
}