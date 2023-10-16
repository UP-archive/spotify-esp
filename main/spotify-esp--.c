#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <stdio.h>
#include <string.h>

// #define PLAY GPIO_NUM_15
// #define PREV GPIO_NUM_18
// #define NEXT GPIO_NUM_19

#define PLAY GPIO_NUM_4
#define PREV GPIO_NUM_18
#define NEXT GPIO_NUM_5

const char *ssid = CONFIG_ESP_WIFI_SSID;
const char *pass = CONFIG_ESP_WIFI_PASSWORD;
int retry_num = 0;

static void wifi_event_handler(void *event_handler_arg,
                               esp_event_base_t event_base, int32_t event_id,
                               void *event_data) {
  if (event_id == WIFI_EVENT_STA_START) {
    printf("WIFI CONNECTING....\n");
  } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
    printf("WiFi CONNECTED\n");
  } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    printf("WiFi lost connection\n");
    if (retry_num < 5) {
      esp_wifi_connect();
      retry_num++;
      printf("Retrying to Connect...\n");
    }
  } else if (event_id == IP_EVENT_STA_GOT_IP) {
    printf("Wifi got IP...\n\n");
  }
}

void wifi_connection() {
  //                          s1.4
  // 2 - Wi-Fi Configuration Phase
  esp_netif_init();
  esp_event_loop_create_default();     // event loop                    s1.2
  esp_netif_create_default_wifi_sta(); // WiFi station                      s1.3
  wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_initiation); //
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler,
                             NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler,
                             NULL);
  wifi_config_t wifi_configuration = {.sta =
                                          {
                                              .ssid = "",
                                              .password = "",

                                          }

  };
  strcpy((char *)wifi_configuration.sta.ssid, ssid);
  strcpy((char *)wifi_configuration.sta.password, pass);
  // esp_log_write(ESP_LOG_INFO, "Kconfig", "SSID=%s, PASS=%s", ssid, pass);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
  // 3 - Wi-Fi Start Phase
  esp_wifi_start();
  esp_wifi_set_mode(WIFI_MODE_STA);
  // 4- Wi-Fi Connect Phase
  esp_wifi_connect();
  printf("wifi_init_softap finished. SSID:%s  password:%s", ssid, pass);
}

void app_main(void) {
   uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };

  nvs_flash_init();
  wifi_connection();

  // Set Button Input
  gpio_set_direction(PLAY, GPIO_MODE_INPUT);
  gpio_set_direction(PREV, GPIO_MODE_INPUT);
  gpio_set_direction(NEXT, GPIO_MODE_INPUT);

  // Set button press works 
  for(;;)
  {
    // if (gpio_get_level(PLAY) == 0) {
		// 	printf("Play Button Pressed\n");

    //   esp_err_t err;
		// 	esp_http_client_config_t config = {
		// 			.url = "http://192.168.43.182/api/controller/play",
		// 	};
		// 	esp_http_client_handle_t client = esp_http_client_init(&config);

		// 	// Send Request
		// 	err = esp_http_client_perform(client);
		// 	esp_http_client_cleanup(client);
    // } else {
    //   printf("not press\n");
    // }

    // if (gpio_get_level(NEXT) == 0) {
		// 	printf("Next Button Pressed\n");
      
		// 	esp_err_t err;
		// 	esp_http_client_config_t config = {
		// 			.url = "http://192.168.43.182/api/controller/next",
		// 	};
		// 	esp_http_client_handle_t client = esp_http_client_init(&config);

		// 	// Send Request
		// 	err = esp_http_client_perform(client);
		// 	esp_http_client_cleanup(client);
    // } else {
    //   printf("not press\n");
    // }
    
    if (gpio_get_level(PREV) == 0) {
			printf("Previous Button Pressed\n");
      
			esp_err_t err;
			esp_http_client_config_t config = {
					.url = "http://192.168.43.182/api/controller/prev",
			};
			esp_http_client_handle_t client = esp_http_client_init(&config);

			// Send Request
			err = esp_http_client_perform(client);
			esp_http_client_cleanup(client);
    } else {
      printf("not press\n");
    }

    vTaskDelay(1000);
  }
  
}
