# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /snap/clion/98/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/98/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/valeria/Visual Studio/RN-VS/Block5"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/client.dir/flags.make

CMakeFiles/client.dir/client.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/client.c.o: ../client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/client.dir/client.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/client.c.o   -c "/home/valeria/Visual Studio/RN-VS/Block5/client.c"

CMakeFiles/client.dir/client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/client.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/home/valeria/Visual Studio/RN-VS/Block5/client.c" > CMakeFiles/client.dir/client.c.i

CMakeFiles/client.dir/client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/client.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/home/valeria/Visual Studio/RN-VS/Block5/client.c" -o CMakeFiles/client.dir/client.c.s

CMakeFiles/client.dir/src/sockUtils.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/src/sockUtils.c.o: ../src/sockUtils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/client.dir/src/sockUtils.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/src/sockUtils.c.o   -c "/home/valeria/Visual Studio/RN-VS/Block5/src/sockUtils.c"

CMakeFiles/client.dir/src/sockUtils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/src/sockUtils.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/home/valeria/Visual Studio/RN-VS/Block5/src/sockUtils.c" > CMakeFiles/client.dir/src/sockUtils.c.i

CMakeFiles/client.dir/src/sockUtils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/src/sockUtils.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/home/valeria/Visual Studio/RN-VS/Block5/src/sockUtils.c" -o CMakeFiles/client.dir/src/sockUtils.c.s

CMakeFiles/client.dir/src/message.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/src/message.c.o: ../src/message.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/client.dir/src/message.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/src/message.c.o   -c "/home/valeria/Visual Studio/RN-VS/Block5/src/message.c"

CMakeFiles/client.dir/src/message.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/src/message.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/home/valeria/Visual Studio/RN-VS/Block5/src/message.c" > CMakeFiles/client.dir/src/message.c.i

CMakeFiles/client.dir/src/message.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/src/message.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/home/valeria/Visual Studio/RN-VS/Block5/src/message.c" -o CMakeFiles/client.dir/src/message.c.s

CMakeFiles/client.dir/src/lookup.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/src/lookup.c.o: ../src/lookup.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/client.dir/src/lookup.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/src/lookup.c.o   -c "/home/valeria/Visual Studio/RN-VS/Block5/src/lookup.c"

CMakeFiles/client.dir/src/lookup.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/src/lookup.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/home/valeria/Visual Studio/RN-VS/Block5/src/lookup.c" > CMakeFiles/client.dir/src/lookup.c.i

CMakeFiles/client.dir/src/lookup.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/src/lookup.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/home/valeria/Visual Studio/RN-VS/Block5/src/lookup.c" -o CMakeFiles/client.dir/src/lookup.c.s

CMakeFiles/client.dir/src/packet.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/src/packet.c.o: ../src/packet.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/client.dir/src/packet.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/src/packet.c.o   -c "/home/valeria/Visual Studio/RN-VS/Block5/src/packet.c"

CMakeFiles/client.dir/src/packet.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/src/packet.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/home/valeria/Visual Studio/RN-VS/Block5/src/packet.c" > CMakeFiles/client.dir/src/packet.c.i

CMakeFiles/client.dir/src/packet.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/src/packet.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/home/valeria/Visual Studio/RN-VS/Block5/src/packet.c" -o CMakeFiles/client.dir/src/packet.c.s

# Object files for target client
client_OBJECTS = \
"CMakeFiles/client.dir/client.c.o" \
"CMakeFiles/client.dir/src/sockUtils.c.o" \
"CMakeFiles/client.dir/src/message.c.o" \
"CMakeFiles/client.dir/src/lookup.c.o" \
"CMakeFiles/client.dir/src/packet.c.o"

# External object files for target client
client_EXTERNAL_OBJECTS =

client: CMakeFiles/client.dir/client.c.o
client: CMakeFiles/client.dir/src/sockUtils.c.o
client: CMakeFiles/client.dir/src/message.c.o
client: CMakeFiles/client.dir/src/lookup.c.o
client: CMakeFiles/client.dir/src/packet.c.o
client: CMakeFiles/client.dir/build.make
client: CMakeFiles/client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/client.dir/build: client

.PHONY : CMakeFiles/client.dir/build

CMakeFiles/client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/client.dir/clean

CMakeFiles/client.dir/depend:
	cd "/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/valeria/Visual Studio/RN-VS/Block5" "/home/valeria/Visual Studio/RN-VS/Block5" "/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug" "/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug" "/home/valeria/Visual Studio/RN-VS/Block5/cmake-build-debug/CMakeFiles/client.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/client.dir/depend

