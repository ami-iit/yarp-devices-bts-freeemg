
# Install FreeEmg device configuration files
file (GLOB FREE_EMG_CONF_XML_FILES *.xml)

install(FILES ${FREE_EMG_CONF_XML_FILES}
             DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/)
