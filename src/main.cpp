// ------------------------------------------------------------------------------
// Program Name: AliaCan
// Creator: Xzrayツ
// Description: Main Application Entry Point
// 
// This is the main entry point for the AliaCan application. It initializes
// the Qt application framework, creates and displays the main window,
// and provides global exception handling. The application gracefully
// handles both normal execution and unexpected errors.
// ------------------------------------------------------------------------------

#include <QApplication>   // Qt application framework
#include "mainwindow.hpp" // Main application window
#include <iostream>       // Standard I/O for error reporting

// ------------------------------------------------------------------------------
// Main Function
// Entry point for the AliaCan application.
// 
// Parameters:
//   argc - Argument count from command line
//   argv - Argument vector from command line
// 
// Returns:
//   0   - Successful execution
//   1   - Error occurred during execution
// ------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // --------------------------------------------------------------------------
    // Step 1: Initialize Qt Application
    // Creates the Qt application object which manages the event loop,
    // resources, and application settings.
    // --------------------------------------------------------------------------
    QApplication app(argc, argv);
    
    // --------------------------------------------------------------------------
    // Step 2: Configure Application Metadata
    // Set application information for system integration.
    // --------------------------------------------------------------------------
    app.setApplicationName("AliaCan");
    app.setApplicationVersion("0.0.1.1");
    app.setOrganizationName("AliaCan");
    app.setOrganizationDomain("aliacan.xzray");
    
    // --------------------------------------------------------------------------
    // Step 3: Enable High-DPI Scaling (Optional but recommended)
    // Ensures proper scaling on high-resolution displays.
    // --------------------------------------------------------------------------
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        app.setAttribute(Qt::AA_EnableHighDpiScaling);
    #endif
    
    // --------------------------------------------------------------------------
    // Step 4: Create and Show Main Window
    // Wrapped in try-catch for comprehensive error handling.
    // --------------------------------------------------------------------------
    try {
        // Create the main application window
        MainWindow window;
        
        // Display the window
        window.show();
        
        // ----------------------------------------------------------------------
        // Step 5: Start Qt Event Loop
        // This starts the main event loop which handles user input,
        // window events, timers, and signals/slots.
        // ----------------------------------------------------------------------
        return app.exec();
        
    } catch (const std::exception& e) {
        // ----------------------------------------------------------------------
        // Step 6: Handle Fatal Exceptions
        // Catches any unhandled exceptions that occur during initialization
        // or execution. Provides user feedback via console and exits cleanly.
        // ----------------------------------------------------------------------
        std::cerr << "Fatal error in AliaCan: " << e.what() << '\n';
        std::cerr << "   Application will now exit.\n";
        
        return 1; // Return error code
    }
}
