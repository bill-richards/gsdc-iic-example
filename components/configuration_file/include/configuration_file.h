#ifndef __CONFIGURATION_FILE_H__
#define __CONFIGURATION_FILE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_FILE_NAME_LENGTH 16
#define MAXIMUM_BASE_PATH_LENGTH 16
#define MAXIMUM_PARTITION_LABEL_LENGTH 16
#define MAXIMUM_FILE_PATH_LENGTH (MAXIMUM_BASE_PATH_LENGTH + MAXIMUM_FILE_NAME_LENGTH + 1)
#define MAXIMUM_KEY_LENGTH 20
#define MAXIMUM_VALUE_LENGTH 43

typedef struct {
    char Key[MAXIMUM_KEY_LENGTH];                   // MAXIMUM_KEY_LENGTH
    char Value[MAXIMUM_VALUE_LENGTH];               // MAXIMUM_VALUE_LENGTH
} config_key_value_pair_t;

typedef struct {
    const char * Name;                              // MAXIMUM_FILE_NAME_LENGTH
    const char * BasePath;                          // MAXIMUM_BASE_PATH_LENGTH
    const char * PartitionLabel;                    // MAXIMUM_PARTITION_LABEL_LENGTH
    int Configuration_Count;
    config_key_value_pair_t * Configuration_Items;  // SPIFFS_MAXIMUM_LINE_COUNT

    void (*clear_all)(void);
    char* (*get_file_path)(void);
    bool (*get_configuration_item)(const char * key, config_key_value_pair_t * key_value_pair);
    void (*read_content)(void);
    bool (*save_configuration)(void);
    bool (*set_configuration_item)(const char * key, const char * value);
    void (*output_all_configuration_items)(void);
} configuration_file_t;

typedef struct {
    const char * partition_label;
    const char * base_path;
    const char * file_name;
} configuration_file_data_t;

void register_configuration_file(const char * partition_label, const char * base_path, const char * file_name);

extern configuration_file_t Configuration_File;

#ifdef __cplusplus
}
#endif

#endif // __CONFIGURATION_FILE_H__

