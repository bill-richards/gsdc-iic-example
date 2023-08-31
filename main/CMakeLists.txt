FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/main/*.cpp)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS include
)

if(CONFIG_GSDC_SPIFFS_INCLUDE_PARTITION_IMAGE)
    spiffs_create_partition_image(config ../partition_table/file_sysytem FLASH_IN_PROJECT)
endif()