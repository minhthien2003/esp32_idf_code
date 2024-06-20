/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "example";
const uint8_t mAdd[6] = {0xA1, 0xA2, 0xA3,0xA4,0xA5,0xA6};
typedef union
{
    uint8_t InternalData[100];
    struct
    {
        uint8_t mac[6];
        uint8_t data[6];
    }bytes;
    
}AppBackupData_t;
 
AppBackupData_t AppDataBackup;

void init()
{
    memcpy(&AppDataBackup.bytes.mac[0],mAdd, 6);
}

void app_main(void)
{
    init();
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }


    FILE *f = fopen("/spiffs/hello.txt", "w");
    fwrite(&AppDataBackup.InternalData[0], 1, sizeof(AppDataBackup), f);
    fclose(f);

    AppBackupData_t AppRead = {0,};
    f =fopen("/spiffs/hello.txt", "r");
    fread(&AppRead.InternalData[0], 1,sizeof(AppRead),f);
    fclose(f);
    printf("%2x %2x %2x %2x %2x %2x\n", AppRead.bytes.mac[0], AppRead.bytes.mac[1],
    AppRead.bytes.mac[2],AppRead.bytes.mac[3],AppRead.bytes.mac[4], AppRead.bytes.mac[5]);

    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}
