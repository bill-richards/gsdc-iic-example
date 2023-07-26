SET(components
        "spiffs"
        "esp_includes"
        "esp_common")


idf_component_register(SRCS "spiffs_file.c"
                        INCLUDE_DIRS "include"
                        REQUIRES ${components})
