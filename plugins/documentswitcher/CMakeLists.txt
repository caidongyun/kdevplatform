project(documentswitcher)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9530 )

########### next target ###############

set(kdevdocumentswitcher_PART_SRCS
    documentswitcherplugin.cpp
    documentswitchertreeview.cpp
)

kde4_add_plugin(kdevdocumentswitcher ${kdevdocumentswitcher_PART_SRCS})
target_link_libraries(kdevdocumentswitcher kdevplatforminterfaces sublime kdevplatforminterfaces )

install(TARGETS kdevdocumentswitcher DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevdocumentswitcher.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevdocumentswitcher.rc DESTINATION ${DATA_INSTALL_DIR}/kdevdocumentswitcher )


