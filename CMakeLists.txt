# ------------------------------------------------------------------------------
# Program Name: AliaCan
# Creator: Xzrayツ
# Description: CMake Build Configuration for AliaCan - Shell Alias Manager with Auto-Backup
#
# This CMake configuration defines how to build the AliaCan application,
# including compiler settings, dependencies, target definitions, and installation
# rules. It sets up aggressive optimization flags for performance while maintaining
# security hardening features. The configuration also includes test targets and
# an uninstall mechanism.
# ------------------------------------------------------------------------------

# Minimum required CMake version.
cmake_minimum_required(VERSION 3.28)

# Using Clang for better diagnostics and modern C++ support.
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Define the project with metadata.
project(alia-can
    VERSION 0.0.1.1
    DESCRIPTION "AliaCan - Shell Alias Manager with Auto-Backup"
    LANGUAGES CXX
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
message(STATUS "Threads found: ${Threads_FOUND}")
if(Threads_FOUND)
    message(STATUS "Threads library: ${CMAKE_THREAD_LIBS_INIT}")
endif()

# C++ language standard configuration..
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable strict warnings for code quality.
add_compile_options(-Wall -Wextra -Wpedantic)

# Optimization and security hardening flags for C++ compilation.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -march=native \
    -mtune=native \
    -O3 \
    -pipe \
    -fno-plt \
    -fexceptions \
    -Wp,-D_FORTIFY_SOURCE=3 \
    -fstack-clash-protection \
    -fcf-protection \
    -fstack-protector \
    -flto \
    -fomit-frame-pointer \
    -Wstrict-aliasing \
    -ffunction-sections \
    -fdata-sections \
    -fPIC \
    -fvisibility=hidden \
    -ftree-vectorize \
    -finline-functions \
    -fstrict-aliasing \
    -fcommon \
")

# Linker flags for executable optimization and security.
# LLD (LLVM linker) is used for faster linking and better optimizations.
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -Wl,-O3 \
    -Wl,--sort-common \
    -Wl,--as-needed \
    -Wl,-z,relro \
    -Wl,-z,now \
    -Wl,-z,pack-relative-relocs \
    -Wl,--gc-sections \
    -Wl,--icf=all \
    -fuse-ld=lld \
")

# Optimization flags for shared libraries.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
    -Wl,-O3 -Wl,--sort-common -Wl,--as-needed \
    -Wl,-z,relro -Wl,-z,now -Wl,-z,pack-relative-relocs \
    Wl,--gc-sections -Wl,--icf=all -fuse-ld=lld \
")

# Optimization flags for loadable modules.
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} \
    -Wl,-O3 -Wl,--sort-common -Wl,--as-needed \
    -Wl,-z,relro -Wl,-z,now -Wl,-z,pack-relative-relocs \
    -Wl,--gc-sections -Wl,--icf=all -fuse-ld=lld \
")

# Link Time Optimization flag for whole-program optimization.
add_compile_options(-flto)

# Enable Qt's meta-object compiler automation for easier Qt development.
set(CMAKE_AUTOMOC ON)                  # Automatic invocation of moc (Meta-Object Compiler)
set(CMAKE_AUTORCC ON)                  # Automatic processing of Qt resource files
set(CMAKE_AUTOUIC ON)                  # Automatic invocation of uic (User Interface Compiler)

# Find and require Qt6 components needed by the application.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Please install Qt6 development packages.")
else()
    message(STATUS "Qt6 version: ${Qt6_VERSION}")
    message(STATUS "Qt6 Core found: ${Qt6Core_FOUND}")
    message(STATUS "Qt6 Gui found: ${Qt6Gui_FOUND}")
    message(STATUS "Qt6 Widgets found: ${Qt6Widgets_FOUND}")
endif()

# Define application source and header files.
set(APP_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/shelldetector.cpp
    src/aliasmanager.cpp
    src/configfilehandler.cpp
    src/backupmanager.cpp
)

set(APP_HEADERS
    src/mainwindow.hpp
    src/shelldetector.hpp
    src/aliasmanager.hpp
    src/configfilehandler.hpp
    src/backupmanager.hpp
)

# Create the main executable target.
add_executable(alia-can ${APP_SOURCES} ${APP_HEADERS})

# Link Qt libraries to the executable.
target_link_libraries(alia-can
    Qt6::Core               # Core Qt functionality
    Qt6::Gui                # GUI components
    Qt6::Widgets            # Widget-based UI components
    Threads::Threads        # Link Threads library
)

# Add include directories for the target.
target_include_directories(alia-can PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set target properties for optimization
target_compile_features(alia-can PRIVATE cxx_std_23)

# Enable testing support for the project.
enable_testing()

# Test target configuration - includes both test files and actual source files.
set(TEST_SOURCES
    tests/main.cpp
    tests/test_shelldetector.cpp
    tests/test_aliasmanager.cpp
    tests/test_confighandler.cpp
    src/shelldetector.cpp
    src/aliasmanager.cpp
    src/configfilehandler.cpp
    src/backupmanager.cpp
)

# Create test executable.
add_executable(alia-can-tests ${TEST_SOURCES})
target_include_directories(alia-can-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link Threads to test executable as well
target_link_libraries(alia-can-tests Threads::Threads)

# Register the test with CTest.
add_test(NAME AliaCan-Tests COMMAND alia-can-tests)

# Installation rules.
install(TARGETS alia-can DESTINATION /usr/local/bin)

# Create uninstall target if it doesn't already exist.
# This generates and registers a custom target that can uninstall the application.
if(NOT TARGET uninstall)
    # Configure the uninstall script template.
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )

    # Add custom target that runs the uninstall script.
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif()

# Display configuration summary for the user.
message(STATUS "========================================")
message(STATUS "AliaCan Configuration")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Threads Found: ${Threads_FOUND}")
message(STATUS "Qt6 Found: ${Qt6_FOUND}")
if(Qt6_FOUND)
    message(STATUS "Qt6 Version: ${Qt6_VERSION}")
endif()
message(STATUS "========================================")
