# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/aynur/Repos/PubSubCPP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/aynur/Repos/PubSubCPP/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/PubSubCPP2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/PubSubCPP2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/PubSubCPP2.dir/flags.make

CMakeFiles/PubSubCPP2.dir/main.cpp.o: CMakeFiles/PubSubCPP2.dir/flags.make
CMakeFiles/PubSubCPP2.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aynur/Repos/PubSubCPP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/PubSubCPP2.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/PubSubCPP2.dir/main.cpp.o -c /home/aynur/Repos/PubSubCPP/main.cpp

CMakeFiles/PubSubCPP2.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/PubSubCPP2.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aynur/Repos/PubSubCPP/main.cpp > CMakeFiles/PubSubCPP2.dir/main.cpp.i

CMakeFiles/PubSubCPP2.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/PubSubCPP2.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aynur/Repos/PubSubCPP/main.cpp -o CMakeFiles/PubSubCPP2.dir/main.cpp.s

CMakeFiles/PubSubCPP2.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/PubSubCPP2.dir/main.cpp.o.requires

CMakeFiles/PubSubCPP2.dir/main.cpp.o.provides: CMakeFiles/PubSubCPP2.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/PubSubCPP2.dir/build.make CMakeFiles/PubSubCPP2.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/PubSubCPP2.dir/main.cpp.o.provides

CMakeFiles/PubSubCPP2.dir/main.cpp.o.provides.build: CMakeFiles/PubSubCPP2.dir/main.cpp.o


# Object files for target PubSubCPP2
PubSubCPP2_OBJECTS = \
"CMakeFiles/PubSubCPP2.dir/main.cpp.o"

# External object files for target PubSubCPP2
PubSubCPP2_EXTERNAL_OBJECTS =

PubSubCPP2: CMakeFiles/PubSubCPP2.dir/main.cpp.o
PubSubCPP2: CMakeFiles/PubSubCPP2.dir/build.make
PubSubCPP2: CMakeFiles/PubSubCPP2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/aynur/Repos/PubSubCPP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable PubSubCPP2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/PubSubCPP2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/PubSubCPP2.dir/build: PubSubCPP2

.PHONY : CMakeFiles/PubSubCPP2.dir/build

CMakeFiles/PubSubCPP2.dir/requires: CMakeFiles/PubSubCPP2.dir/main.cpp.o.requires

.PHONY : CMakeFiles/PubSubCPP2.dir/requires

CMakeFiles/PubSubCPP2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PubSubCPP2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PubSubCPP2.dir/clean

CMakeFiles/PubSubCPP2.dir/depend:
	cd /home/aynur/Repos/PubSubCPP/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/aynur/Repos/PubSubCPP /home/aynur/Repos/PubSubCPP /home/aynur/Repos/PubSubCPP/cmake-build-debug /home/aynur/Repos/PubSubCPP/cmake-build-debug /home/aynur/Repos/PubSubCPP/cmake-build-debug/CMakeFiles/PubSubCPP2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PubSubCPP2.dir/depend
