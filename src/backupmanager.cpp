// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Backup Manager Component Implementation
// 
// This file implements the BackupManager class, providing comprehensive
// backup management with compression, rotation, and restoration features.
// It uses XZ compression for space efficiency and maintains backup
// organization in ~/.shellbackup/ directory.
// ------------------------------------------------------------------------------

#include "backupmanager.hpp"
#include <cstdlib>    // For std::system
#include <filesystem> // For filesystem operations
#include <fstream>    // For file I/O
#include <chrono>     // For timestamps
#include <algorithm>  // For sorting
#include <vector>     // For backup lists
#include <sstream>    // For string formatting
#include <iomanip>    // For timestamp formatting

// Alias for convenience
namespace fs = std::filesystem;

// ------------------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------------------
BackupManager::BackupManager(const std::string& originalFilePath) 
    : originalFilePath(originalFilePath) {
    // Store the path to the file we'll be backing up
}

// ------------------------------------------------------------------------------
// Create Backup
// Steps:
// 1. Validate original file exists
// 2. Generate backup path with timestamp
// 3. Copy file to backup location
// 4. Trigger cleanup/compression of old backups
// ------------------------------------------------------------------------------
std::string BackupManager::createBackup() {
    // Check if original file exists
    if (!fs::exists(originalFilePath)) {
        lastError = "Original file does not exist: " + originalFilePath;
        return "";
    }
    
    // Generate backup path: ~/.shellbackup/filename.bakYYYYMMDD_HHMMSS
    std::string backupDir = getBackupDirectory();
    std::string backupFilename = fs::path(originalFilePath).filename().string() + 
                                 ".bak" + generateTimestamp();
    std::string backupPath = fs::path(backupDir) / backupFilename;
    
    try {
        // Create backup by copying the file
        fs::copy_file(originalFilePath, backupPath, 
                     fs::copy_options::overwrite_existing);
        
        // Clean up old backups to prevent unlimited growth
        cleanupAndCompressOldBackups(20);
        
        return backupPath;
        
    } catch (const std::exception& e) {
        lastError = std::string("Failed to create backup: ") + e.what();
        return "";
    }
}

// ------------------------------------------------------------------------------
// Cleanup and Compress Old Backups
// Strategy:
// - Keep 10 most recent backups uncompressed
// - Compress backups 11-20 with XZ
// - Delete backups beyond 20
// ------------------------------------------------------------------------------
int BackupManager::cleanupAndCompressOldBackups(int maxBackups) {
    // Ensure reasonable maximum
    if (maxBackups <= 0) maxBackups = 20;
    
    // Get all backups for this file
    std::vector<std::string> backups = listBackups();
    
    // Pair backups with their modification times for sorting
    std::vector<std::pair<std::string, fs::file_time_type>> backupsWithTime;
    for (const auto& backup : backups) {
        try {
            backupsWithTime.emplace_back(backup, fs::last_write_time(backup));
        } catch (...) {
            continue; // Skip if we can't get file time
        }
    }
    
    // Sort by modification time (newest first)
    std::sort(backupsWithTime.begin(), backupsWithTime.end(),
              [](const auto& a, const auto& b) { 
                  return a.second > b.second; 
              });
    
    int deleted = 0;
    
    // Process backups from oldest to newest
    for (size_t i = 0; i < backupsWithTime.size(); ++i) {
        const std::string& path = backupsWithTime[i].first;
        
        if (i >= static_cast<size_t>(maxBackups)) {
            // Delete backups beyond maximum limit
            try {
                fs::remove(path);
                deleted++;
            } catch (...) {
                continue; // Skip if deletion fails
            }
        } 
        else if (i >= 10 && path.substr(path.size() - 3) != ".xz") {
            // Compress backups beyond the 10 most recent (if not already compressed)
            // Using XZ compression with maximum compression level (-9e)
            std::string cmd = "xz -9e " + path;
            if (std::system(cmd.c_str()) != 0) {
                lastError = "Failed to compress backup: " + path;
            }
        }
    }
    
    return deleted;
}

// ------------------------------------------------------------------------------
// Restore From Specific Backup
// Supports both regular and .xz compressed backups
// ------------------------------------------------------------------------------
bool BackupManager::restoreFromBackup(const std::string& backupPath) {
    std::string actualBackupPath = backupPath;
    
    // Handle compressed backups
    if (backupPath.substr(backupPath.size() - 3) == ".xz") {
        // Remove .xz extension for decompressed file
        actualBackupPath = backupPath.substr(0, backupPath.size() - 3);
        
        // Decompress using xz command
        // -d: decompress, -k: keep original, -f: force overwrite
        std::string cmd = "xz -d -k -f " + backupPath;
        if (std::system(cmd.c_str()) != 0) {
            lastError = "Failed to decompress backup: " + backupPath;
            return false;
        }
    }
    
    // Verify decompressed file exists
    if (!fs::exists(actualBackupPath)) {
        lastError = "Backup file does not exist: " + actualBackupPath;
        return false;
    }
    
    try {
        // Restore by copying backup over original
        fs::copy_file(actualBackupPath, originalFilePath, 
                     fs::copy_options::overwrite_existing);
        return true;
        
    } catch (const std::exception& e) {
        lastError = std::string("Failed to restore from backup: ") + e.what();
        return false;
    }
}

