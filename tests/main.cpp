// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Test Runner for AliaCan - Shell Alias Manager with Auto-Backup
//
// This file serves as the main entry point for running all unit tests
// in the AliaCan project. It provides a minimal test framework that
// sequentially executes test functions for different components of the
// application. This allows for automated testing of core functionality
// including shell detection, alias management, and configuration handling.
// ------------------------------------------------------------------------------

#include <iostream>

// Forward declarations of test functions from other test modules.
// Each function tests a specific component of the AliaCan system.
void test_shelldetector();      // Tests for shell detection functionality
void test_aliasmanager();       // Tests for alias management operations
void test_confighandler();      // Tests for configuration file handling

// Main function - Entry point for the test suite.
int main() {
    // Display test suite header for clear output organization.
    std::cout << "========================================" << std::endl;
    std::cout << "AliaCan Test Suite v0.0.1.1" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    // Execute shell detector tests.
    // This component identifies the user's current shell (bash, zsh, fish, etc.)
    // and determines the appropriate alias file location.
    std::cout << "[TEST] Running ShellDetector tests..." << std::endl;
    test_shelldetector();
    std::cout << "[TEST] ShellDetector tests completed." << std::endl << std::endl;
    
    // Execute alias manager tests.
    // This component handles CRUD operations (Create, Read, Update, Delete)
    // for shell aliases, including validation and conflict resolution.
    std::cout << "[TEST] Running AliasManager tests..." << std::endl;
    test_aliasmanager();
    std::cout << "[TEST] AliasManager tests completed." << std::endl << std::endl;
    
    // Execute configuration handler tests.
    // This component manages application settings, preferences, and
    // persistent storage of user configurations.
    std::cout << "[TEST] Running ConfigHandler tests..." << std::endl;
    test_confighandler();
    std::cout << "[TEST] ConfigHandler tests completed." << std::endl << std::endl;
    
    // Display test suite completion summary.
    std::cout << "========================================" << std::endl;
    std::cout << "All tests completed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Return 0 to indicate successful execution of all tests.
    return 0;
}