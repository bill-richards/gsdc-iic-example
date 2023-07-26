set(components 
        "driver" 
        "esp_includes"
        "freertos"
        "newlib")

idf_component_register(SRCS "led_controller.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ${components})
