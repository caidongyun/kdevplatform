
### Configuration module for general kdevelop settings

set( documentview_config_SRC
preferences.cpp
)
set( documentview_config_UI
settingswidget.ui
)
kde4_add_ui_files( documentview_config_SRC ${documentview_config_UI} )

kde4_add_kcfg_files(documentview_config_SRC config.kcfgc )
kde4_add_plugin(kcm_documentview_settings ${documentview_config_SRC})

target_link_libraries(kcm_documentview_settings ${KDE4_KCMUTILS_LIBS} kdevplatform )
install(TARGETS kcm_documentview_settings  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kcm_documentview_settings.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES data.kdev4 DESTINATION ${DATA_INSTALL_DIR}/kdevdocumentview )
