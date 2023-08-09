FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.c)
#SET( app_sources
#    ${CMAKE_SOURCE_DIR}/main/main.c
#    ${CMAKE_SOURCE_DIR}/partition_table/file_system/iic.cfg
#)

SET(required_components
    freertos
    spi_flash
    configuration_file
    esp_includes
    gsdc_iic
    led_controller
    driver
    gsdc_ota
)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "include"
    REQUIRES ${required_components}
)

if(CONFIG_GSDC_SPIFFS_INCLUDE_PARTITION_IMAGE)
    spiffs_create_partition_image(config ../partition_table/file_sysytem FLASH_IN_PROJECT)
endif()