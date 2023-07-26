FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.c)
#SET( app_sources
#    "${CMAKE_SOURCE_DIR}/main/main.c")

SET( required_components
    "freertos"
    "configuration_file" 
    "esp_includes" 
    "iic"
    "led_controller"
    "driver")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "include"
    REQUIRES ${required_components}
)

if(CONFIG_GSDC_SPIFFS_INCLUDE_PARTITION_IMAGE)
    spiffs_create_partition_image(configuration ../partition_table/file_sysytem FLASH_IN_PROJECT)
endif()