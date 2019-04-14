#
# Build me by running `cmake .`, then `make`.
#
# For the extra-keen, you can do an out-of-tree build with make(1) or even
# a non-make tool like the incredible Ninja:
#
# ```sh
# $ mkdir build
# $ cd build
# $ cmake -G Ninja ..
# $ ninja
# ```
#

cmake_minimum_required(VERSION 3.0)

# Give our project a name:
project(rusage-lab)

# Point the compiler at the directory containing our header files:
include_directories("include")

# Build your implementation of get_rusage_string as a shared library:
add_library(rusage SHARED rusage.c)

# Build the main() function with threading support:
find_package(Threads)
add_executable(lab main.c)
target_link_libraries(lab ${CMAKE_THREAD_LIBS_INIT})

# Build GUI libraries:
add_subdirectory(cmdline)
add_subdirectory(gui)

# Link against the command-line UI as a reasonable default:
target_link_libraries(lab cmdlineui rusage)
