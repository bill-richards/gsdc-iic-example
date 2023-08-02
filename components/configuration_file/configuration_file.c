#include <configuration_file.h>
#include <spiffs_file.h>
#include <esp_logging.h>
#include <string.h>

#include "freertos/FreeRTOS.h"

void configuration_file_clear_all_entries(void);
void configuration_file_read(void);
bool configuration_file_get_config_item(const char * name, config_key_value_pair_t * key_value_pair);
char * configuration_file_get_file_path(void);
void configuration_file_output_all_key_value_pairs(void);
bool configuration_file_save();
bool configuration_file_set_configuration_item(const char * name, const char * config_item_value);

static const char * CONFIGURATION_FILE_TAG = "configuration_file";

gsdc_configuration_file_t Configuration_File;
char file_path[CONFIGURATION_FILE_MAXIMUM_FILE_PATH_LENGTH];

void register_configuration_file(gsdc_configuration_file_descriptor_t * fileDescriptor)
{
    ESP_LOGV(CONFIGURATION_FILE_TAG, "\tregister_configuration_file(%s, %s, %s) entering...",  fileDescriptor->partition_label, fileDescriptor->base_path, fileDescriptor->file_name);
    ESP_LOGV(CONFIGURATION_FILE_TAG, "Initializing file system ...");

    spiffs_configure_and_initialize_vfs(fileDescriptor->base_path, fileDescriptor->partition_label);

    gsdc_configuration_file_t configuration_file = {
        .clear_all = configuration_file_clear_all_entries,
        .get_configuration_item = configuration_file_get_config_item,
        .get_file_path = configuration_file_get_file_path,
        .output_all_configuration_items = configuration_file_output_all_key_value_pairs,
        .read_content = configuration_file_read,
        .save_configuration = configuration_file_save,
        .set_configuration_item = configuration_file_set_configuration_item,
        .PartitionLabel = fileDescriptor->partition_label,
        .BasePath = fileDescriptor->base_path,
        .Name = fileDescriptor->file_name
    };

    Configuration_File = configuration_file;

    ESP_LOGV(CONFIGURATION_FILE_TAG, "\tregister_configuration_file() leaving...");
}

void configuration_file_clear_all_entries(void)
{
    Configuration_File.Configuration_Count = 0;
    free(Configuration_File.Configuration_Items);
    Configuration_File.Configuration_Items = calloc(SPIFFS_MAXIMUM_LINE_COUNT, sizeof(config_key_value_pair_t));
}

char * configuration_file_get_file_path(void)
{
    sprintf(file_path, "%s/%s", Configuration_File.BasePath, Configuration_File.Name);
    return &file_path[0];
}

void configuration_file_read(void)
{
    char file_path[CONFIGURATION_FILE_MAXIMUM_FILE_PATH_LENGTH];
    sprintf(file_path, "%s/%s", Configuration_File.BasePath, Configuration_File.Name);

    ESP_LOGV(CONFIGURATION_FILE_TAG, "\tReading file %s", file_path);
    if(!Spiffs_File.open_to_read(file_path)){
        ESP_LOGE(CONFIGURATION_FILE_TAG, "Failed to open %s for reading", file_path);
        return;
    }
    int line_count = Spiffs_File.load_content();
    
    ESP_LOGV(CONFIGURATION_FILE_TAG, "\tClosing the file %s", file_path);
    Spiffs_File.close();

    ESP_LOGV(CONFIGURATION_FILE_TAG, "\tParsing the configuration file...");
    char * saveptr;
    char * pos;
    char * inner_pos;

    Configuration_File.Configuration_Items = calloc(SPIFFS_MAXIMUM_LINE_COUNT, sizeof(config_key_value_pair_t));
    for(int line_index = 0, configuration_index = 0; line_index < line_count; )
    {
        ESP_LOGV(CONFIGURATION_FILE_TAG, 
                    "\t\tParsing the current line [%x of %x] [%s]", 
                    (line_index+1), line_count, 
                    Spiffs_File.Content[line_index].content);

        if(Spiffs_File.Content[line_index].content[0] == '#') { 
            ESP_LOGV(CONFIGURATION_FILE_TAG, 
                        "\t\t\tComment line found, ignoring content [%s]", 
                        Spiffs_File.Content[line_index].content);
            line_index++;
            continue; 
        }

        pos = strtok_r(Spiffs_File.Content[line_index++].content, "=", &saveptr);

        while ((inner_pos = strchr(saveptr, '\n')) != NULL || (inner_pos = strchr(saveptr, 0x0D)) != NULL ) {
            *inner_pos = '\0';
        }
        ESP_LOGV(CONFIGURATION_FILE_TAG, "\t\t\tSetting name to [%s] and value to [%s]", pos, saveptr);

        config_key_value_pair_t * key_value_pair = (config_key_value_pair_t *)calloc(1, sizeof(config_key_value_pair_t));
        memcpy(key_value_pair->Key, pos, CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH);
        memcpy(key_value_pair->Value, saveptr, CONFIGURATION_FILE_MAXIMUM_VALUE_LENGTH);

        ESP_LOGV(CONFIGURATION_FILE_TAG, "\t\tCaching the config item");
        memcpy(&Configuration_File.Configuration_Items[configuration_index++], key_value_pair, sizeof(config_key_value_pair_t));
        free(key_value_pair);
        Configuration_File.Configuration_Count = configuration_index;
    }
    ESP_LOGV(CONFIGURATION_FILE_TAG, "\t\tConfiguration item count : %x", Configuration_File.Configuration_Count);

    ESP_LOGV(CONFIGURATION_FILE_TAG, "[%-20s]\t[%-43s]", "Key", "Value");
    for(int i = 0; i < Configuration_File.Configuration_Count; i++)
    {
        ESP_LOGV(CONFIGURATION_FILE_TAG, "[%-20s]\t[%-43s]", 
                        Configuration_File.Configuration_Items[i].Key, 
                        Configuration_File.Configuration_Items[i].Value);
    }
}