// ------------------------------------------------------------------------------
// List All Backups
// Scans backup directory for files matching the backup pattern
// ------------------------------------------------------------------------------
std::vector<std::string> BackupManager::listBackups() const {
    std::vector<std::string> backups;
    std::string backupPattern = getBackupBaseName();
    std::string backupDir = getBackupDirectory();
    
    try {
        // Scan backup directory
        for (const auto& entry : fs::directory_iterator(backupDir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                
                // Match files that start with the backup base name
                // (e.g., ".bashrc.bak" for .bashrc file)
                if (filename.find(backupPattern) != std::string::npos) {
                    backups.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        // Directory might not exist or be inaccessible
        lastError = std::string("Failed to list backups: ") + e.what();
    }
    
    return backups;
}

// ------------------------------------------------------------------------------
// Get Backup Directory
// Default: ~/.shellbackup/
// Fallback: Same directory as original file
// ------------------------------------------------------------------------------
std::string BackupManager::getBackupDirectory() const {
    // Try to use HOME directory
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) {
        // Fallback to original file's directory
        return fs::path(originalFilePath).parent_path().string();
    }
    
    // Create ~/.shellbackup/ directory
    fs::path backupDir = fs::path(homeDir) / ".shellbackup";
    
    try {
        if (!fs::exists(backupDir)) {
            // Create directory with all parent directories
            fs::create_directories(backupDir);
            
            // Set appropriate permissions (owner read/write/execute)
            fs::permissions(backupDir, 
                           fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec,
                           fs::perm_options::replace);
        }
        return backupDir.string();
        
    } catch (const std::exception& e) {
        // If creation fails, fallback to original directory
        lastError = std::string("Failed to create backup directory: ") + e.what();
        return fs::path(originalFilePath).parent_path().string();
    }
}

// ------------------------------------------------------------------------------
// Get Most Recent Backup
// Returns the path to the newest backup file
// ------------------------------------------------------------------------------
std::string BackupManager::getLastBackupPath() const {
    std::vector<std::string> backups = listBackups();
    if (backups.empty()) {
        return ""; // No backups available
    }
    
    // Get modification times for all backups
    std::vector<std::pair<std::string, fs::file_time_type>> backupsWithTime;
    for (const auto& backup : backups) {
        try {
            backupsWithTime.emplace_back(backup, fs::last_write_time(backup));
        } catch (...) {
            continue; // Skip if we can't get file time
        }
    }
    
    if (backupsWithTime.empty()) {
        return "";
    }
    
    // Sort by modification time (newest first)
    std::sort(backupsWithTime.begin(), backupsWithTime.end(),
              [](const auto& a, const auto& b) { 
                  return a.second > b.second; 
              });
    
    // Return the newest backup
    return backupsWithTime[0].first;
}

// ------------------------------------------------------------------------------
// Restore From Most Recent Backup
// Convenience wrapper around restoreFromBackup
// ------------------------------------------------------------------------------
bool BackupManager::restoreFromLastBackup() {
    std::string lastBackup = getLastBackupPath();
    if (lastBackup.empty()) {
        lastError = "No backup found";
        return false;
    }
    return restoreFromBackup(lastBackup);
}

// ------------------------------------------------------------------------------
// Get Original File Path
// ------------------------------------------------------------------------------
std::string BackupManager::getOriginalFilePath() const {
    return originalFilePath;
}

// ------------------------------------------------------------------------------
// Generate Timestamp
// Format: YYYYMMDD_HHMMSS
// Example: 20240115_143025
// ------------------------------------------------------------------------------
std::string BackupManager::generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    // Format: YearMonthDay_HourMinuteSecond
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
    
    return ss.str();
}

// ------------------------------------------------------------------------------
// Get Backup Base Name
// Example: For "/home/user/.bashrc" returns ".bashrc.bak"
// ------------------------------------------------------------------------------
std::string BackupManager::getBackupBaseName() const {
    return fs::path(originalFilePath).filename().string() + ".bak";
}

// ------------------------------------------------------------------------------
// Get Last Error Message
// Useful for debugging failed operations
// ------------------------------------------------------------------------------
std::string BackupManager::getLastError() const {
    return lastError;
}

// ------------------------------------------------------------------------------
// Compare File Ages
// Returns true if file1 is newer than file2
// ------------------------------------------------------------------------------
bool BackupManager::isNewer(const std::string& file1, const std::string& file2) {
    try {
        return fs::last_write_time(file1) > fs::last_write_time(file2);
    } catch (...) {
        return false; // Assume not newer if we can't compare
    }
}