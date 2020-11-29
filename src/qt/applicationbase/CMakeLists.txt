#--------------------------------------------------------------------
# Inviwo Qt ApplicationBase
project(inviwo-qtapplicationbase)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/inviwoapplicationqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/qtapplicationbasemoduledefine.h
    ${MOC_FILES}
)
ivw_group("Header Files" BASE ${IVW_INCLUDE_DIR}/inviwo/qt/applicationbase/ ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    inviwoapplicationqt.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
find_package(Qt6Core QUIET REQUIRED)
find_package(Qt6Gui QUIET REQUIRED)
find_package(Qt6Widgets QUIET REQUIRED)

qt6_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

add_library(inviwo-qtapplicationbase ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
add_library(inviwo::qtapplicationbase ALIAS inviwo-qtapplicationbase)
target_link_libraries(inviwo-qtapplicationbase PUBLIC inviwo-core Qt6::Core Qt6::Gui Qt6::Widgets)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(IVW_QTAPPLICATIONBASE inviwo-qtapplicationbase)
ivw_define_standard_properties(inviwo-qtapplicationbase)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_comp_targets(qt_applicationbase inviwo-qtapplicationbase)
ivw_qt_add_to_install(inviwo-qtapplicationbase Qt6Core Qt6Gui Qt6Widgets)
ivw_make_package(InviwoQtApplicationBase inviwo-qtapplicationbase)

