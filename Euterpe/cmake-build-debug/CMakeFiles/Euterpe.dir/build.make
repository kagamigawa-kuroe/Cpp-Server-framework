# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/whz/learning/Cpp-Server-framework/Euterpe

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Euterpe.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/Euterpe.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Euterpe.dir/flags.make

CMakeFiles/Euterpe.dir/src/log.cpp.o: CMakeFiles/Euterpe.dir/flags.make
CMakeFiles/Euterpe.dir/src/log.cpp.o: ../src/log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Euterpe.dir/src/log.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Euterpe.dir/src/log.cpp.o -c /Users/whz/learning/Cpp-Server-framework/Euterpe/src/log.cpp

CMakeFiles/Euterpe.dir/src/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Euterpe.dir/src/log.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/whz/learning/Cpp-Server-framework/Euterpe/src/log.cpp > CMakeFiles/Euterpe.dir/src/log.cpp.i

CMakeFiles/Euterpe.dir/src/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Euterpe.dir/src/log.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/whz/learning/Cpp-Server-framework/Euterpe/src/log.cpp -o CMakeFiles/Euterpe.dir/src/log.cpp.s

# Object files for target Euterpe
Euterpe_OBJECTS = \
"CMakeFiles/Euterpe.dir/src/log.cpp.o"

# External object files for target Euterpe
Euterpe_EXTERNAL_OBJECTS =

Euterpe: CMakeFiles/Euterpe.dir/src/log.cpp.o
Euterpe: CMakeFiles/Euterpe.dir/build.make
Euterpe: CMakeFiles/Euterpe.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Euterpe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Euterpe.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Euterpe.dir/build: Euterpe
.PHONY : CMakeFiles/Euterpe.dir/build

CMakeFiles/Euterpe.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Euterpe.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Euterpe.dir/clean

CMakeFiles/Euterpe.dir/depend:
	cd /Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/whz/learning/Cpp-Server-framework/Euterpe /Users/whz/learning/Cpp-Server-framework/Euterpe /Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug /Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug /Users/whz/learning/Cpp-Server-framework/Euterpe/cmake-build-debug/CMakeFiles/Euterpe.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Euterpe.dir/depend
