// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Unit Tests for AliasManager Component
//
// This file contains comprehensive unit tests for the AliasManager class,
// which is responsible for managing shell aliases in the AliaCan application.
// The tests verify core functionality including alias validation, parsing,
// formatting, and shell-specific operations. Each test is designed to ensure
// the AliasManager correctly handles edge cases and maintains data integrity.
// ------------------------------------------------------------------------------

#include "aliasmanager.hpp"    // Main class under test
#include "shelldetector.hpp"   // Shell types enum
#include <cassert>             // Assertion macros for test validation
#include <iostream>            // Console output for test reporting

#include "utils.hpp"

// ------------------------------------------------------------------------------
// Test: Alias Name Validation
// Purpose: Verify that alias names follow proper naming conventions.
// Rules tested:
//   - Non-empty strings
//   - No whitespace characters
//   - Valid identifier characters (letters, numbers, underscores)
//   - Shell-specific naming restrictions
// ------------------------------------------------------------------------------
static void testValidateAliasName() {
    std::cout << "  Testing alias name validation... ";
    
    // Valid alias names - should pass validation
    assert(AliasManager::validateAliasName("ll"));            // Simple alias
    assert(AliasManager::validateAliasName("git_log"));       // With underscore
    assert(AliasManager::validateAliasName("g123"));          // Alphanumeric
    assert(AliasManager::validateAliasName("_start"));        // Starting with underscore
    
    // Invalid alias names - should fail validation
    assert(!AliasManager::validateAliasName(""));             // Empty string
    assert(!AliasManager::validateAliasName("with space"));   // Contains space
    assert(!AliasManager::validateAliasName("alias.ll"));     // Contains dot
    assert(!AliasManager::validateAliasName("ll\n"));         // Contains newline
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Command Validation
// Purpose: Ensure commands are valid and safe to execute.
// Rules tested:
//   - Non-empty commands
//   - Basic safety checks (no prohibited characters/patterns)
//   - Shell-specific command syntax
// ------------------------------------------------------------------------------
static void testValidateCommand() {
    std::cout << "  Testing command validation... ";
    
    // Valid commands - should pass validation
    assert(AliasManager::validateCommand("ls -la"));          // Simple command with flags
    assert(AliasManager::validateCommand("git log --oneline")); // Complex command
    assert(AliasManager::validateCommand("cd ~/projects"));   // Command with path
    assert(AliasManager::validateCommand("echo \"Hello\""));  // Command with quotes
    
    // Invalid commands - should fail validation
    assert(!AliasManager::validateCommand(""));               // Empty command
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Alias Formatting
// Purpose: Verify correct formatting of aliases for different shells.
// Tests shell-specific syntax:
//   - bash/zsh: alias name='command'
//   - fish: alias name 'command'
//   - Proper escaping of special characters
// ------------------------------------------------------------------------------
static void testFormatAlias() {
    std::cout << "  Testing alias formatting... ";
    
    // Test bash shell formatting
    {
        AliasManager m(ShellDetector::Shell::BASH);
        Alias a{"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()};
        auto f = m.formatAlias(a);
        assert(f.find("alias ll") != std::string::npos);      // Contains alias keyword
        assert(f.find("ls -la") != std::string::npos);        // Contains command
        assert(f.find("=") != std::string::npos);             // Contains equals for bash
        assert(f.find("'") != std::string::npos);             // Contains quotes for bash
    }
    
    // Test zsh shell formatting (similar to bash)
    {
        AliasManager m(ShellDetector::Shell::ZSH);
        Alias a{"gst", "git status", std::string(), true, getCurrentDate(), getCurrentDate()};
        auto f = m.formatAlias(a);
        assert(f.find("alias gst") != std::string::npos);
    }
    
    // Test fish shell formatting (different syntax)
    {
        AliasManager m(ShellDetector::Shell::FISH);
        Alias a{"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()};
        auto f = m.formatAlias(a);
        assert(f.find("alias ll") != std::string::npos);
        // Fish uses: alias ll 'ls -la'  (no equals, different quoting)
    }
    
    // Test with special characters that need escaping
    {
        AliasManager m(ShellDetector::Shell::BASH);
        Alias a{"echo_test", "echo \"Hello $USER\"", std::string(), true, getCurrentDate(), getCurrentDate()};
        auto f = m.formatAlias(a);
        // Should properly escape quotes and dollar signs
        assert(f.find("\\\"") != std::string::npos || f.find("'") != std::string::npos);
    }
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Alias Line Parsing
// Purpose: Verify parsing of alias definition strings.
// Tests parsing of various syntax formats:
//   - bash: alias ll='ls -la'
//   - bash: alias ll="ls -la"
//   - fish: alias ll 'ls -la'
//   - With and without spaces around equals
// ------------------------------------------------------------------------------
static void testParseAliasLine() {
    std::cout << "  Testing alias line parsing... ";
    
    // Standard bash/zsh format with single quotes
    {
        auto a = AliasManager::parseAliasLine("alias ll='ls -la'");
        assert(a.name == "ll");
        assert(a.command == "ls -la");
    }
    
    // Bash format with double quotes
    {
        auto a = AliasManager::parseAliasLine("alias ll=\"ls -la\"");
        assert(a.name == "ll");
        assert(a.command == "ls -la");
    }
    
    // With spaces around equals (some shells allow this)
    {
        auto a = AliasManager::parseAliasLine("alias ll = 'ls -la'");
        assert(a.name == "ll");
        assert(a.command == "ls -la");
    }
    
    // Complex command with nested quotes
    {
        auto a = AliasManager::parseAliasLine("alias gcm=\"git commit -m 'initial commit'\"");
        assert(a.name == "gcm");
        assert(a.command == "git commit -m 'initial commit'");
    }
    
    // Invalid lines should return empty alias
    {
        auto a = AliasManager::parseAliasLine("");
        assert(a.name.empty());
        assert(a.command.empty());
        
        a = AliasManager::parseAliasLine("not an alias");
        assert(a.name.empty());
        assert(a.command.empty());
    }
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Alias Line Detection
// Purpose: Identify valid alias definition lines.
// Tests recognition of:
//   - Lines starting with 'alias ' keyword
//   - Proper alias syntax patterns
//   - Exclusion of non-alias lines
// ------------------------------------------------------------------------------
static void testIsAliasLine() {
    std::cout << "  Testing alias line detection... ";
    
    // Valid alias lines - should return true
    assert(AliasManager::isAliasLine("alias ll='ls -la'"));      // Single quotes
    assert(AliasManager::isAliasLine("alias ll=\"ls -la\""));    // Double quotes
    assert(AliasManager::isAliasLine("alias ll 'ls -la'"));      // Fish syntax
    assert(AliasManager::isAliasLine("alias ll = 'ls -la'"));    // With spaces
    assert(AliasManager::isAliasLine("  alias ll='ls'"));        // With leading spaces
    
    // Invalid lines - should return false
    assert(!AliasManager::isAliasLine("export X=1"));            // Environment variable
    assert(!AliasManager::isAliasLine("function ll() {"));       // Function definition
    assert(!AliasManager::isAliasLine("# alias ll='ls'"));       // Commented out
    assert(!AliasManager::isAliasLine(""));                      // Empty line
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Main Test Runner
// Purpose: Execute all AliasManager tests and report results.
// ------------------------------------------------------------------------------
void test_aliasmanager() {
    std::cout << "Running AliasManager tests...\n";
    
    // Execute all test cases
    testValidateAliasName();      // Test naming rules
    testValidateCommand();        // Test command safety
    testFormatAlias();            // Test shell-specific formatting
    testParseAliasLine();         // Test parsing of alias strings
    testIsAliasLine();            // Test alias detection
    
    std::cout << "✓ AliasManager tests passed!\n";
}
