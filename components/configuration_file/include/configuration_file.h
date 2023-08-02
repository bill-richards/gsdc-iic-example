#ifndef __CONFIGURATION_FILE_H__
#define __CONFIGURATION_FILE_H__

/**
 * 
 *  A configuration file is a series of key=value pairs.
 *      A key can contain spaces, its maximum size is 20 characters
 *      A value can contain spaces, its maximum size is 43 characters
 * 
 *  example:
 * 
 *  KEY_ONE=key one's value
 *  KEY TWO=key two's value
 *  ___________________________
 * |   key   |      value      |
 * |---------|-----------------|
 * | KEY_ONE | key one's value |
 * | KEY TWO | key two's value |
 *  ---------------------------
 * 
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIGURATION_FILE_MAXIMUM_FILE_NAME_LENGTH 16
#define CONFIGURATION_FILE_MAXIMUM_BASE_PATH_LENGTH 16
#define CONFIGURATION_FILE_MAXIMUM_PARTITION_LABEL_LENGTH 16
#define CONFIGURATION_FILE_MAXIMUM_FILE_PATH_LENGTH (CONFIGURATION_FILE_MAXIMUM_BASE_PATH_LENGTH + CONFIGURATION_FILE_MAXIMUM_FILE_NAME_LENGTH + 1)
#define CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH 20
#define CONFIGURATION_FILE_MAXIMUM_VALUE_LENGTH 43

typedef struct {
    char Key[CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH];                   // MAXIMUM_KEY_LENGTH
    char Value[CONFIGURATION_FILE_MAXIMUM_VALUE_LENGTH];               // MAXIMUM_VALUE_LENGTH
} config_key_value_pair_t;

/**
 * @brief Represents a configuration file stored in a SPIFFS file system
 */
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
} gsdc_configuration_file_t;

/**
 * @brief Describes the configuraiton file in terms of SPIFFS partition, base path, and the file name.
 */
typedef struct {
    // @brief The name of the partition within which the file can be found
    const char * partition_label;
    // @brief The base-path to use for addressing the named partition
    const char * base_path;
    // @brief The name of the configuration file to use
    const char * file_name;
} gsdc_configuration_file_descriptor_t;

/**
 * @brief Instantiate a gsdc_configuration_file_t
 * @param fileDescriptor a pointer to an instance of a gsdc_configuration_file_descriptor_t which describes the configuration file
 */
void register_configuration_file(gsdc_configuration_file_descriptor_t * fileDescriptor);

// @brief The instantiated gsdc_configuration_file_t
extern gsdc_configuration_file_t Configuration_File;

#ifdef __cplusplus
}
#endif

#endif // __CONFIGURATION_FILE_H__

