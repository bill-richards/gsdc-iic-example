#ifndef __SPIFFS_FILE_H__
#define __SPIFFS_FILE_H__

#include <stdbool.h>
#include <stdio.h>

#include "esp_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPIFFS_MAXIMUM_LINE_COUNT 40
#define SPIFFS_CONFIG_MAX_LINE_LENGTH 64

typedef struct {
    char content[SPIFFS_CONFIG_MAX_LINE_LENGTH];
} spiffs_file_line_t;

typedef struct {
    FILE * File;
    const char * FileName;
    spiffs_file_line_t * Content;
    int LineCount;

    void (*close)();
    void (*delete_if_exists)(const char * fileName);
    int (*load_content)(void);
    bool (*open_to_read)(const char * fileName);
    bool (*open_to_write)(const char * fileName);
    bool (*read_line)(char * buffer, int size);
    bool (*rename_to)(const char * to_fileName);
    void (*set_file_name)(const char * fileName);    
    void (*write_text)(const char * text);
} spiffs_file_t;

void spiffs_check_for_spiffs_partition(const char* label);
bool spiffs_configure_and_initialize_vfs(const char * base_path, const char * partition_label);
void spiffs_format_if_spiffs_partition_is_missing(const char * label);
void spiffs_unmount_vfs(const char * partition_label);

extern spiffs_file_t Spiffs_File;

#ifdef __cplusplus
}
#endif

#endif // __SPIFFS_FILE_H__