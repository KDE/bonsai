include_directories(${LIBGIT2_INCLUDE_DIR} ${LIBQGIT2_INCLUDE_DIR})

set(bonsai_SRCS
    main.cpp
    controllers/project.cpp
    controllers/projectmanager.cpp
    controllers/branchesmanager.cpp
    controllers/actionrunner.cpp
    models/projectsmodel.cpp
    assets.qrc
    )

if(QUICK_COMPILER)
    qtquick_compiler_add_resources(bonsai_QML_QRC qml.qrc)
else()
    qt5_add_resources(bonsai_QML_QRC qml.qrc)
endif()

add_executable(${PROJECT_NAME}
    ${bonsai_SRCS}
    ${bonsai_QML_QRC}
    )

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME} MauiKit MauiKit::FileBrowsing Qt5::Widgets Qt5::Quick Qt5::Core Qt5::Concurrent KF5::CoreAddons KF5::I18n libkommit_bonsai)

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES assets/bonsai.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
