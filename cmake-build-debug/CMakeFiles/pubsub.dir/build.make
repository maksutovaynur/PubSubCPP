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
include CMakeFiles/pubsub.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/pubsub.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pubsub.dir/flags.make

CMakeFiles/pubsub.dir/pubsub.cpp.o: CMakeFiles/pubsub.dir/flags.make
CMakeFiles/pubsub.dir/pubsub.cpp.o: ../pubsub.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aynur/Repos/PubSubCPP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/pubsub.dir/pubsub.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/pubsub.dir/pubsub.cpp.o -c /home/aynur/Repos/PubSubCPP/pubsub.cpp

CMakeFiles/pubsub.dir/pubsub.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/pubsub.dir/pubsub.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aynur/Repos/PubSubCPP/pubsub.cpp > CMakeFiles/pubsub.dir/pubsub.cpp.i

CMakeFiles/pubsub.dir/pubsub.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/pubsub.dir/pubsub.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aynur/Repos/PubSubCPP/pubsub.cpp -o CMakeFiles/pubsub.dir/pubsub.cpp.s

CMakeFiles/pubsub.dir/pubsub.cpp.o.requires:

.PHONY : CMakeFiles/pubsub.dir/pubsub.cpp.o.requires

CMakeFiles/pubsub.dir/pubsub.cpp.o.provides: CMakeFiles/pubsub.dir/pubsub.cpp.o.requires
	$(MAKE) -f CMakeFiles/pubsub.dir/build.make CMakeFiles/pubsub.dir/pubsub.cpp.o.provides.build
.PHONY : CMakeFiles/pubsub.dir/pubsub.cpp.o.provides

CMakeFiles/pubsub.dir/pubsub.cpp.o.provides.build: CMakeFiles/pubsub.dir/pubsub.cpp.o


# Object files for target pubsub
pubsub_OBJECTS = \
"CMakeFiles/pubsub.dir/pubsub.cpp.o"

# External object files for target pubsub
pubsub_EXTERNAL_OBJECTS =

pubsub: CMakeFiles/pubsub.dir/pubsub.cpp.o
pubsub: CMakeFiles/pubsub.dir/build.make
pubsub: /usr/lib/x86_64-linux-gnu/librt.so
pubsub: /usr/lib/x86_64-linux-gnu/libpthread.so
pubsub: CMakeFiles/pubsub.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/aynur/Repos/PubSubCPP/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable pubsub"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pubsub.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pubsub.dir/build: pubsub

.PHONY : CMakeFiles/pubsub.dir/build

CMakeFiles/pubsub.dir/requires: CMakeFiles/pubsub.dir/pubsub.cpp.o.requires

.PHONY : CMakeFiles/pubsub.dir/requires

CMakeFiles/pubsub.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pubsub.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pubsub.dir/clean

CMakeFiles/pubsub.dir/depend:
	cd /home/aynur/Repos/PubSubCPP/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/aynur/Repos/PubSubCPP /home/aynur/Repos/PubSubCPP /home/aynur/Repos/PubSubCPP/cmake-build-debug /home/aynur/Repos/PubSubCPP/cmake-build-debug /home/aynur/Repos/PubSubCPP/cmake-build-debug/CMakeFiles/pubsub.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pubsub.dir/depend
