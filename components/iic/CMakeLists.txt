
SET(app_sources
        "iic_common.c"
        "iic_configuration.c"
        "iic_master.c"
        "iic_receiver.c"
        "iic_transmission.c"
)

set(components 
        "log" 
        "driver" 
        "configuration_file"
        "esp_includes"
        "led_controller"
        "freertos"
)

idf_component_register(SRCS ${app_sources}
                        INCLUDE_DIRS "include"
                        REQUIRES ${components})
