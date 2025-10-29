idf_component_register(SRCS "lcd.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES
                        driver
                        esp_driver_spi
                        esp_driver_gpio
                        freertos
                        heap
                        log
                    REQUIRES config)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
