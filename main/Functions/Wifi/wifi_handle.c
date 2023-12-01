#include "sdkconfig.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "wifi_info.h"


//static EventGroupHandle_t wifi_event_group;
//static int ssid_index = 0;
static const char *TAG = "wifi station";
static int s_retry_num = 0;

const int IPV4_GOTIP_BIT = BIT0;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

//static void ssid_change();
//
//static esp_err_t event_handler(void *ctx, system_event_t *event) {
//    /* For accessing reason codes in case of disconnection */
//    system_event_info_t *info = &event->event_info;
//
//    switch (event->event_id) {
//    case WIFI_EVENT_STA_CONNECTED:
//        esp_wifi_connect();
//        break;
//    case SYSTEM_EVENT_STA_CONNECTED:
//#ifdef CONFIG_EXAMPLE_IPV6
//        /* enable ipv6 */
//        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
//#endif
//        break;
//    case SYSTEM_EVENT_STA_GOT_IP:
//        GPIO_SET_LEVEL_HIGH(PIN_LED_WIFI_STATUS);
//
//        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
//        os_printf("SYSTEM EVENT STA GOT IP : %s\r\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
//        break;
//    case SYSTEM_EVENT_STA_DISCONNECTED:
//        GPIO_SET_LEVEL_LOW(PIN_LED_WIFI_STATUS);
//
//        os_printf("Disconnect reason : %d\r\n", (int)info->disconnected.reason);
//
//#ifdef CONFIG_IDF_TARGET_ESP8266
//        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
//            /*Switch to 802.11 bgn mode */
//            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
//        }
//#endif
//        ssid_change();
//        esp_wifi_connect();
//        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
//#ifdef CONFIG_EXAMPLE_IPV6
//        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
//#endif
//
//#if (USE_UART_BRIDGE == 1)
//        uart_bridge_close();
//#endif
//        break;
//    case SYSTEM_EVENT_AP_STA_GOT_IP6:
//#ifdef CONFIG_EXAMPLE_IPV6
//        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
//        os_printf("SYSTEM_EVENT_STA_GOT_IP6\r\n");
//
//        char *ip6 = ip6addr_ntoa(&event->event_info.got_ip6.ip6_info.ip);
//        os_printf("IPv6: %s\r\n", ip6);
//#endif
//    default:
//        break;
//    }
//    return ESP_OK;
//}
//
//static void ssid_change() {
//    if (ssid_index > WIFI_LIST_SIZE - 1) {
//        ssid_index = 0;
//    }
//
//    wifi_config_t wifi_config = {
//        .sta = {
//            .ssid = "",
//            .password = "",
//        },
//    };
//
//    strcpy((char *)wifi_config.sta.ssid, wifi_list[ssid_index].ssid);
//    strcpy((char *)wifi_config.sta.password, wifi_list[ssid_index].password);
//    ssid_index++;
//    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
//}
//
//static void wait_for_ip() {
//#ifdef CONFIG_EXAMPLE_IPV6
//    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT;
//#else
//    uint32_t bits = IPV4_GOTIP_BIT;
//#endif
//
//    os_printf("Waiting for AP connection...\r\n");
//    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
//    os_printf("Connected to AP\r\n");
//}
//
//void wifi_init(void) {
//    GPIO_FUNCTION_SET(PIN_LED_WIFI_STATUS);
//    GPIO_SET_DIRECTION_NORMAL_OUT(PIN_LED_WIFI_STATUS);
//
//    esp_netif_init();
//    esp_netif_t* tmp=esp_netif_create_default_wifi_sta()
//#if (USE_STATIC_IP == 1)
//    esp_netif_dhcps_stop(tmp);
//
//    esp_netif_ip_info_t ip_info;
//
//#define MY_IP4_ADDR(...) IP4_ADDR(__VA_ARGS__)
//    MY_IP4_ADDR(&ip_info.ip, DAP_IP_ADDRESS);
//    MY_IP4_ADDR(&ip_info.gw, DAP_IP_GATEWAY);
//    MY_IP4_ADDR(&ip_info.netmask, DAP_IP_NETMASK);
//#undef MY_IP4_ADDR
//
//    esp_netif_set_ip_info(tmp, &ip_info);
//#endif // (USE_STATIC_IP == 1)
//
//    wifi_event_group = xEventGroupCreate();
//
//    ESP_ERROR_CHECK(esp_event_loop_create(event_handler, NULL));
//    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
//
//    // os_printf("Setting WiFi configuration SSID %s...\r\n", wifi_config.sta.ssid);
//    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
//    ssid_change();
//    ESP_ERROR_CHECK(esp_wifi_start());
//
//
//    wait_for_ip();
//}
static EventGroupHandle_t s_wifi_event_group;
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


void wifi_init(){
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}