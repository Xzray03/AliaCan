// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Backup Manager Component Header
// 
// This header defines the BackupManager class, which provides robust
// backup and restoration functionality for shell configuration files.
// It handles automatic backup creation, compression, rotation, and
// restoration with comprehensive error handling and management.
// ------------------------------------------------------------------------------

#ifndef BACKUPMANAGER_HPP
#define BACKUPMANAGER_HPP

#include <string>
#include <filesystem>
#include <ctime>
#include <vector>

class BackupManager {
public:
    // --------------------------------------------------------------------------
    // Constructor
    // --------------------------------------------------------------------------
    
    // Initialize with path to the file that needs backup protection
    explicit BackupManager(const std::string& originalFilePath);
    
    // --------------------------------------------------------------------------
    // Backup Operations
    // --------------------------------------------------------------------------
    
    // Create a timestamped backup of the original file
    // Returns: Path to created backup, empty string on failure
    std::string createBackup();
    
    // Get path to the most recent backup
    // Returns: Path to latest backup, empty if no backups exist
    std::string getLastBackupPath() const;
    
    // List all available backups for the original file
    // Returns: Vector of backup file paths, sorted by creation time
    std::vector<std::string> listBackups() const;
    
    // --------------------------------------------------------------------------
    // Restoration Operations
    // --------------------------------------------------------------------------
    
    // Restore original file from the most recent backup
    // Returns: true if restoration successful, false otherwise
    bool restoreFromLastBackup();
    
    // Restore original file from a specific backup file
    // Parameters: backupPath - Path to backup file (supports .xz compressed)
    // Returns: true if restoration successful, false otherwise
    bool restoreFromBackup(const std::string& backupPath);
    
    // --------------------------------------------------------------------------
    // Information Getters
    // --------------------------------------------------------------------------
    
    // Get the original file path being backed up
    std::string getOriginalFilePath() const;
    
    // Get the directory where backups are stored
    // Default: ~/.shellbackup/
    std::string getBackupDirectory() const;
    
    // Get the last error message (for debugging)
    std::string getLastError() const;
    
    // --------------------------------------------------------------------------
    // Backup Management
    // --------------------------------------------------------------------------
    
    // Remove old backups, keeping only the specified number of most recent
    // Parameters: keepCount - Number of recent backups to preserve
    // Returns: Number of backups deleted
    int cleanupOldBackups(int keepCount = 10);
    
    // Advanced cleanup: keep recent backups, compress older ones, delete oldest
    // Parameters: maxBackups - Maximum total backups (recent + compressed)
    // Returns: Number of backups deleted
    int cleanupAndCompressOldBackups(int maxBackups);
    
private:
    // --------------------------------------------------------------------------
    // Private Methods
    // --------------------------------------------------------------------------
    
    // Generate timestamp string for backup filenames
    // Format: YYYYMMDD_HHMMSS
    static std::string generateTimestamp();
    
    // Get base backup filename without timestamp
    // Format: original_filename.bak
    std::string getBackupBaseName() const;
    
    // Compare file modification times
    // Returns: true if file1 is newer than file2
    static bool isNewer(const std::string& file1, const std::string& file2);
    
    // --------------------------------------------------------------------------
    // Member Variables
    // --------------------------------------------------------------------------
    
    std::string originalFilePath;  // Path to file being backed up
    mutable std::string lastError; // Last error message (thread-safe mutable)
};

#endif // BACKUPMANAGER_HPP