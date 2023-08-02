#include "gsdc_iic_configuration.h"

#include "esp_logging.h"
#include <string.h>

static const char * IIC_CONFIGURATION_TAG = "iic_configuration";
static const char * clients_key =  "CLIENTS";
static const char * local_key =  "IIC";

gsdc_iic_connected_device_t * iic_configuration_get_connected_device(int index);
void iic_configuration_load_iic_configuration(void);
bool iic_configuration_read_configuration_item(const char * key, config_key_value_pair_t * config_line);
config_key_value_pair_t * create_new_config_line(void);

void parse_clients(int configuration_item_index, uint8_t iic_master);

gsdc_iic_configuration_t IIC_Configuration;

gsdc_iic_configuration_t * gsdc_iic_configuration_init(gsdc_iic_configuration_file_data_t * spiffs_info)
{
    ESP_LOGI(IIC_CONFIGURATION_TAG, "Initializing IIC configuration ...");
    gsdc_iic_configuration_t configuration = {
        .SpiffsInfo = spiffs_info,
        .get_connected_device = iic_configuration_get_connected_device,
        .read_configuration_item = iic_configuration_read_configuration_item,
        .load = iic_configuration_load_iic_configuration,
    };

    IIC_Configuration = configuration;
    return &IIC_Configuration;
}

gsdc_iic_connected_device_t * iic_configuration_get_connected_device(int index) {
    return &IIC_Configuration.ConnectedDevices[index];
}

void iic_configuration_load_iic_configuration()
{
    ESP_LOGV(IIC_CONFIGURATION_TAG, "Preparing the gsdc_configuration_file_t ...");
    gsdc_configuration_file_descriptor_t descriptor = {
        IIC_Configuration.SpiffsInfo->partition_label,
        IIC_Configuration.SpiffsInfo->base_path,
        IIC_Configuration.SpiffsInfo->file_name
    };
    register_configuration_file(&descriptor);

    ESP_LOGV(IIC_CONFIGURATION_TAG, "Reading the configuration file ...");
    Configuration_File.read_content();

    ESP_LOGV(IIC_CONFIGURATION_TAG, "Parsing the IIC addresses ...");
    config_key_value_pair_t * config_line = create_new_config_line();

    ESP_LOGI(IIC_CONFIGURATION_TAG, "\tReading the local IIC address ...");
    if(!Configuration_File.get_configuration_item(local_key, config_line))
    {
        ESP_LOGE(IIC_CONFIGURATION_TAG, "\tFailed to read the local IIC address ...");
        return;
    }

    ESP_LOGI(IIC_CONFIGURATION_TAG, "\t\t%x", (uint8_t)strtol(config_line->Value, (char**)NULL, 16));
    IIC_Configuration.I2CAddress = strtol(config_line->Value, (char**)NULL, 16);
    
    ESP_LOGV(IIC_CONFIGURATION_TAG, "\tParsing the remainder of the configuration file ...");
    for(int index = 0; index < Configuration_File.Configuration_Count; index++)
    {
        if(strcmp((char *)&Configuration_File.Configuration_Items[index].Key, clients_key) == 0) { 

            ESP_LOGI(IIC_CONFIGURATION_TAG, "\tParsing the connected IIC addresses ...");
            parse_clients(index, IIC_Configuration.I2CAddress);
            continue;
        }
        
        if(strcmp((char *)&Configuration_File.Configuration_Items[index].Key, local_key) == 0) {
            continue;
        }

        ESP_LOGV(IIC_CONFIGURATION_TAG, "\t[%s] setting read from config file", Configuration_File.Configuration_Items[index].Key);
    }
    free(config_line);
}

bool iic_configuration_read_configuration_item(const char * key, config_key_value_pair_t * config_line)
{
    if(!Configuration_File.get_configuration_item(key, config_line)) {
        ESP_LOGE(IIC_CONFIGURATION_TAG, "Failed to retrieve config item [%-10s]", key);
        return false;
    }

    ESP_LOGV(IIC_CONFIGURATION_TAG, "Retrieved config item [%s]. It's value is [%s]", config_line->Key, config_line->Value);
    return true;
}


config_key_value_pair_t * create_new_config_line()
{
    return (config_key_value_pair_t *)calloc(1, sizeof(config_key_value_pair_t));
}

void parse_clients(int configuration_item_index, uint8_t iic_master)
{
    char * saveptr = (char *)&Configuration_File.Configuration_Items[configuration_item_index].Value;
    char * current_address;
    int index = 0;

    free(IIC_Configuration.ConnectedDevices);
    gsdc_iic_connected_device_t * clients = (gsdc_iic_connected_device_t *)calloc(IIC_CONFIGURATION_MAXIMUM_CONNECTED_DEVICES, sizeof(gsdc_iic_connected_device_t));

    while((current_address = strtok_r(saveptr, " ", &saveptr)))
    {
        gsdc_iic_connected_device_t client = {
            .I2CAddress = strtol(current_address, (char**)NULL, 16),
            .MasterI2CAddress = iic_master,
            .DataLength = 0,
            //.ReceivedData = (uint8_t *)calloc(PACKET_LENGTH, sizeof(uint8_t)),
        };

        memcpy(&clients[index++], &client, sizeof(gsdc_iic_connected_device_t));
        ESP_LOGV(IIC_CONFIGURATION_TAG, "\t\tConnected address : %x", client.I2CAddress);
    }
    IIC_Configuration.ConnectedDeviceCount = index;
    IIC_Configuration.ConnectedDevices = clients;

    ESP_LOGI(IIC_CONFIGURATION_TAG, "\tNumber of connected IIC devices %x", IIC_Configuration.ConnectedDeviceCount);
}
