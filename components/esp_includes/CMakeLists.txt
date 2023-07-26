set(components 
        "esp_common" 
        "log" 
        "esp_timer" 
        "driver" 
        "fatfs" 
        "spiffs" 
        "spi_flash" 
        "esp_hw_support" 
        "esp_system")
idf_component_register(SRCS "" 
                        INCLUDE_DIRS "include" 
                        REQUIRES ${components})
