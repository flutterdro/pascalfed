# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/flutterdro/dev/somelang

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/flutterdro/dev/somelang/build

# Include any dependencies generated for this target.
include lib/CMakeFiles/frontend.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lib/CMakeFiles/frontend.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/CMakeFiles/frontend.dir/progress.make

# Include the compile flags for this target's objects.
include lib/CMakeFiles/frontend.dir/flags.make

lib/CMakeFiles/frontend.dir/scanner/lex.cpp.o: lib/CMakeFiles/frontend.dir/flags.make
lib/CMakeFiles/frontend.dir/scanner/lex.cpp.o: /Users/flutterdro/dev/somelang/lib/scanner/lex.cpp
lib/CMakeFiles/frontend.dir/scanner/lex.cpp.o: lib/CMakeFiles/frontend.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/flutterdro/dev/somelang/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/CMakeFiles/frontend.dir/scanner/lex.cpp.o"
	cd /Users/flutterdro/dev/somelang/build/lib && /opt/homebrew/opt/llvm/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/CMakeFiles/frontend.dir/scanner/lex.cpp.o -MF CMakeFiles/frontend.dir/scanner/lex.cpp.o.d -o CMakeFiles/frontend.dir/scanner/lex.cpp.o -c /Users/flutterdro/dev/somelang/lib/scanner/lex.cpp

lib/CMakeFiles/frontend.dir/scanner/lex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/frontend.dir/scanner/lex.cpp.i"
	cd /Users/flutterdro/dev/somelang/build/lib && /opt/homebrew/opt/llvm/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/flutterdro/dev/somelang/lib/scanner/lex.cpp > CMakeFiles/frontend.dir/scanner/lex.cpp.i

lib/CMakeFiles/frontend.dir/scanner/lex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/frontend.dir/scanner/lex.cpp.s"
	cd /Users/flutterdro/dev/somelang/build/lib && /opt/homebrew/opt/llvm/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/flutterdro/dev/somelang/lib/scanner/lex.cpp -o CMakeFiles/frontend.dir/scanner/lex.cpp.s

lib/CMakeFiles/frontend.dir/parser/parser.cpp.o: lib/CMakeFiles/frontend.dir/flags.make
lib/CMakeFiles/frontend.dir/parser/parser.cpp.o: /Users/flutterdro/dev/somelang/lib/parser/parser.cpp
lib/CMakeFiles/frontend.dir/parser/parser.cpp.o: lib/CMakeFiles/frontend.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/flutterdro/dev/somelang/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object lib/CMakeFiles/frontend.dir/parser/parser.cpp.o"
	cd /Users/flutterdro/dev/somelang/build/lib && /opt/homebrew/opt/llvm/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT lib/CMakeFiles/frontend.dir/parser/parser.cpp.o -MF CMakeFiles/frontend.dir/parser/parser.cpp.o.d -o CMakeFiles/frontend.dir/parser/parser.cpp.o -c /Users/flutterdro/dev/somelang/lib/parser/parser.cpp

lib/CMakeFiles/frontend.dir/parser/parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/frontend.dir/parser/parser.cpp.i"
	cd /Users/flutterdro/dev/somelang/build/lib && /opt/homebrew/opt/llvm/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/flutterdro/dev/somelang/lib/parser/parser.cpp > CMakeFiles/frontend.dir/parser/parser.cpp.i

lib/CMakeFiles/frontend.dir/parser/parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/frontend.dir/parser/parser.cpp.s"
	cd /Users/flutterdro/dev/somelang/build/lib && /opt/homebrew/opt/llvm/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/flutterdro/dev/somelang/lib/parser/parser.cpp -o CMakeFiles/frontend.dir/parser/parser.cpp.s

# Object files for target frontend
frontend_OBJECTS = \
"CMakeFiles/frontend.dir/scanner/lex.cpp.o" \
"CMakeFiles/frontend.dir/parser/parser.cpp.o"

# External object files for target frontend
frontend_EXTERNAL_OBJECTS =

lib/libfrontend.a: lib/CMakeFiles/frontend.dir/scanner/lex.cpp.o
lib/libfrontend.a: lib/CMakeFiles/frontend.dir/parser/parser.cpp.o
lib/libfrontend.a: lib/CMakeFiles/frontend.dir/build.make
lib/libfrontend.a: lib/CMakeFiles/frontend.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/flutterdro/dev/somelang/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libfrontend.a"
	cd /Users/flutterdro/dev/somelang/build/lib && $(CMAKE_COMMAND) -P CMakeFiles/frontend.dir/cmake_clean_target.cmake
	cd /Users/flutterdro/dev/somelang/build/lib && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/frontend.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/CMakeFiles/frontend.dir/build: lib/libfrontend.a
.PHONY : lib/CMakeFiles/frontend.dir/build

lib/CMakeFiles/frontend.dir/clean:
	cd /Users/flutterdro/dev/somelang/build/lib && $(CMAKE_COMMAND) -P CMakeFiles/frontend.dir/cmake_clean.cmake
.PHONY : lib/CMakeFiles/frontend.dir/clean

lib/CMakeFiles/frontend.dir/depend:
	cd /Users/flutterdro/dev/somelang/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/flutterdro/dev/somelang /Users/flutterdro/dev/somelang/lib /Users/flutterdro/dev/somelang/build /Users/flutterdro/dev/somelang/build/lib /Users/flutterdro/dev/somelang/build/lib/CMakeFiles/frontend.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : lib/CMakeFiles/frontend.dir/depend

