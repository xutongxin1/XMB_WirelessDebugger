#include <sys/cdefs.h>
#include "sdkconfig.h"
#include <stdint.h>
#include <sys/param.h>
#include <lwip/sockets.h>


#include "UART/uart_val.h"
#include "TCPChannelController/TCPChannelController.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netbuf.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/priv/api_msg.h"


#ifdef CALLBACK_DEBUG
#define debug(s, ...) printf("%s: " s "\n", "Cb:", ##__VA_ARGS__)
#else
#define debug(s, ...)
#endif
#define MY_IP_ADDR(...) IP4_ADDR(__VA_ARGS__)
static const char *TCP_TAG = "TCP";

static err_t GetConnectState(struct netconn *conn);

TcpInfoTaskHandleT tcp_info_task_handle[4];//1921,1922,1923,1924


esp_err_t CreateTcpServer(TcpInfoTaskHandleT *parameter) {

    int addr_family = AF_INET;
    int ip_protocol;

    int on = 1;

    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *) &dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(parameter->port_);
    ip_protocol = IPPROTO_IP;

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(setsockopt(listen_sock, SOL_SOCKET, SO_KEEPALIVE, (void *) &on, sizeof(on)));
    ESP_ERROR_CHECK(setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (void *) &on, sizeof(on)));

    ESP_ERROR_CHECK(bind(listen_sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr)));//绑定socket

    ESP_LOGI(TCP_TAG, "Socket bound, port %d", parameter->port_);

    ESP_ERROR_CHECK(listen(listen_sock, 1));
    parameter->k_server_ = listen_sock;
    return ESP_OK;
}

/// TCP发送的任务
/// \param parameter 传参
_Noreturn void TcpServerSend(TcpInfoTaskHandleT *parameter) {
    ESP_LOGI(TCP_TAG, "port %d Enter TcpServerSend...",parameter->port_);

    QueueHandle_t *uart_to_tcp_queue = parameter->uart_tcp_queue_.uart_to_tcp_queue_;

    parameter->son_task_exists_ = true;
    TcpInfoEvent event;
    int ret = false;
    int send_bytes;
    while (1) {
        ESP_LOGI(TCP_TAG, "client %d TCP SENDING mode is working...",parameter->k_client_);

        ret = xQueueReceive(*uart_to_tcp_queue, &event, portMAX_DELAY);
        if (ret == pdTRUE) {

            do {
                send_bytes = send(parameter->k_client_, event.buff_arr_, event.buff_len_, 0);
            } while (send_bytes < 0);
        } else {
            ESP_LOGE(TCP_TAG, "uart to tcp queue sending error!!");
        }
    }
    ESP_LOGE(TCP_TAG, "TcpServerSend task is deleted!!!");
    vTaskDelete(NULL);
}

