### logger_context ###
set(REQ gps_log logger_common)
set(PRIVREQ logger_config logger_ubx)
set(SRCS context.c)
SET(INCS include)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS ${INCS}
    REQUIRES ${REQ}
    PRIV_REQUIRES ${PRIVREQ}
)
