# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /home/aynur/Installed/clion/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/aynur/Installed/clion/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/aynur/Repos/PubSubCPP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/aynur/Repos/PubSubCPP/release

# Include any dependencies generated for this target.
include CMakeFiles/clap_pubsub.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/clap_pubsub.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/clap_pubsub.dir/flags.make

CMakeFiles/clap_pubsub.dir/lib/topic.cpp.o: CMakeFiles/clap_pubsub.dir/flags.make
CMakeFiles/clap_pubsub.dir/lib/topic.cpp.o: ../lib/topic.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/aynur/Repos/PubSubCPP/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/clap_pubsub.dir/lib/topic.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/clap_pubsub.dir/lib/topic.cpp.o -c /home/aynur/Repos/PubSubCPP/lib/topic.cpp

CMakeFiles/clap_pubsub.dir/lib/topic.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/clap_pubsub.dir/lib/topic.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/aynur/Repos/PubSubCPP/lib/topic.cpp > CMakeFiles/clap_pubsub.dir/lib/topic.cpp.i

CMakeFiles/clap_pubsub.dir/lib/topic.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/clap_pubsub.dir/lib/topic.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/aynur/Repos/PubSubCPP/lib/topic.cpp -o CMakeFiles/clap_pubsub.dir/lib/topic.cpp.s

# Object files for target clap_pubsub
clap_pubsub_OBJECTS = \
"CMakeFiles/clap_pubsub.dir/lib/topic.cpp.o"

# External object files for target clap_pubsub
clap_pubsub_EXTERNAL_OBJECTS =

libclap_pubsub.so: CMakeFiles/clap_pubsub.dir/lib/topic.cpp.o
libclap_pubsub.so: CMakeFiles/clap_pubsub.dir/build.make
libclap_pubsub.so: /usr/lib/x86_64-linux-gnu/librt.so
libclap_pubsub.so: /usr/lib/x86_64-linux-gnu/libpthread.so
libclap_pubsub.so: CMakeFiles/clap_pubsub.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/aynur/Repos/PubSubCPP/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libclap_pubsub.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/clap_pubsub.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/clap_pubsub.dir/build: libclap_pubsub.so

.PHONY : CMakeFiles/clap_pubsub.dir/build

CMakeFiles/clap_pubsub.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/clap_pubsub.dir/cmake_clean.cmake
.PHONY : CMakeFiles/clap_pubsub.dir/clean

CMakeFiles/clap_pubsub.dir/depend:
	cd /home/aynur/Repos/PubSubCPP/release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/aynur/Repos/PubSubCPP /home/aynur/Repos/PubSubCPP /home/aynur/Repos/PubSubCPP/release /home/aynur/Repos/PubSubCPP/release /home/aynur/Repos/PubSubCPP/release/CMakeFiles/clap_pubsub.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/clap_pubsub.dir/depend