bool configuration_file_get_config_item(const char * key, config_key_value_pair_t * key_value_pair)
{
    ESP_LOGV(CONFIGURATION_FILE_TAG, "configuration_file_get_config_item(%s) entering...", key);

    for(int index = 0; index < Configuration_File.Configuration_Count; index++)
    {
        if(strcmp(Configuration_File.Configuration_Items[index].Key, key) == 0)
        {
            memcpy(key_value_pair, &Configuration_File.Configuration_Items[index], sizeof(config_key_value_pair_t));
    
            ESP_LOGV(CONFIGURATION_FILE_TAG, "configuration_file_get_config_item(%s) returning true...", key);
            return true;
        }
    }

    ESP_LOGV(CONFIGURATION_FILE_TAG, "configuration_file_get_config_item(%s) returning false...", key);
    return false;
}

void configuration_file_output_all_key_value_pairs(void)
{
    ESP_LOGI(CONFIGURATION_FILE_TAG, "\t[%20s]\t[%43s]",  "key", "value");
    for(int index = 0; index < Configuration_File.Configuration_Count; index++)
    {
        config_key_value_pair_t* item = &Configuration_File.Configuration_Items[index];
        ESP_LOGI(CONFIGURATION_FILE_TAG, "\t[%20s]\t[%43s]",  item->Key, item->Value);        
    }
}

bool configuration_file_save()
{
    char file_path[CONFIGURATION_FILE_MAXIMUM_FILE_PATH_LENGTH];
    sprintf(file_path, "%s/%s", Configuration_File.BasePath, Configuration_File.Name);
    
    ESP_LOGI(CONFIGURATION_FILE_TAG, "Saving configuration to [%s]", file_path);
    Spiffs_File.delete_if_exists(file_path);
    if(!Spiffs_File.open_to_write(file_path)){
        ESP_LOGE(CONFIGURATION_FILE_TAG, "Failed to open %s for writing", file_path);
        return false;
    }

    char line[SPIFFS_CONFIG_MAX_LINE_LENGTH];
    for(int index = 0; index < Configuration_File.Configuration_Count; index++)
    {
        sprintf(line, "%s=%s\n", Configuration_File.Configuration_Items[index].Key, Configuration_File.Configuration_Items[index].Value);
        Spiffs_File.write_text(line);
    }

    Spiffs_File.close();
    return true;
}

bool configuration_file_set_configuration_item(const char * name, const char * config_item_value)
{
    ESP_LOGI(CONFIGURATION_FILE_TAG, "spiffs_set_config_item(%s, %s) entering...", name, config_item_value);

    for(int index = 0; index < Configuration_File.Configuration_Count; index++)
    {
        if(strcmp(Configuration_File.Configuration_Items[index].Key, name) == 0)
        {
            ESP_LOGV(CONFIGURATION_FILE_TAG, "Configuration item [%s] exists. Modifying it's value...", name);
            memcpy(&Configuration_File.Configuration_Items[index].Value, config_item_value, CONFIGURATION_FILE_MAXIMUM_VALUE_LENGTH);    
            
            ESP_LOGV(CONFIGURATION_FILE_TAG, "spiffs_set_config_item() returning true...");
            return true;
        }
    }

    ESP_LOGV(CONFIGURATION_FILE_TAG, "Configuration item [%s] does not exist. Creating...", name);

    config_key_value_pair_t * new_item = (config_key_value_pair_t *)calloc(1, sizeof(config_key_value_pair_t));

    memcpy(new_item->Key, name, CONFIGURATION_FILE_MAXIMUM_KEY_LENGTH);
    memcpy(new_item->Value, config_item_value, CONFIGURATION_FILE_MAXIMUM_VALUE_LENGTH);
    memcpy(&Configuration_File.Configuration_Items[Configuration_File.Configuration_Count++], new_item, sizeof(config_key_value_pair_t));
    free(new_item);

    ESP_LOGV(CONFIGURATION_FILE_TAG, "Configuration item Created [%s][%s]", 
        Configuration_File.Configuration_Items[Configuration_File.Configuration_Count-1].Key, 
        Configuration_File.Configuration_Items[Configuration_File.Configuration_Count-1].Value);

    ESP_LOGI(CONFIGURATION_FILE_TAG, "spiffs_set_config_item() returning true...");
    return true;
}
