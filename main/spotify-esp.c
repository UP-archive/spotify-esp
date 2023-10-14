#include "esp_event.h" //for wifi event
#include "esp_http_client.h"
#include "esp_log.h"           //for showing logs
#include "esp_system.h"        //esp_init funtions esp_err_t
#include "esp_wifi.h"          //esp_wifi_init functions and wifi operations
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "lwip/err.h"          //light weight ip packets error handling
#include "lwip/sys.h"          //system applications for light weight ip apps
#include "nvs_flash.h"         //non volatile storage
#include <stdio.h>             //for basic printf commands
#include <string.h>            //for handling strings

const char *ssid = "dethz";
const char *pass = "orewadetzz";
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

void controller(char state) {
  if (state == 'p') {
    esp_err_t err;
    esp_http_client_config_t config = {
        .url = "http://tunnel.katzu.wtf/api/controller/play",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // first request
    err = esp_http_client_perform(client);

    esp_http_client_cleanup(client);
  } else if (state = 'n') {
    esp_err_t err;
    esp_http_client_config_t config = {
        .url = "http://tunnel.katzu.wtf/api/controller/next",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // first request
    err = esp_http_client_perform(client);

    esp_http_client_cleanup(client);
  } else if (state = 'o') {
    esp_err_t err;
    esp_http_client_config_t config = {
        .url = "http://tunnel.katzu.wtf/api/controller/prev",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // first request
    err = esp_http_client_perform(client);

    esp_http_client_cleanup(client);
  }
}

void app_main(void) {
  nvs_flash_init();
  wifi_connection();
}
