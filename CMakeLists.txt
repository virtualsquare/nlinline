cmake_minimum_required(VERSION 3.12)
project(nlinline 
		VERSION 0.0.1
    DESCRIPTION "bind mount in user namespaces"
    HOMEPAGE_URL "https://github.com/rd235/userbindmount"
		LANGUAGES C)
include(GNUInstallDirs)

install(FILES nlinline.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
add_subdirectory(man)
