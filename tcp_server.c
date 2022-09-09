static void app_tcp_server_multi_conn_task(void *arg)
{
    struct sockaddr_in serv_addr;
    fd_set all_set, read_set; /*!< 定义文件句柄集合 */
    int sockfd_max = 0;       /*!< 文件句柄最大值 */

    int serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sockfd < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "Socket created, serv_sockfd=%d", serv_sockfd);

    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(TCP_SERVER_PORT);

    if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "Socket binded");

    if (listen(serv_sockfd, TCP_SERVER_LISTEN_CLIENT_NUM) < 0)
    {
        ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
        vTaskDelete(NULL);
    }
    ESP_LOGI(TAG, "Socket listening");

    FD_ZERO(&all_set);             /*!< 清空集合 */
    FD_SET(serv_sockfd, &all_set); /*!< 添加serv_sockfd */
    sockfd_max = serv_sockfd;

    while (1)
    {
        read_set = all_set;
        if (select(sockfd_max + 1, &read_set, NULL, NULL, NULL) == -1) /*!< 监听句柄集的可读性 */
        {
            ESP_LOGE(TAG, "Sever select error");
        }
        for (int sockfd = 0; sockfd <= sockfd_max; sockfd++) /*!< 从零开始判断 */
        {
            if (!FD_ISSET(sockfd, &read_set)) /*!< sockfd在集合中状态是否变化 */
            {
                continue; /*!< 跳出当前循环 */
            }
            if (sockfd == serv_sockfd) /*!< 新的客户端连接 */
            {
                struct sockaddr_in cli_addr;
                socklen_t cli_addr_len;
                int cli_sockfd;
                cli_addr_len = sizeof(cli_addr);
                memset(&cli_addr, 0, cli_addr_len);
                cli_sockfd = accept(serv_sockfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
                if (cli_sockfd < 0)
                {
                    ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
                }
                else
                {
                    FD_SET(cli_sockfd, &all_set);
                    if (cli_sockfd > sockfd_max)
                    {
                        sockfd_max = cli_sockfd;
                    }
                    ESP_LOGI(TAG, "sockfd_max=%d", sockfd_max);
                    ESP_LOGI(TAG, "A new client[cli_sockfd=%d] is connected from %s", cli_sockfd, inet_ntoa(cli_addr.sin_addr));
                }
            }
            else /*!< 客户端的通信数据 */
            {
                char rx_buffer[128] = {0};
                int len = recv(sockfd, rx_buffer, sizeof(rx_buffer) - 1, 0);
                if (len < 0)
                {
                    ESP_LOGE(TAG, "Recv failed: errno %d", errno);
                    FD_CLR(sockfd, &all_set); /*!< 从集合中删除 */
                    close(sockfd);
                    ESP_LOGI(TAG, "sockfd_max=%d", sockfd_max);
                    break;
                }
                else if (len == 0)
                {
                    ESP_LOGI(TAG, "Connection closed");
                    FD_CLR(sockfd, &all_set); /*!< 从集合中删除 */
                    close(sockfd);
                    ESP_LOGI(TAG, "sockfd_max=%d", sockfd_max);
                    break;
                }
                else
                {
                    ESP_LOGI(TAG, "Received %d bytes from socket_fd[%d]:", len, sockfd);
                    ESP_LOGI(TAG, "%s", rx_buffer);

                    flow_control_msg_t msg = {
                        .packet = rx_buffer,
                        .length = len};
                    if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
                    {
                        ESP_LOGE(TAG, "send flow control message failed or timeout");
                       //free rx_buffer?
                    }
                }
            }
        }
    }

    vTaskDelete(NULL);
}
