// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Main Window Header - GUI Interface for AliaCan
// 
// This header defines the MainWindow class, which serves as the primary
// graphical interface for the AliaCan shell alias manager. It provides
// a comprehensive GUI for managing shell aliases with features including
// alias CRUD operations, backup management, theme toggling, and real-time
// validation. The window integrates all core components of the application.
// ------------------------------------------------------------------------------

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>
#include <vector>
#include "shelldetector.hpp"
#include "aliasmanager.hpp"
#include "configfilehandler.hpp"
#include "backupmanager.hpp"

// Forward declarations for Qt widgets (reduces compilation dependencies)
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT  // Required for Qt signals/slots

public:
    // Constructor with optional parent widget
    explicit MainWindow(QWidget* parent = nullptr);
    
    // Destructor
    ~MainWindow() override;

private slots:
    // --------------------------------------------------------------------------
    // Event Handlers (connected to UI signals)
    // --------------------------------------------------------------------------
    
    // Add new alias to configuration
    void onAddAlias();
    
    // Remove selected alias from configuration
    void onRemoveAlias();
    
    // Refresh alias list from configuration file
    void onRefresh();
    
    // Handle alias selection from list
    void onAliasSelected();
    
    // Validate alias name as user types
    void onNameChanged(const QString& text);
    
    // Validate command as user types
    void onCommandChanged(const QString& text);
    
    // Show backup management dialog
    void onShowBackups();
    
    // Restore from most recent backup
    void onRestoreBackup();
    
    // Toggle between light and dark themes
    void toggleTheme();
    
    // Filter alias list based on search text
    void onSearchTextChanged(const QString& text);

private:
    // --------------------------------------------------------------------------
    // Core Application Components
    // --------------------------------------------------------------------------
    std::unique_ptr<ConfigFileHandler> configHandler;  // Handles config file I/O
    std::unique_ptr<BackupManager> backupManager;      // Manages backup operations
    ShellDetector::Shell currentShell;                 // Detected shell type
    std::string configFilePath;                        // Path to shell config file
    
    // --------------------------------------------------------------------------
    // UI Widgets
    // --------------------------------------------------------------------------
    QLabel* shellInfoLabel;       // Displays shell detection info
    QLineEdit* aliasNameInput;    // Input for alias name
    QLineEdit* commandInput;      // Input for command
    QLabel* commandStatus;        // Shows command validation status
    QPushButton* addButton;       // Add/Update alias button
    QPushButton* removeButton;    // Remove alias button
    QPushButton* refreshButton;   // Refresh list button
    QPushButton* backupButton;    // View backups button
    QPushButton* restoreButton;   // Restore backup button
    QPushButton* themeToggle;     // Theme toggle button
    QListWidget* aliasList;       // List of current aliases
    QLabel* statusLabel;          // Status message display
    QLineEdit* searchInput;       // Search/filter input
    
    // --------------------------------------------------------------------------
    // Application State
    // --------------------------------------------------------------------------
    std::vector<Alias> currentAliases;  // Current list of aliases
    bool isModifying = false;           // Flag to prevent recursive updates
    bool isDarkTheme = false;           // Current theme state
    
    // --------------------------------------------------------------------------
    // Initialization Methods
    // --------------------------------------------------------------------------
    void initializeUI();                 // Create and arrange all UI widgets
    void setupConnections();            // Connect signals to slots
    void initializeShellDetection();    // Detect shell and setup file handlers
    
    // --------------------------------------------------------------------------
    // Alias Management Methods
    // --------------------------------------------------------------------------
    void loadAliasesFromFile();         // Load aliases from config file
    void updateShellInfo();             // Update shell info display
    void updateAliasList();             // Refresh alias list widget
    void filterAliasList(const QString& searchText);  // Filter displayed aliases
    
    // --------------------------------------------------------------------------
    // UI Feedback Methods
    // --------------------------------------------------------------------------
    void showError(const QString& title, const QString& message);  // Show error dialog
    void showSuccess(const QString& message);                     // Show success notification
    bool validateInput(QString& aliasName, QString& command);     // Validate user input
    void clearInputFields();                                      // Clear input fields
    
    // --------------------------------------------------------------------------
    // Theme Management Methods
    // --------------------------------------------------------------------------
    void applyStylesheet();              // Apply current theme stylesheet
    QString getLightTheme() const;       // Get light theme CSS
    QString getDarkTheme() const;        // Get dark theme CSS
    
    // --------------------------------------------------------------------------
    // Utility Methods
    // --------------------------------------------------------------------------
    QIcon createAppIcon();               // Create application icon
};

#endif // MAINWINDOW_HPP
