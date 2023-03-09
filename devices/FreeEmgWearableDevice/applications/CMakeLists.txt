
# FreeEmg xml application files installation
file (GLOB FREE_EMG_APP_XML_FILES *.xml)

install(FILES ${FREE_EMG_APP_XML_FILES}
             DESTINATION ${CMAKE_INSTALL_PREFIX}/share/yarp/applications/)
