idf_component_register(SRCS pin.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES driver esp_driver_gpio)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
