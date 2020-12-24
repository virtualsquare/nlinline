cmake_minimum_required(VERSION 3.12)
project(nlinline 
		VERSION 0.2.1
    DESCRIPTION "A quick and clean API for networking configuration via NETLINK"
    HOMEPAGE_URL "https://github.com/virtualsquare/nlinline"
		LANGUAGES C)
include(GNUInstallDirs)

install(FILES nlinline.h nlinline+.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
add_subdirectory(man)
