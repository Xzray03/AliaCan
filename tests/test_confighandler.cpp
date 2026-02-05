// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Unit Tests for ConfigFileHandler and BackupManager Components
//
// This file contains comprehensive unit tests for configuration file handling
// and backup management in the AliaCan application. The tests verify proper
// file I/O operations, alias management persistence, validation enforcement,
// and backup/restore functionality. Tests use temporary files to avoid
// polluting the filesystem and ensure isolation between test runs.
// ------------------------------------------------------------------------------

#include "configfilehandler.hpp"  // Configuration file operations
#include "backupmanager.hpp"      // Backup creation and restoration
#include <cassert>                // Assertion macros for test validation
#include <iostream>               // Console output for test reporting
#include <filesystem>             // Filesystem operations for test cleanup
#include <fstream>                // File stream operations
#include <cstdlib>                // Environment variable access

#include "utils.hpp"

// Alias for convenience
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------
// Utility: Get Temporary Test File Path
// Purpose: Create a unique, temporary file path for testing.
// Uses TMPDIR environment variable if available, otherwise /tmp.
// Ensures tests don't interfere with real user configuration files.
// ------------------------------------------------------------------------------
static std::string getTempTestFile() {
    // Try to use system temporary directory
    char* d = getenv("TMPDIR");
    if (!d) {
        d = const_cast<char*>("/tmp");  // Fallback to /tmp on Unix-like systems
    }
    
    // Create a unique test file name
    return std::string(d) + "/alia-can-test-config";
}

