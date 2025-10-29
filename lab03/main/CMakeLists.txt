idf_component_register(SRCS main.c watch.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES driver esp_timer esp_driver_gptimer config lcd pin)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
