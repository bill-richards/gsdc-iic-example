#ifndef __MAIN_H__
#define __MAIN_H__

/**
 *  These values are taken from sdkconfig
 */
#ifdef CONFIG_GSDC_SPIFFS_CONFIG_FILE_SYSTEM_ROOT
static const char * base_path = CONFIG_GSDC_SPIFFS_CONFIG_FILE_SYSTEM_ROOT;
#else
static const char * base_path = "/spiffs";
#endif

#ifdef CONFIG_GSDC_SPIFFS_PARTITION_LABEL
static const char * partition_label = CONFIG_GSDC_SPIFFS_PARTITION_LABEL;
#else 
static const char * partition_label = "data";
#endif

#if CONFIG_GSDC_IIC_OVERRIDE_CONFIGURATION_FILE_NAME
static const char * iic_configuration_file_name = CONFIG_GSDC_IIC_CONFIGURATION_FILE_NAME;
#else
static const char * iic_configuration_file_name = "configuration";
#endif


#endif // __MAIN_H__