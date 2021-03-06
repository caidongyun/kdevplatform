project(executescript)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecutescript_PART_UIS
    scriptappconfig.ui
)

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)

kde4_add_ui_files( kdevexecutescript_PART_SRCS ${kdevexecutescript_PART_UIS} )
kde4_add_plugin(kdevexecutescript ${kdevexecutescript_PART_SRCS}  )
target_link_libraries(kdevexecutescript 
    ${KDE4_KDEUI_LIBS} 
    ${KDEVPLATFORM_INTERFACES_LIBRARIES} 
    ${KDEVPLATFORM_UTIL_LIBRARIES} 
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES})

install(TARGETS kdevexecutescript DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install(FILES iexecutescriptplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/executescript COMPONENT Devel)
install(FILES kdevexecutescript.desktop DESTINATION ${SERVICES_INSTALL_DIR})