/// TCP任务创建
/// \param parameter
/// \param priority
/// \return
esp_err_t TcpTaskCreate(TcpInfoTaskHandleT *parameter, int priority) {
    char pc_name[18];
    ESP_LOGI(TCP_TAG, "\nParam->mode:%d", parameter->mode_);
    switch (parameter->mode_) {
        case TCP_ALL:
            sprintf(pc_name, "TcpRec%d", parameter->port_);
            if (xTaskCreatePinnedToCore((TaskFunction_t) TcpServerAcceptWithRec,
                                        (const char *const) pc_name,
                                        5120,
                                        parameter,
                                        priority,
                                        parameter->k_father_task_,
                                        0) == pdPASS) {
                parameter->father_task_exists_ = true;
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

/// 删除TCP NetIF 所有句柄和任务
/// \param tcp_task_handle_delete
/// \return 删除是否成功
esp_err_t TcpTaskAllDelete(TcpInfoTaskHandleT *tcp_task_handle_delete) {

    if ((tcp_task_handle_delete->father_task_exists_) == true) {
        if (tcp_task_handle_delete->son_task_exists_ == true) {//判断子任务是否存在
            vTaskDelete(*(tcp_task_handle_delete->k_son_task_));
            tcp_task_handle_delete->son_task_exists_ = false;
            tcp_task_handle_delete->k_son_task_ = NULL;
        }

        close(tcp_task_handle_delete->k_client_);
        close(tcp_task_handle_delete->k_server_);
        tcp_task_handle_delete->k_client_ = 0;
        tcp_task_handle_delete->k_server_ = 0;
        vTaskDelete(*(tcp_task_handle_delete->k_father_task_));
        tcp_task_handle_delete->k_father_task_ = NULL;
        tcp_task_handle_delete->father_task_exists_ = false;

        tcp_task_handle_delete->mode_ = 0;


    }

    return ESP_OK;
}

/// TCP接受连接与接收的任务
/// \param parameter 传参
void TcpServerAcceptWithRec(TcpInfoTaskHandleT *parameter) {
    esp_err_t err = ESP_OK;

    ESP_LOGI(TCP_TAG, "entering TcpServerAcceptWithRec with port %d", parameter->port_);
    ESP_ERROR_CHECK(CreateTcpServer(parameter));//创建TCP Server netCONN协议
    while (1) {
        ESP_ERROR_CHECK(TCPBSDAccept(parameter));
        /* Process the new connection. */

        if (!parameter->son_task_exists_) {
            /*Create Receive Subtask*/
            char tmp[16];
            sprintf(tmp, "TcpSend%d", parameter->port_);
            if (xTaskCreatePinnedToCore((TaskFunction_t) TcpServerSend,
                                        (const char *const) tmp,
                                        4096,
                                        (parameter),
                                        14,
                                        parameter->k_son_task_, 0)
                == pdPASS) {
                parameter->son_task_exists_ = true;
            } else {
                ESP_LOGE(TCP_TAG, "tcp sending task is not built!\n");
            }
        }
        /*Create send buffer*/

        //发送工作
        char rx_buffer[512];
        int len;
        TcpInfoEvent tx_event_;
        while (1) {

            len = recv(parameter->k_client_, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
                break;
            } else if (len == 0) {
                ESP_LOGW(TCP_TAG, "Connection closed");
                break;
            }

            strcpy(tx_event_.buff_arr_, rx_buffer);
            tx_event_.buff_len_ = len;
            if (xQueueSend(*(parameter->uart_tcp_queue_.tcp_to_uart_queue_), &tx_event_, pdMS_TO_TICKS(10))
                == pdPASS) {

            } else {
                ESP_LOGE(TCP_TAG, "TCP_SEND TO QUEUE FAILD\n");
            }

        }
        ESP_LOGE(TCP_TAG, "TcpServerAcceptWithRec task is deleted!!!\n");
        vTaskDelete(NULL);

    }
    vTaskDelete(NULL);
}

esp_err_t TCPBSDAccept(TcpInfoTaskHandleT *parameter) {
    ESP_LOGI(TCP_TAG,"port %d begin to be accepted",parameter->port_);
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);
    int sock = accept(parameter->k_server_, (struct sockaddr *) &source_addr, &addr_len);
    if (sock < 0) {
        ESP_LOGE(TCP_TAG, "Unable to accept connection: errno %d", errno);
        return ESP_FAIL;
    }
    ESP_LOGI(TCP_TAG,"port %d is accepted",parameter->port_);
    int keepInterval = 5;
    int keepIdle = 5;
    int keepCount = 3;
    int keepAlive = 1;
    // Set tcp keepalive option
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

    parameter->k_client_ = sock;
    return ERR_OK;
}

[[maybe_unused]]

static err_t GetConnectState(struct netconn *conn) {
    void *msg;
    err_t err;
    if (sys_arch_mbox_tryfetch(&conn->recvmbox, &msg) != SYS_MBOX_EMPTY) {
        lwip_netconn_is_err_msg(msg, &err);
        if (err == ERR_CLSD) {
            return ERR_CLSD;
        }
    }
    return ERR_OK;
}