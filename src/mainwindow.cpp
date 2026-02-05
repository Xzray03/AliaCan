// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Main Window Implementation - GUI for Alias Management
// 
// This file implements the MainWindow class, providing a modern, responsive
// interface for managing shell aliases. It features real-time validation,
// theme switching, backup management, and a clean, intuitive design.
// ------------------------------------------------------------------------------

#include "mainwindow.hpp"
#include <QApplication>          // Qt application framework
#include <QVBoxLayout>           // Vertical layout manager
#include <QHBoxLayout>           // Horizontal layout manager
#include <QPushButton>           // Button widget
#include <QLineEdit>             // Text input widget
#include <QListWidget>           // List display widget
#include <QLabel>                // Text label widget
#include <QGroupBox>             // Group container widget
#include <QMessageBox>           // Dialog boxes
#include <QTimer>                // Timer for animations
#include <QIcon>                 // Icon handling
#include <QPixmap>               // Image handling
#include <QPainter>              // Custom painting
#include <QDialog>               // Custom dialog windows
#include <QFont>                 // Font customization
#include <QGraphicsOpacityEffect> // Visual effects
#include <QPropertyAnimation>    // Animation framework

// ------------------------------------------------------------------------------
// Constructor
// Initializes the main window with all UI components and functionality
// ------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), isDarkTheme(false) {
    
    // Window configuration
    setWindowTitle("AliaCan - Alias Manager");
    setWindowIcon(createAppIcon());
    setGeometry(100, 100, 1000, 750);  // Initial position and size
    setMinimumSize(900, 650);          // Minimum window size
    
    // Initialize core functionality
    initializeShellDetection();
    initializeUI();
    setupConnections();
    loadAliasesFromFile();
    updateShellInfo();
    applyStylesheet();
}

// ------------------------------------------------------------------------------
// Destructor
// Using default destructor as smart pointers handle cleanup
// ------------------------------------------------------------------------------
MainWindow::~MainWindow() = default;

// ------------------------------------------------------------------------------
// Shell Detection Initialization
// Detects current shell and sets up file handlers
// ------------------------------------------------------------------------------
void MainWindow::initializeShellDetection() {
    currentShell = ShellDetector::detectShell();
    configFilePath = ShellDetector::getConfigFilePath(currentShell);
    configHandler = std::make_unique<ConfigFileHandler>(configFilePath, currentShell);
    backupManager = std::make_unique<BackupManager>(configFilePath);
}