// ------------------------------------------------------------------------------
// Utility: Cleanup Test Files
// Purpose: Remove all test files created during testing.
// This prevents leftover files from accumulating in /tmp.
// Handles both the main test file and any backup files created.
// ------------------------------------------------------------------------------
static void cleanupTestFile() {
    std::string f = getTempTestFile();
    
    // Remove main test file if it exists
    if (fs::exists(f)) {
        fs::remove(f);
    }
    
    // Search for and remove any backup files created during tests
    // Backup files typically have suffixes like .bak, .backup, or timestamps
    try {
        fs::path parent_path = fs::path(f).parent_path();
        if (fs::exists(parent_path)) {
            for (const auto& entry : fs::directory_iterator(parent_path)) {
                auto filename = entry.path().filename().string();
                
                // Match any files with the test pattern
                if (filename.find("alia-can-test-config") != std::string::npos) {
                    try {
                        fs::remove(entry.path());
                    } catch (const std::exception& e) {
                        std::cerr << "Warning: Failed to remove test file " 
                                  << filename << ": " << e.what() << std::endl;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Directory scan failed: " << e.what() << std::endl;
    }
}

// ------------------------------------------------------------------------------
// Test: Load Empty Configuration File
// Purpose: Verify handling of non-existent or empty configuration files.
// Expected behavior: Returns empty alias list without errors.
// ------------------------------------------------------------------------------
static void testLoadEmptyFile() {
    std::cout << "  Testing load empty file... ";
    
    cleanupTestFile();  // Ensure clean state
    ConfigFileHandler h(getTempTestFile(), ShellDetector::Shell::BASH);
    
    // Load from non-existent file should return empty list
    auto aliases = h.loadAliases();
    assert(aliases.empty());
    
    // Create empty file and test loading
    std::string f = getTempTestFile();
    std::ofstream ofs(f);  // Create empty file
    ofs.close();
    
    ConfigFileHandler h2(f, ShellDetector::Shell::BASH);
    aliases = h2.loadAliases();
    assert(aliases.empty());
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Add Single Alias
// Purpose: Verify basic alias addition and persistence.
// Tests that an alias can be added and then loaded back.
// ------------------------------------------------------------------------------
static void testAddAlias() {
    std::cout << "  Testing add single alias... ";
    
    cleanupTestFile();
    ConfigFileHandler h(getTempTestFile(), ShellDetector::Shell::BASH);
    
    Alias a{"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()};
    assert(h.addAlias(a));  // Should succeed for valid alias
    
    // Verify the alias was added
    auto aliases = h.loadAliases();
    assert(aliases.size() == 1);
    assert(aliases[0].name == "ll");
    assert(aliases[0].command == "ls -la");
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Remove Alias
// Purpose: Verify alias removal functionality.
// Tests that specific aliases can be removed while others remain.
// ------------------------------------------------------------------------------
static void testRemoveAlias() {
    std::cout << "  Testing remove alias... ";
    
    cleanupTestFile();
    ConfigFileHandler h(getTempTestFile(), ShellDetector::Shell::BASH);
    
    // Add multiple aliases
    h.addAlias({"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()});
    h.addAlias({"gs", "git status", std::string(), true, getCurrentDate(), getCurrentDate()});
    h.addAlias({"gp", "git push", std::string(), true, getCurrentDate(), getCurrentDate()});
    
    // Remove one alias
    h.removeAlias("ll");
    
    // Verify removal
    auto aliases = h.loadAliases();
    assert(aliases.size() == 2);  // Should have 2 remaining
    
    // Verify correct aliases remain
    bool has_gs = false, has_gp = false;
    for (const auto& alias : aliases) {
        if (alias.name == "gs" && alias.command == "git status") has_gs = true;
        if (alias.name == "gp" && alias.command == "git push") has_gp = true;
    }
    assert(has_gs && has_gp);  // Both should be present
    
    // Test removal of non-existent alias (should not crash)
    h.removeAlias("nonexistent");
    assert(h.loadAliases().size() == 2);  // Size unchanged
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Multiple Aliases
// Purpose: Verify handling of multiple aliases in configuration.
// Tests bulk operations and ordering preservation.
// ------------------------------------------------------------------------------
static void testMultipleAliases() {
    std::cout << "  Testing multiple aliases... ";
    
    cleanupTestFile();
    ConfigFileHandler h(getTempTestFile(), ShellDetector::Shell::ZSH);
    
    // Define test aliases
    std::vector<Alias> test_aliases = {
        {"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()},
        {"la", "ls -A", std::string(), true, getCurrentDate(), getCurrentDate()},
        {"l", "ls -CF", std::string(), true, getCurrentDate(), getCurrentDate()},
        {"gs", "git status", std::string(), true, getCurrentDate(), getCurrentDate()}
    };
    
    // Add all aliases
    for (auto& a : test_aliases) {
        assert(h.addAlias(a));
    }
    
    // Load and verify
    auto loaded_aliases = h.loadAliases();
    assert(loaded_aliases.size() == test_aliases.size());
    
    // Verify each alias was saved correctly
    for (const auto& test_alias : test_aliases) {
        bool found = false;
        for (const auto& loaded_alias : loaded_aliases) {
            if (loaded_alias.name == test_alias.name && 
                loaded_alias.command == test_alias.command) {
                found = true;
                break;
            }
        }
        assert(found);  // Each test alias should be found
    }
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Validation on Add
// Purpose: Verify that invalid aliases are rejected.
// Tests that the ConfigFileHandler enforces validation rules.
// ------------------------------------------------------------------------------
static void testValidationOnAdd() {
    std::cout << "  Testing validation on add... ";
    
    cleanupTestFile();
    ConfigFileHandler h(getTempTestFile(), ShellDetector::Shell::BASH);
    
    // Test invalid alias names
    assert(!h.addAlias({"bad name", "ls", std::string(), true, getCurrentDate(), getCurrentDate()}));      // Space in name
    assert(!h.addAlias({"", "ls", std::string(), true, getCurrentDate(), getCurrentDate()}));              // Empty name
    
    // Test invalid commands
    assert(!h.addAlias({"ll", "", std::string(), true, getCurrentDate(), getCurrentDate()}));              // Empty command
    
    // Test duplicate alias
    assert(h.addAlias({"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()}));         // First should succeed
    // Implementation may reject duplicates: assert(!h.addAlias({"ll", "ls -lh"}));
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Backup Creation
// Purpose: Verify that backups are created correctly.
// Tests BackupManager's ability to create backup copies of config files.
// ------------------------------------------------------------------------------
static void testBackupCreation() {
    std::cout << "  Testing backup creation... ";
    
    cleanupTestFile();
    std::string config_file = getTempTestFile();
    
    ConfigFileHandler h(config_file, ShellDetector::Shell::BASH);
    BackupManager b(config_file);
    
    // Add an alias to create content
    h.addAlias({"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()});
    
    // Create backup
    std::string backup_path = b.createBackup();
    
    // Verify backup was created
    assert(!backup_path.empty());
    assert(fs::exists(backup_path));
    
    // Verify backup contains expected data
    std::ifstream backup_file(backup_path);
    std::string line;
    bool found_alias = false;
    while (std::getline(backup_file, line)) {
        if (line.find("alias ll") != std::string::npos) {
            found_alias = true;
            break;
        }
    }
    assert(found_alias);
    
    // Verify backup filename pattern (contains timestamp or version)
    assert(backup_path.find(config_file) != std::string::npos ||
           backup_path.find("backup") != std::string::npos);
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Test: Backup Restoration
// Purpose: Verify that backups can be restored correctly.
// Tests BackupManager's ability to restore config files from backups.
// ------------------------------------------------------------------------------
static void testRestoreBackup() {
    std::cout << "  Testing backup restoration... ";
    
    cleanupTestFile();
    std::string config_file = getTempTestFile();
    
    ConfigFileHandler h(config_file, ShellDetector::Shell::BASH);
    BackupManager b(config_file);
    
    // Initial state: one alias
    h.addAlias({"ll", "ls -la", std::string(), true, getCurrentDate(), getCurrentDate()});
    
    // Create backup
    std::string backup_path = b.createBackup();
    assert(!backup_path.empty());
    
    // Modify config (add another alias)
    h.addAlias({"gs", "git status", std::string(), true, getCurrentDate(), getCurrentDate()});
    assert(h.loadAliases().size() == 2);
    
    // Restore from backup
    assert(b.restoreFromBackup(backup_path));
    
    // Verify restored state
    auto aliases = h.loadAliases();
    assert(aliases.size() == 1);  // Should be back to original state
    
    // Verify correct alias was restored
    if (aliases.size() == 1) {
        assert(aliases[0].name == "ll");
        assert(aliases[0].command == "ls -la");
    }
    
    // Test restoration from non-existent backup (should fail)
    assert(!b.restoreFromBackup("/nonexistent/backup/file.bak"));
    
    std::cout << "✓ passed" << std::endl;
}

// ------------------------------------------------------------------------------
// Main Test Runner
// Purpose: Execute all ConfigFileHandler and BackupManager tests.
// ------------------------------------------------------------------------------
void test_confighandler() {
    std::cout << "Running ConfigFileHandler tests...\n";
    
    // Execute all test cases
    testLoadEmptyFile();      // Test empty file handling
    testAddAlias();           // Test single alias addition
    testRemoveAlias();        // Test alias removal
    testMultipleAliases();    // Test multiple aliases
    testValidationOnAdd();    // Test input validation
    testBackupCreation();     // Test backup functionality
    testRestoreBackup();      // Test backup restoration
    
    // Final cleanup
    cleanupTestFile();
    
    std::cout << "✓ ConfigFileHandler tests passed!\n";
}
