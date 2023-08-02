#ifndef __SPIFFS_FILE_H__
#define __SPIFFS_FILE_H__

#include <stdbool.h>
#include <stdio.h>

#include "esp_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief The maximum number of lines that can be stored in a file
#define SPIFFS_MAXIMUM_LINE_COUNT 40
// @brief The maximum length of any single line in a file
#define SPIFFS_CONFIG_MAX_LINE_LENGTH 64

/**
 * @brief structure representing a single line from the SPIFFS file
 * @param content (char[SPIFFS_CONFIG_MAX_LINE_LENGTH]) the content of a single line in a file
 */
typedef struct {
    // @brief the content of a single line in a file
    char content[SPIFFS_CONFIG_MAX_LINE_LENGTH];
} spiffs_file_line_t;

/**
 * @brief structure representing a single SPIFFS file. Memebers are provided for manipulating said structure
 * @param File      (FILE *)               the SPIFFS file
 * @param FileName  (const char *)         the name of the file
 * @param Content   (spiffs_file_line_t *) the contents of the file parsed and stored in an array of spiffs_file_line_t structures
 * @param LineCount (int)                  the total number of lines in this file
 */
typedef struct {
    // @brief the SPIFFS file
    FILE * File;
    // @brief the name of the file
    const char * FileName;
    // @brief the contents of the file parsed and stored in an array of spiffs_file_line_t structures
    spiffs_file_line_t * Content;
    // @brief the total number of lines in this file
    int LineCount;
    
    /**
     * @brief close the file
     */
    void (*close)(void);
    /**
     * @brief deletes a file if it exists
     * @param the name of the ile to delete
     */
    void (*delete_if_exists)(const char * fileName);
    // @brief load the contents of the file, and store each line in a spiffs_file_line_t structure
    int (*load_content)(void);
    /**
     * @brief opens the file in read mode
     * @param the name of the file to open for reading
     */
    bool (*open_to_read)(const char * fileName);
    /**
     * @brief opens a file in write mode
     * @param the name of the file to open in write mode
     */
    bool (*open_to_write)(const char * fileName);
    /**
     * @brief reads a single line from the file currently opened in read mode
     * @param buffer (char *) buffer to read the line into
     * @param size   (int)    the length of the line red in
     */
    bool (*read_line)(char * buffer, int size);
    /**
     * @brief renames a file
     * @param to_fileName (const char *) the new name for the file
     */
    bool (*rename_to)(const char * to_fileName);
    /**
     * @brief changes the name of the file to use in subsequent operations
     * @param fileName (const char *) the name of the file to use
     */
    void (*set_file_name)(const char * fileName);
    /**
     * @brief writes content to the file currently opened in write mode
     * @param the content to write into the file
     */
    void (*write_text)(const char * text);
} spiffs_file_t;

/**
 * @brief performs a check to locate the identified SPIFFS partition
 * @param label (const char *) the name of the partition to locate
 */
void spiffs_check_for_spiffs_partition(const char * label);
/**
 * @brief call this member to initialize the SPIFFS Virtual File System
 * @param base_path       (const char *) the base path for this partition (eg. .base_path = "fs/")
 * @param partition_label (const char *) the name of the partition where the SPIFFS VFS will be mounted from
 */
bool spiffs_configure_and_initialize_vfs(const char * base_path, const char * partition_label);
/**
 * @brief if the SPIFFS partition is missing, then the partition will be formatted
 * @param label (const char *) the name of the partition to format (if needed)
 */
void spiffs_format_if_spiffs_partition_is_missing(const char * label);
/**
 * @brief unmounts the SPIFFS Virtual File System
 * @param partition_label (const char *) the name of the mounted partition to unmount
 */
void spiffs_unmount_vfs(const char * partition_label);

extern spiffs_file_t Spiffs_File;

#ifdef __cplusplus
}
#endif

#endif // __SPIFFS_FILE_H__