// ------------------------------------------------------------------------------
// UI Initialization
// Creates and arranges all UI components with modern design
// ------------------------------------------------------------------------------
void MainWindow::initializeUI() {
    // Create central widget and main layout
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(25, 20, 25, 20);  // Modern margins
    mainLayout->setSpacing(18);                       // Consistent spacing
    
    // --------------------------------------------------------------------------
    // Header Section (Shell Info + Theme Toggle)
    // --------------------------------------------------------------------------
    auto* headerLayout = new QHBoxLayout();
    
    // Shell information label
    shellInfoLabel = new QLabel(this);
    shellInfoLabel->setStyleSheet("font-weight: bold; font-size: 13px; letter-spacing: 0.5px;");
    headerLayout->addWidget(shellInfoLabel);
    headerLayout->addStretch();  // Push theme toggle to the right
    
    // Theme toggle button (emoji for visual appeal)
    themeToggle = new QPushButton("🌙", this);
    themeToggle->setMaximumSize(40, 40);
    themeToggle->setCursor(Qt::PointingHandCursor);
    themeToggle->setStyleSheet("QPushButton { border-radius: 20px; font-size: 18px; border: none; }");
    headerLayout->addWidget(themeToggle);
    
    mainLayout->addLayout(headerLayout);
    
    // --------------------------------------------------------------------------
    // Add Alias Section
    // --------------------------------------------------------------------------
    auto* inputGroup = new QGroupBox("➕ Add New Alias", this);
    inputGroup->setCursor(Qt::ArrowCursor);
    auto* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(12);
    
    // Alias name input
    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel("Alias Name:", this);
    nameLabel->setMinimumWidth(100);
    aliasNameInput = new QLineEdit(this);
    aliasNameInput->setPlaceholderText("e.g., 'll'");
    aliasNameInput->setMaximumWidth(250);
    aliasNameInput->setCursor(Qt::IBeamCursor);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(aliasNameInput);
    nameLayout->addStretch();
    inputLayout->addLayout(nameLayout);
    
    // Command input
    auto* commandLayout = new QHBoxLayout();
    auto* commandLabel = new QLabel("Command:", this);
    commandLabel->setMinimumWidth(100);
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("e.g., 'ls -la'");
    commandInput->setCursor(Qt::IBeamCursor);
    commandLayout->addWidget(commandLabel);
    commandLayout->addWidget(commandInput);
    inputLayout->addLayout(commandLayout);
    
    // Command validation status
    commandStatus = new QLabel(this);
    commandStatus->setStyleSheet("font-size: 11px; font-weight: 500;");
    inputLayout->addWidget(commandStatus);
    
    // Add button
    auto* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("✨ Add Alias", this);
    addButton->setMinimumHeight(36);
    addButton->setMaximumWidth(160);
    addButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    inputLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(inputGroup);
    
    // --------------------------------------------------------------------------
    // Search Section
    // --------------------------------------------------------------------------
    auto* searchLayout = new QVBoxLayout();
    searchLayout->setSpacing(8);
    
    auto* searchLabel = new QLabel("🔍 Search Aliases", this);
    searchLabel->setStyleSheet("font-weight: 600; font-size: 12px; letter-spacing: 0.3px;");
    searchLayout->addWidget(searchLabel);
    
    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Type alias name or command to filter...");
    searchInput->setMaximumHeight(38);
    searchInput->setCursor(Qt::IBeamCursor);
    searchLayout->addWidget(searchInput);
    
    mainLayout->addLayout(searchLayout);
    
    // --------------------------------------------------------------------------
    // Alias List Section
    // --------------------------------------------------------------------------
    auto* listGroup = new QGroupBox("📋 Current Aliases", this);
    listGroup->setCursor(Qt::ArrowCursor);
    auto* listLayout = new QVBoxLayout(listGroup);
    listLayout->setSpacing(12);
    
    aliasList = new QListWidget(this);
    aliasList->setMinimumHeight(280);
    aliasList->setCursor(Qt::PointingHandCursor);
    listLayout->addWidget(aliasList);
    
    // Action buttons for alias list
    auto* listButtonLayout = new QHBoxLayout();
    listButtonLayout->setSpacing(10);
    
    removeButton = new QPushButton("❌ Remove", this);
    removeButton->setMinimumHeight(34);
    removeButton->setCursor(Qt::PointingHandCursor);
    
    refreshButton = new QPushButton("🔄 Refresh", this);
    refreshButton->setMinimumHeight(34);
    refreshButton->setCursor(Qt::PointingHandCursor);
    
    backupButton = new QPushButton("💾 View Backups", this);
    backupButton->setMinimumHeight(34);
    backupButton->setCursor(Qt::PointingHandCursor);
    
    restoreButton = new QPushButton("⚡ Restore", this);
    restoreButton->setMinimumHeight(34);
    restoreButton->setCursor(Qt::PointingHandCursor);
    
    listButtonLayout->addWidget(removeButton);
    listButtonLayout->addWidget(refreshButton);
    listButtonLayout->addStretch();
    listButtonLayout->addWidget(backupButton);
    listButtonLayout->addWidget(restoreButton);
    
    listLayout->addLayout(listButtonLayout);
    mainLayout->addWidget(listGroup);
    
    // --------------------------------------------------------------------------
    // Status Bar
    // --------------------------------------------------------------------------
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("font-size: 12px; font-weight: 500;");
    mainLayout->addWidget(statusLabel);
}

