#include "spiffs_file.h"
#include "esp_logging.h"

#include <sys/stat.h>
#include <sys/unistd.h>
#include <string.h>


void spiffs_file_close_file();
void spiffs_file_delete_if_exists(const char * fileName);
int spiffs_file_load_content(void);
bool spiffs_file_open_to_read(const char * fileName);
bool spiffs_file_open_to_write(const char * fileName);
bool spiffs_file_read_line(char * buffer, int size);
bool spiffs_file_rename_to(const char * to_fileName);
void spiffs_file_write_text(const char * text);
void spiffs_file_set_file_name(const char * fileName);
bool spiffs_file_open_file_in_mode(const char * mode);

static const char * SPIFFS_FILE_TAG = "spiffs_file";
spiffs_file_t Spiffs_File;

void spiffs_file_close_file()
{
    fclose(Spiffs_File.File);
}

void spiffs_file_delete_if_exists(const char * fileName)
{
    // Check if destination File exists before renaming
    struct stat st;
    if (stat(fileName, &st) == 0) {
        // Delete it if it exists
        unlink(fileName);
    }
}

int spiffs_file_load_content(void)
{
    ESP_LOGV(SPIFFS_FILE_TAG, "-> spiffs_file_load_content entering ...");
    int line_count = 0;
    char line[SPIFFS_CONFIG_MAX_LINE_LENGTH];
    Spiffs_File.Content = calloc(SPIFFS_MAXIMUM_LINE_COUNT, sizeof(spiffs_file_line_t));

    while(Spiffs_File.read_line(line, SPIFFS_CONFIG_MAX_LINE_LENGTH))
    {
        char * pos = strchr(line, '\n');
        while ((pos = strchr(line, '\n')) != NULL || (pos = strchr(line, 0x0D)) != NULL ) {
            *pos = '\0';
        }

        spiffs_file_line_t * current_line = (spiffs_file_line_t *)calloc(1, sizeof(spiffs_file_line_t));
        memcpy(current_line->content, line, strlen(line)+1);

        ESP_LOGV(SPIFFS_FILE_TAG, "Caching line %x : %s", line_count+1, current_line->content);
        memcpy(&Spiffs_File.Content[line_count++], current_line, sizeof(spiffs_file_line_t));
        free(current_line);

        ESP_LOGV(SPIFFS_FILE_TAG, "Line cached [%s]", Spiffs_File.Content[line_count-1].content);
    }
    Spiffs_File.LineCount = line_count;

    ESP_LOGV(SPIFFS_FILE_TAG, "<- spiffs_file_load_content leaving ...");
    return Spiffs_File.LineCount;
}

bool spiffs_file_open_file_in_mode(const char * mode)
{
    Spiffs_File.File = fopen(Spiffs_File.FileName, mode);
    if (Spiffs_File.File == NULL) {
        return false;
    }

    return true;
}

bool spiffs_file_open_to_read(const char * fileName)
{
    spiffs_file_set_file_name(fileName);
    return spiffs_file_open_file_in_mode("r");
}

bool spiffs_file_open_to_write(const char * fileName)
{
    spiffs_file_set_file_name(fileName);
    return spiffs_file_open_file_in_mode("w");
}

bool spiffs_file_read_line(char * buffer, int size)
{
    ESP_LOGV(SPIFFS_FILE_TAG, "Using a buffer of [%x] bytes", size);
    return(fgets(buffer, size, Spiffs_File.File) != NULL);
}

bool spiffs_file_rename_to(const char * to_fileName)
{
    if (rename(Spiffs_File.FileName, to_fileName) != 0) {
        return false;
    }
    return true;
}

void spiffs_file_set_file_name(const char * fileName)
{
    ESP_LOGV(SPIFFS_FILE_TAG, "Setting FileName to %s", fileName);
    Spiffs_File.FileName = fileName;
    ESP_LOGV(SPIFFS_FILE_TAG, "FileName set to %s", Spiffs_File.FileName);
}

void spiffs_file_write_text(const char * text)
{
    fprintf(Spiffs_File.File, text);
}


void spiffs_check_for_spiffs_partition(const char* label)
{
    ESP_LOGI(SPIFFS_FILE_TAG, "Performing SPIFFS_check().");
    esp_err_t ret = esp_spiffs_check(label);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_FILE_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return;
    } 

    ESP_LOGV(SPIFFS_FILE_TAG, "SPIFFS_check() successful");
}

bool spiffs_configure_and_initialize_vfs(const char * base_path, const char * partition_label)
{
    ESP_LOGI(SPIFFS_FILE_TAG, "Initializing SPIFFS for [%s]%s", partition_label, base_path);

    esp_vfs_spiffs_conf_t vfs_configuration = {
      .base_path = base_path,
      .partition_label = partition_label,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    ESP_LOGV(SPIFFS_FILE_TAG, "Mount or register filesystem ...");
    esp_err_t ret = esp_vfs_spiffs_register(&vfs_configuration);
    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SPIFFS_FILE_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(SPIFFS_FILE_TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(SPIFFS_FILE_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return false;
    }


#ifdef CONFIG_GSDC_SPIFFS_CHECK_ON_START
    ESP_LOGI(SPIFFS_FILE_TAG, "Checking for SPIFFS partition ...");
    spiffs_check_for_spiffs_partition(partition_label);
#endif

    spiffs_format_if_spiffs_partition_is_missing(partition_label);

    spiffs_file_t new_file = {
        .close = &spiffs_file_close_file,
        .delete_if_exists = &spiffs_file_delete_if_exists,
        .load_content = &spiffs_file_load_content,
        .open_to_read = &spiffs_file_open_to_read,
        .open_to_write = &spiffs_file_open_to_write,
        .read_line = &spiffs_file_read_line,
        .rename_to = &spiffs_file_rename_to,
        .set_file_name = &spiffs_file_set_file_name,
        .write_text = &spiffs_file_write_text,
    };
    Spiffs_File = new_file;
    Spiffs_File.Content = (spiffs_file_line_t*)calloc(SPIFFS_MAXIMUM_LINE_COUNT, SPIFFS_CONFIG_MAX_LINE_LENGTH);

    return true;
}

void spiffs_format_if_spiffs_partition_is_missing(const char * label)
{
    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info(label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_FILE_TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(label);
        return;
    } else {
        ESP_LOGI(SPIFFS_FILE_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(SPIFFS_FILE_TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        esp_err_t ret = esp_spiffs_check(label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(SPIFFS_FILE_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return;
        }

        ESP_LOGV(SPIFFS_FILE_TAG, "SPIFFS_check() successful");        
    }
}

void spiffs_unmount_vfs(const char * partition_label)
{
    ESP_LOGI(SPIFFS_FILE_TAG, "Unmounting SPIFFS");
    esp_vfs_spiffs_unregister(partition_label);
    ESP_LOGV(SPIFFS_FILE_TAG, "SPIFFS unmounted");
}
