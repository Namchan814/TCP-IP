// TCP SOCKET Client with WiFi connection communication via Socket

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_system.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "nvs_flash.h"
#include "ping/ping_sock.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "mywifi.h"
// #define MOTOR_PIN GPIO_NUM_22
// #define MOTOR_PIN_2 GPIO_NUM_3
// #define MOTOR_PIN_3 GPIO_NUM_5
#define PORT 3333
// #define PWM_CHANNEL LEDC_CHANNEL_0
// #define PWM_FREQUENCY 1000 // Frequency
// #define PWM_RESOLUTION LEDC_TIMER_10_BIT // Resolution
#define LEDC_OUTPUT_IO    (22) // GPIO để xuất tín hiệu PWM
#define LEDC_CHANNEL      LEDC_CHANNEL_0
#define LEDC_TIMER        LEDC_TIMER_0
#define LEDC_MODE         LEDC_HIGH_SPEED_MODE
#define LEDC_DUTY_RES     LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY    (5000) // 5 kHz
static const char *TAG = "TCP SOCKET Client";
static const char *payload = "Message from ESP32 TCP Socket Client";
// void motor_vibrate(uint32_t duty_cycle){
//     ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, duty_cycle);
//     ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
// }
// void motor_task(void *pvParameter){
//     uint32_t duty_cycle = 0;
//     bool increasing = true;
//     while(1){
//         motor_vibrate(duty_cycle);
//         // set speed
//         if (increasing) {
//             duty_cycle += 50;
//             if (duty_cycle >= 1023) {
//                 increasing = false;
//             }
//         } else {
//             duty_cycle -= 50;
//             if (duty_cycle == 0) {
//                 increasing = true;
//             }
//         }

//         vTaskDelay(pdMS_TO_TICKS(100)); // Delay 100ms
//     }
// }
// void motor_init(){
//     // Initialize LEDC PWM
//     ledc_timer_config_t timer_conf;
//     timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
//     timer_conf.timer_num = LEDC_TIMER_0;
//     timer_conf.duty_resolution = PWM_RESOLUTION;
//     timer_conf.freq_hz = PWM_FREQUENCY;
//     ledc_timer_config(&timer_conf);

//     ledc_channel_config_t ledc_conf;
//     ledc_conf.gpio_num = MOTOR_PIN;
//     ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
//     ledc_conf.channel = PWM_CHANNEL;
//     ledc_conf.intr_type = LEDC_INTR_DISABLE;
//     ledc_conf.timer_sel = LEDC_TIMER_0;
//     ledc_conf.duty = 0;
//     ledc_channel_config(&ledc_conf);
// }
// config ledc pwm 
void configure_ledc_pwm() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}
void set_pwm_duty(uint32_t duty) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
void tcp_client(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = "172.20.10.3"; // Server IP
    int addr_family = 0;
    int ip_protocol = 0;

    while (1)
    {
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created ");
        ESP_LOGI(TAG, "Connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            continue;;
        }
        ESP_LOGI(TAG, "Successfully connected");
        
        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGI(TAG, "Connection closed");
                break;
            } else {
                rx_buffer[len] = 0; // Null-terminate received data
                ESP_LOGI(TAG, "Received %d bytes: '%s'", len, rx_buffer);

                // Chuyển đổi dữ liệu nhận được thành giá trị duty cycle
                int duty = atoi(rx_buffer);
                if (duty >= 0 && duty <= 8191) {
                    set_pwm_duty(duty);
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}
    //     err = send(sock, "2", strlen("2"), 0);
    //     if (err < 0) {
    //         ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    //     } else {
    //         ESP_LOGI(TAG, "Request sent successfully");
    //         gpio_set_level(MOTOR_PIN, 1);
    //          vTaskDelay(1000 / portTICK_PERIOD_MS); // Chờ 1 giây
    //         // ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, (1 << PWM_RESOLUTION) / 2);
    //         // ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
    //         // vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     }
    //     vTaskDelay(5000 / portTICK_PERIOD_MS); // Chờ 5 giây
    //     err = send(sock, "stop", strlen("stop"), 0);
    //     if (err < 0) {
    //         ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    //     } else {
    //         ESP_LOGI(TAG, "Request sent successfully");
    //          gpio_set_level(MOTOR_PIN, 0);
    //         vTaskDelay(1000 / portTICK_PERIOD_MS); // Chờ 1 giây
    //         // ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, 0);
    //         // ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
    //         // vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     }
    //     vTaskDelay(5000 / portTICK_PERIOD_MS); // Chờ 5 giây
    //     send(sock, payload, strlen(payload), 0);

    //     int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    //     rx_buffer[len] = 0; // Null-terminate
    //     ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
    //     ESP_LOGI(TAG, "%s", rx_buffer);
    //     vTaskDelay(5000 / portTICK_PERIOD_MS);

    //     if (sock != -1)
    //     {
    //         shutdown(sock, 0);
    //         close(sock);
    //     }
    // }
    // case 2


static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting WIFI_EVENT_STA_START ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected WIFI_EVENT_STA_CONNECTED ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection WIFI_EVENT_STA_DISCONNECTED ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_connect();
}

void app_main(void)
{
    // gpio_config_t io_conf;
    // // Khởi tạo cấu hình GPIO
    // io_conf.intr_type = GPIO_INTR_DISABLE; // Vô hiệu hóa ngắt
    // io_conf.mode = GPIO_MODE_OUTPUT;           // Chế độ output
    // io_conf.pin_bit_mask = (1ULL << MOTOR_PIN); // Sử dụng chân GPIO đã được định nghĩa
    // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Không kích hoạt pull-down resistor
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     // Không kích hoạt pull-up resistor

    // // Cấu hình GPIO
    // gpio_config(&io_conf); 
    configure_ledc_pwm();
    wifi_connection();
    //motor_init();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    // tcp_client();
   
    xTaskCreate(tcp_client,"TCP SOCKET Client",8092,NULL,5,NULL);
    //xTaskCreate(motor_task, "motor_task", 2048, NULL, 5, NULL); // Create motor task
 }

