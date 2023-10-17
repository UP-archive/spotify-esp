#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "nvs_flash.h"
#include "ssd1306.h"

#define PLAY CONFIG_GPIO_1
#define PREV CONFIG_GPIO_0
#define NEXT CONFIG_GPIO_2

#define I2C_MASTER_SCL_IO CONFIG_S_CLK        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_S_DAT        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

static ssd1306_handle_t ssd1306_dev = NULL;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
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
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void rest_post_next()
{
    esp_http_client_config_t config_post = {
        .url = "http://tunnel.katzu.wtf/api/controller/next",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "next";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

static void rest_post_prev()
{
    esp_http_client_config_t config_post = {
        .url = "http://tunnel.katzu.wtf/api/controller/prev",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "prev";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

static void rest_post_play()
{
    esp_http_client_config_t config_post = {
        .url = "http://tunnel.katzu.wtf/api/controller/play",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .event_handler = client_event_post_handler};
        
    esp_http_client_handle_t client = esp_http_client_init(&config_post);

    char  *post_data = "play";
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

void app_main(void)
{
  nvs_flash_init();
  wifi_connection();

  vTaskDelay(2000 / portTICK_PERIOD_MS);
  printf("WIFI was initiated ...........\n\n");

  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

  i2c_param_config(I2C_MASTER_NUM, &conf);
  i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

  ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
  ssd1306_refresh_gram(ssd1306_dev);
  ssd1306_clear_screen(ssd1306_dev, 0x00);

  char data_str[10] = {0};
  sprintf(data_str, "READY");
  ssd1306_draw_string(ssd1306_dev, 70, 10, (const uint8_t *)data_str, 16, 1);
  ssd1306_refresh_gram(ssd1306_dev);


  // Set Button Input
  gpio_reset_pin(PLAY);
  gpio_reset_pin(PREV);
  gpio_reset_pin(NEXT);

  gpio_set_direction(PLAY, GPIO_MODE_INPUT);
  gpio_set_direction(PREV, GPIO_MODE_INPUT);
  gpio_set_direction(NEXT, GPIO_MODE_INPUT);

  // Button Check
  while (1) {
    if (gpio_get_level(PREV) == 0 && gpio_get_level(NEXT) == 1 && gpio_get_level(PLAY) == 1) {
      printf("%d\n", gpio_get_level(PREV));
      rest_post_prev();
      printf("\n");
    } else if (gpio_get_level(PREV) == 1 && gpio_get_level(NEXT) == 0 && gpio_get_level(PLAY) == 1) {
      printf("%d\n", gpio_get_level(NEXT));
      rest_post_next();
      printf("\n");
    } else if (gpio_get_level(PREV) == 1 && gpio_get_level(NEXT) == 1 && gpio_get_level(PLAY) == 0) {
      printf("%d\n", gpio_get_level(PLAY));
      rest_post_play();
      printf("\n");
    } else {
      printf(".");
    }

    vTaskDelay(1);
  }
}