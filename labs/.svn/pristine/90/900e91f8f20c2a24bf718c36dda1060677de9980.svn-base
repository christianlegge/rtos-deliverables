# Find gtk+ (GUI library):
include(FindPkgConfig)
pkg_check_modules("GTK" REQUIRED "gtk+-2.0")
link_directories(${GTK_LIBRARY_DIRS})

# Build GUI library with Gtk+ CFLAGS and libraries:
add_definitions(${GTK_CFLAGS})
add_library(gui SHARED gtk-ui.c)
target_link_libraries(gui ${GTK_LIBRARIES})
