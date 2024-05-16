#include <stdio.h>
#include <string.h>    //strlen
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "nvs.h"
#include "nvs_flash.h"

#define SSID "Nam"
#define PASSPHARSE "12345678"
#define MESSAGE "HelloTCPServer"
#define TCPServerIP "172.20.10.3"

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static const char *TAG="tcp_client";

void wifi_connect(){
    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHARSE,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

static esp_err_t event_handler(void *ctx, int32_t event)
{
    switch(event) {
    case WIFI_EVENT_STA_START:
        wifi_connect();
        break;
    case IP_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    esp_netif_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_event_loop_create_default()  ); // Khởi tạo vòng lặp sự kiện ở đây
}

void tcp_client(void *pvParam){
    ESP_LOGI(TAG,"tcp_client task started \n");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = inet_addr(TCPServerIP);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons( 8080 );
    int s, r;
    char recv_buf[1024];
    while(1){
        xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\n");
        if(connect(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... connected \n");

        // Gửi dữ liệu và nhận phản hồi từ server
        if (write(s , MESSAGE , strlen(MESSAGE)) < 0) {
            ESP_LOGE(TAG, "... Send failed \n");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            if (r > 0) {
                recv_buf[r] = 0; // Thêm null terminator để có thể in ra chuỗi
                ESP_LOGI(TAG, "Received: %s", recv_buf);
            }
        } while(r > 0);
        close(s);
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        ESP_LOGI(TAG, "... new request in 5 seconds");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main()
{	
    wifi_event_group = xEventGroupCreate();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    initialise_wifi();
    xTaskCreate(&tcp_client,"tcp_client",8092,NULL,5,NULL);
}