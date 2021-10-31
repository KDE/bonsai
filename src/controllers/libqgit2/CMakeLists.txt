if (LIBGIT2_FOUND)
    include_directories(${LIBGIT2_INCLUDE_DIR})
    add_definitions(-DWITH_LIBGIT2)
endif()

file(GLOB_RECURSE SRC *.cpp)
file(GLOB QGIT2_HEADERS qgit*.h libqgit2_config.h)
file(GLOB_RECURSE QGIT2_PRIVATE_HEADERS *.h)
list(REMOVE_ITEM QGIT2_PRIVATE_HEADERS ${QGIT2_HEADERS})

kde_enable_exceptions(qgit2 PRIVATE)


add_library(qgit2 ${SRC} ${QGIT2_HEADERS} ${QGIT2_PRIVATE_HEADERS})


target_include_directories(qgit2 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(qgit2 ${LIBGIT2_LIBRARIES} Qt5::Core Qt5::Network)
