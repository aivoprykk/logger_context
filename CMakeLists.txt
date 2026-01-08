set(SRCS context.c)
SET(INCS include)

idf_component_register(
   SRCS ${SRCS}
   INCLUDE_DIRS ${INCS}
   REQUIRES logger_common
   PRIV_REQUIRES nvs_flash
)

if(CONFIG_GPS_LOG_ENABLED)
   target_link_libraries(${COMPONENT_LIB} PUBLIC idf::gps_log)
endif()

if(CONFIG_UBLOX_ENABLED)
   target_link_libraries(${COMPONENT_LIB} PRIVATE idf::logger_ubx)
endif()
