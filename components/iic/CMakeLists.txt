
SET(app_sources
        "iic_configuration.c"
        "iic_common.c"
        "iic_transmission.c"
        "iic_receiver.c"
        "master.c")
    
set(components 
        "log" 
        "driver" 
        "configuration_file"
        "esp_includes"
        "led_controller"
        "freertos")

idf_component_register(SRCS ${app_sources}
                        INCLUDE_DIRS "include"
                        REQUIRES ${components})
