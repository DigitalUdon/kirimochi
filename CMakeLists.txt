cmake_minimum_required(VERSION 3.16)

project(Kirimochi VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Sql WebEngineQuick Test)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appKirimochi
    src/main.cpp
    src/DatabaseManager.cpp
    src/SettingModel.h
    src/PreviewExporter.h
)

target_include_directories(appKirimochi PRIVATE src)

if (MSVC)
    target_compile_options(appKirimochi PRIVATE /Zc:__cplusplus /permissive-)
endif()

set(plain_qml_files
    qml/Main.qml
    qml/HomePage.qml
    qml/ArchivePlayerPage.qml
    qml/DraftEditorPage.qml
    qml/LibraryPage.qml
    qml/AssetLibraryPage.qml
    qml/IdeaLibraryPage.qml
    qml/SettingDialog.qml
    qml/CardGridView.qml
    qml/PlayerControls.qml
)
set(qml_singletons
    qml/Theme.qml
)
set_source_files_properties(${qml_singletons}
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(appKirimochi
    URI Kirimochi
    VERSION 1.0

    QML_FILES
        ${plain_qml_files}
        ${qml_singletons}

    RESOURCES
        assets/images/asset_note.png
        assets/images/asset_video.png
        assets/images/asset_font.png
        assets/images/asset_other.png

    SOURCES
        src/AppNavigator.h
        src/viewmodels/ArchivePlayerViewModel.h
        src/viewmodels/AssetLibraryViewModel.h
        src/viewmodels/DraftEditorViewModel.h
        src/viewmodels/HomeViewModel.h
        src/viewmodels/IdeaLibraryViewModel.h
        src/viewmodels/SettingViewModel.h
        src/viewmodels/models/AssetListModel.h
        src/viewmodels/models/DraftBlockListModel.h
        src/viewmodels/models/IdeaListModel.h
        src/viewmodels/models/MemoBlockListModel.h
        src/viewmodels/models/ProjectListModel.h
        src/viewmodels/models/SrtCueListModel.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appKirimochi PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appKirimochi
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appKirimochi
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Sql
    PRIVATE Qt6::WebEngineQuick
)

install(TARGETS appKirimochi
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_compile_definitions(appKirimochi PRIVATE
    $<$<CONFIG:Debug>:QT_QML_DEBUG>
    $<$<CONFIG:Debug>:APP_QML_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}">
)

enable_testing()

set(KIRIMOCHI_TEST_COMMON_SOURCES
    src/DatabaseManager.cpp
    src/PathUtils.h
    src/PreviewExporter.h
    src/viewmodels/ArchivePlayerViewModel.h
    src/viewmodels/AssetLibraryViewModel.h
    src/viewmodels/DraftEditorViewModel.h
    src/viewmodels/HomeViewModel.h
    src/viewmodels/IdeaLibraryViewModel.h
    src/viewmodels/models/AssetListModel.h
    src/viewmodels/models/DraftBlockListModel.h
    src/viewmodels/models/IdeaListModel.h
    src/viewmodels/models/MemoBlockListModel.h
    src/viewmodels/models/ProjectListModel.h
    src/viewmodels/models/SrtCueListModel.h
)

set(KIRIMOCHI_TEST_FILES
    tests/test_path_utils.cpp
    tests/test_srt_utils.cpp
    tests/test_preview_exporter.cpp
    tests/test_archive_player_view_model.cpp
    tests/test_asset_library_view_model.cpp
    tests/test_draft_editor_view_model.cpp
    tests/test_home_view_model.cpp
    tests/test_idea_library_view_model.cpp
)

foreach(test_file ${KIRIMOCHI_TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)

    qt_add_executable(${test_name}
        ${test_file}
        ${KIRIMOCHI_TEST_COMMON_SOURCES}
    )

    target_include_directories(${test_name} PRIVATE src)

    target_link_libraries(${test_name} PRIVATE
        PRIVATE Qt6::Test
        PRIVATE Qt6::Sql
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
