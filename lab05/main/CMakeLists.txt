set(SOURCE main.c game.c board.c graphics.c nav.c)
set(MILESTONE 2)
list(APPEND SOURCE com.c)
message(STATUS "MILESTONE=${MILESTONE}")
idf_component_register(SRCS ${SOURCE}
                       INCLUDE_DIRS .
                       PRIV_REQUIRES esp_timer driver net config lcd pin joy)
target_compile_options(${COMPONENT_LIB} PRIVATE -DMILESTONE=${MILESTONE})
