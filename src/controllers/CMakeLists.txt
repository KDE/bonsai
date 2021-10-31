cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(GitWrap LANGUAGES CXX VERSION 1.0.0)

find_package(ECM ${KF5_VERSION} NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core)

find_package(LibGit2 REQUIRED)
set_package_properties(Libgit2 PROPERTIES DESCRIPTION "A library to manipulate git repositories"
    PURPOSE "Add git support"
    URL "https://libgit2.org"
    TYPE REQUIRED
)

# Set the Include-Directory here, so we can use "libGitWrap/XXX" includes inside libGitWrap and be
# sure forever that they will work on an installed source tree...
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory (libGitWrap)
#add_subdirectory(libqgit2)