// ------------------------------------------------------------------------------
// Signal-Slot Connections
// Connects UI events to their handler methods
// ------------------------------------------------------------------------------
void MainWindow::setupConnections() {
    // Button clicks
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddAlias);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveAlias);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(backupButton, &QPushButton::clicked, this, &MainWindow::onShowBackups);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::onRestoreBackup);
    
    // List interactions
    connect(aliasList, &QListWidget::itemSelectionChanged, this, &MainWindow::onAliasSelected);
    
    // Input field changes
    connect(aliasNameInput, &QLineEdit::textChanged, this, &MainWindow::onNameChanged);
    connect(commandInput, &QLineEdit::textChanged, this, &MainWindow::onCommandChanged);
    
    // Theme and search
    connect(themeToggle, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    connect(searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
}

// ------------------------------------------------------------------------------
// Load Aliases from Configuration File
// ------------------------------------------------------------------------------
void MainWindow::loadAliasesFromFile() {
    try {
        currentAliases = configHandler->loadAliases();
        updateAliasList();
    } catch (const std::exception& e) {
        showError("Error", QString("Failed to load aliases: ") + e.what());
    }
}

// ------------------------------------------------------------------------------
// Update Shell Information Display
// ------------------------------------------------------------------------------
void MainWindow::updateShellInfo() {
    std::string shellName = ShellDetector::getShellName(currentShell);
    shellInfoLabel->setText(
        QString::fromStdString(
            "🖥️  Detected: " + shellName + " | Config: " + configFilePath
        )
    );
}

// ------------------------------------------------------------------------------
// Update Alias List Widget
// ------------------------------------------------------------------------------
void MainWindow::updateAliasList() {
    aliasList->clear();
    for (const auto& alias : currentAliases) {
        // Format: "alias_name = command"
        aliasList->addItem(
            QString::fromStdString(alias.name + " = " + alias.command)
        );
    }
    statusLabel->setText(QString("Total aliases: %1").arg(currentAliases.size()));
}

// ------------------------------------------------------------------------------
// Filter Alias List Based on Search Text
// ------------------------------------------------------------------------------
void MainWindow::filterAliasList(const QString& searchText) {
    for (int i = 0; i < aliasList->count(); ++i) {
        QListWidgetItem* item = aliasList->item(i);
        bool matches = item->text().contains(searchText, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void MainWindow::onSearchTextChanged(const QString& text) {
    filterAliasList(text);
}

// ------------------------------------------------------------------------------
// Toggle Between Light and Dark Themes
// ------------------------------------------------------------------------------
void MainWindow::toggleTheme() {
    isDarkTheme = !isDarkTheme;
    themeToggle->setText(isDarkTheme ? "☀️" : "🌙");
    applyStylesheet();
    
    // Add smooth transition animation
    auto* effect = new QGraphicsOpacityEffect();
    centralWidget()->setGraphicsEffect(effect);
    
    auto* opacityAnim = new QPropertyAnimation(effect, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(0.7);
    opacityAnim->setEndValue(1.0);
    opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

#include <chrono>
#include <iomanip>
#include <sstream>

std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}

// ------------------------------------------------------------------------------
// Add New Alias Handler
// ------------------------------------------------------------------------------
void MainWindow::onAddAlias() {
    QString aliasName = aliasNameInput->text().trimmed();
    QString command = commandInput->text().trimmed();
    
    if (!validateInput(aliasName, command)) {
        return;
    }
    
    // Create backup before modification (safety first!)
    std::string backupPath = backupManager->createBackup();
    if (backupPath.empty()) {
        showError("Backup Error", "Failed to create backup. Operation cancelled.");
        return;
    }
    
    // Create and add the alias
    Alias newAlias{aliasName.toStdString(), command.toStdString(), std::string(),true, getCurrentDate(), getCurrentDate()};
    if (!configHandler->addAlias(newAlias)) {
        showError("Error", 
            QString::fromStdString("Failed to add alias: " + configHandler->getLastError())
        );
        return;
    }
    
    showSuccess("✨ Alias added successfully!");
    clearInputFields();
    loadAliasesFromFile();  // Refresh the list
}

// ------------------------------------------------------------------------------
// Remove Selected Alias Handler
// ------------------------------------------------------------------------------
void MainWindow::onRemoveAlias() {
    QListWidgetItem* currentItem = aliasList->currentItem();
    if (!currentItem) {
        showError("Error", "Please select an alias to remove.");
        return;
    }
    
    // Extract alias name from list item
    QString aliasName = currentItem->text().split(" = ")[0].trimmed();
    
    // Confirm deletion
    if (QMessageBox::question(
        this, 
        "Confirm Deletion", 
        QString("Remove alias '%1'?").arg(aliasName),
        QMessageBox::Yes | QMessageBox::No
    ) != QMessageBox::Yes) {
        return;
    }
    
    // Create backup before removal
    std::string backupPath = backupManager->createBackup();
    if (backupPath.empty()) {
        showError("Backup Error", "Failed to create backup. Operation cancelled.");
        return;
    }
    
    // Remove the alias
    if (!configHandler->removeAlias(aliasName.toStdString())) {
        showError("Error", 
            QString::fromStdString("Failed to remove alias: " + configHandler->getLastError())
        );
        return;
    }
    
    showSuccess("❌ Alias removed successfully!");
    loadAliasesFromFile();  // Refresh the list
}

// ------------------------------------------------------------------------------
// Refresh Alias List Handler
// ------------------------------------------------------------------------------
void MainWindow::onRefresh() {
    loadAliasesFromFile();
    showSuccess("🔄 Alias list refreshed!");
}

// ------------------------------------------------------------------------------
// Alias Selection Handler
// Loads selected alias into input fields for editing
// ------------------------------------------------------------------------------
void MainWindow::onAliasSelected() {
    QListWidgetItem* currentItem = aliasList->currentItem();
    if (!currentItem) return;
    
    // Parse "alias_name = command" format
    QStringList parts = currentItem->text().split(" = ");
    if (parts.size() == 2) {
        isModifying = true;  // Prevent recursive updates
        aliasNameInput->setText(parts[0].trimmed());
        commandInput->setText(parts[1].trimmed());
        isModifying = false;
    }
}

// ------------------------------------------------------------------------------
// Alias Name Input Handler
// Updates button text based on whether we're adding or editing
// ------------------------------------------------------------------------------
void MainWindow::onNameChanged(const QString& text) {
    if (isModifying) return;
    addButton->setText(text.isEmpty() ? "✨ Add Alias" : "⚙️  Update Alias");
}

// ------------------------------------------------------------------------------
// Command Input Handler
// Validates command in real-time and updates status
// ------------------------------------------------------------------------------
void MainWindow::onCommandChanged(const QString& text) {
    // Enable add button only if both fields have content
    addButton->setEnabled(!aliasNameInput->text().isEmpty() && !text.isEmpty());
    
    // Real-time command validation
    bool valid = AliasManager::validateCommand(text.toStdString());
    commandStatus->setText(valid ? "✅ Valid command" : "❌ Invalid command");
    
    // Color-coded validation feedback
    commandStatus->setStyleSheet(
        QString("color: %1; font-size: 11px; font-weight: 500;")
            .arg(valid ? "#51cf66" : "#ff6b6b")  // Green for valid, red for invalid
    );
}

// ------------------------------------------------------------------------------
// Show Backup Dialog Handler
// Displays all available backups in a modal dialog
// ------------------------------------------------------------------------------
void MainWindow::onShowBackups() {
    std::vector<std::string> backups = backupManager->listBackups();
    if (backups.empty()) {
        showError("No Backups", "No backup files found for this configuration.");
        return;
    }
    
    // Create backup selection dialog
    auto* backupDialog = new QDialog(this);
    backupDialog->setWindowTitle("Available Backups");
    backupDialog->setGeometry(150, 150, 550, 450);
    backupDialog->setModal(true);
    
    auto* layout = new QVBoxLayout(backupDialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Dialog title
    auto* titleLabel = new QLabel("💾 Available Backups", backupDialog);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    layout->addWidget(titleLabel);
    
    // Backup list
    auto* backupList = new QListWidget(backupDialog);
    backupList->setCursor(Qt::PointingHandCursor);
    for (const auto& backup : backups) {
        backupList->addItem(QString::fromStdString(backup));
    }
    layout->addWidget(backupList);
    
    // Usage hint
    auto* hintLabel = new QLabel("⬆️ Double-click to restore a backup", backupDialog);
    hintLabel->setStyleSheet("font-size: 11px; font-style: italic;");
    layout->addWidget(hintLabel);
    
    // Double-click to restore
    connect(backupList, &QListWidget::itemDoubleClicked, 
        [this, backupDialog, backupList]() {
            if (!backupList->currentItem()) return;
            
            std::string backup = backupList->currentItem()->text().toStdString();
            if (backupManager->restoreFromBackup(backup)) {
                showSuccess("⚡ Restored from backup!");
                loadAliasesFromFile();
                backupDialog->close();
            } else {
                showError("Error", 
                    QString::fromStdString("Failed to restore: " + backupManager->getLastError())
                );
            }
        }
    );
    
    backupDialog->exec();
}

// ------------------------------------------------------------------------------
// Restore from Latest Backup Handler
// ------------------------------------------------------------------------------
void MainWindow::onRestoreBackup() {
    std::string lastBackup = backupManager->getLastBackupPath();
    if (lastBackup.empty()) {
        showError("Error", "No backup found to restore.");
        return;
    }
    
    // Confirm restoration
    if (QMessageBox::question(
        this, 
        "Confirm Restore", 
        "Restore from most recent backup?",
        QMessageBox::Yes | QMessageBox::No
    ) == QMessageBox::Yes) {
        if (backupManager->restoreFromLastBackup()) {
            showSuccess("⚡ Restored from backup successfully!");
            loadAliasesFromFile();
        } else {
            showError("Error", 
                QString::fromStdString("Failed to restore: " + backupManager->getLastError())
            );
        }
    }
}

// ------------------------------------------------------------------------------
// Validate User Input
// Returns true if input is valid, false otherwise
// ------------------------------------------------------------------------------
bool MainWindow::validateInput(QString& aliasName, QString& command) {
    // Check for empty fields
    if (aliasName.isEmpty() || command.isEmpty()) {
        showError("Validation Error", "Please fill in both alias name and command.");
        return false;
    }
    
    // Validate alias name format
    if (!AliasManager::validateAliasName(aliasName.toStdString())) {
        showError("Invalid Alias Name", 
            "Alias name must contain only alphanumeric characters, underscores, and hyphens."
        );
        return false;
    }
    
    // Validate command
    if (!AliasManager::validateCommand(command.toStdString())) {
        showError("Invalid Command", "Command is too long or empty.");
        return false;
    }
    
    return true;
}

// ------------------------------------------------------------------------------
// Clear Input Fields
// ------------------------------------------------------------------------------
void MainWindow::clearInputFields() {
    aliasNameInput->clear();
    commandInput->clear();
    commandStatus->clear();
    searchInput->clear();
}

// ------------------------------------------------------------------------------
// Show Error Dialog
// ------------------------------------------------------------------------------
void MainWindow::showError(const QString& title, const QString& message) {
    QMessageBox::critical(this, title, message);
}

// ------------------------------------------------------------------------------
// Show Success Notification
// Displays a temporary success message in the status label
// ------------------------------------------------------------------------------
void MainWindow::showSuccess(const QString& message) {
    statusLabel->setText(message);
    statusLabel->setStyleSheet(
        QString("color: %1; font-weight: 600; font-size: 12px;")
            .arg(isDarkTheme ? "#51cf66" : "#2d9a1d")  // Theme-appropriate green
    );
    
    // Clear message after 4 seconds
    QTimer::singleShot(4000, this, [this]() {
        statusLabel->setText("");
        statusLabel->setStyleSheet("font-size: 12px; font-weight: 500;");
    });
}

// ------------------------------------------------------------------------------
// Light Theme Stylesheet
// Modern, clean design with blue accent colors
// ------------------------------------------------------------------------------
QString MainWindow::getLightTheme() const {
    return R"(
QMainWindow{background-color:#f8f9fa}
QGroupBox{color:#1a1a1a;border:2px solid #e0e0e0;border-radius:10px;margin-top:12px;padding-top:12px;font-weight:600;background-color:#ffffff;font-size:12px}
QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 5px 0 5px}
QLineEdit{border:2px solid #e0e0e0;border-radius:6px;padding:8px 12px;background-color:#ffffff;selection-background-color:#2196F3;color:#1a1a1a;font-size:13px}
QLineEdit:focus{border:2px solid #2196F3;background-color:#f0f7ff}
QLineEdit:hover{border:2px solid #90caf9}
QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #2196F3,stop:1 #1976D2);color:white;border:none;border-radius:6px;padding:8px 16px;font-weight:600;font-size:12px}
QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #42a5f5,stop:1 #1565C0)}
QPushButton:pressed{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1565C0,stop:1 #0d47a1)}
QPushButton:disabled{background-color:#cccccc;color:#666666}
QListWidget{border:2px solid #e0e0e0;border-radius:6px;background-color:#ffffff;color:#1a1a1a}
QListWidget::item{padding:8px;border-radius:4px;margin:2px}
QListWidget::item:selected{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #42a5f5,stop:1 #2196F3);color:white;border-radius:4px}
QListWidget::item:hover{background-color:#f0f7ff}
QLabel{color:#1a1a1a})";
}

// ------------------------------------------------------------------------------
// Dark Theme Stylesheet
// GitHub Dark theme inspired design
// ------------------------------------------------------------------------------
QString MainWindow::getDarkTheme() const {
    return R"(
QMainWindow{background-color:#0d1117}
QGroupBox{color:#e0e0e0;border:2px solid #30363d;border-radius:10px;margin-top:12px;padding-top:12px;font-weight:600;background-color:#161b22;font-size:12px}
QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 5px 0 5px}
QLineEdit{border:2px solid #30363d;border-radius:6px;padding:8px 12px;background-color:#0d1117;selection-background-color:#1f6feb;color:#e0e0e0;font-size:13px}
QLineEdit:focus{border:2px solid #1f6feb;background-color:#0d1117}
QLineEdit:hover{border:2px solid #388bfd}
QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1f6feb,stop:1 #1555d6);color:#ffffff;border:none;border-radius:6px;padding:8px 16px;font-weight:600;font-size:12px}
QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #388bfd,stop:1 #1f6feb)}
QPushButton:pressed{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #0969da,stop:1 #0860ca)}
QPushButton:disabled{background-color:#21262d;color:#666666}
QListWidget{border:2px solid #30363d;border-radius:6px;background-color:#0d1117;color:#e0e0e0}
QListWidget::item{padding:8px;border-radius:4px;margin:2px}
QListWidget::item:selected{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #388bfd,stop:1 #1f6feb);color:white;border-radius:4px}
QListWidget::item:hover{background-color:#161b22}
QLabel{color:#e0e0e0})";
}

// ------------------------------------------------------------------------------
// Apply Current Theme
// Sets the application-wide stylesheet
// ------------------------------------------------------------------------------
void MainWindow::applyStylesheet() {
    qApp->setStyle("Fusion");  // Modern Qt style
    qApp->setStyleSheet(isDarkTheme ? getDarkTheme() : getLightTheme());
}

// ------------------------------------------------------------------------------
// Create Application Icon
// Generates a simple gradient icon with "A" for AliaCan
// ------------------------------------------------------------------------------
QIcon MainWindow::createAppIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Create blue gradient background
    QLinearGradient gradient(0, 0, 64, 64);
    gradient.setColorAt(0, QColor(33, 150, 243));  // Light blue
    gradient.setColorAt(1, QColor(21, 101, 192));  // Dark blue
    
    painter.fillRect(0, 0, 64, 64, gradient);
    
    // Draw "A" in center
    QFont font;
    font.setPointSize(36);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "A");
    
    return QIcon(pixmap);
}
