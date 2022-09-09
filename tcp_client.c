int tcp_client()
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
     while (1)
    {
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(CLIENTPORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, CLIENTPORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0)
        {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            s_sta_is_connected = false;
            break;
        }
        else
        {
            s_sta_is_connected = true;
            ESP_LOGI(TAG, "Successfully connected");

          //  return sock;
        }
        while(1) {
                fd_set reads;
                FD_ZERO(&reads);
                FD_SET(sock, &reads);

                struct timeval timeout;
                timeout.tv_sec = 0;
                timeout.tv_usec = 100000;
                if (select(sock+1, &reads, 0, 0, &timeout) < 0) {
                    ESP_LOGI(TAG, "select() failed.\n");

                }
                if (FD_ISSET(sock, &reads)) {
                    char read[512];
                    int bytes_received = recv(sock, read, 512, 0);
                    if (bytes_received < 1) {
                        printf("Connection closed by peer.\n");
                        break;
                    }
                    printf("Received (%d bytes): %.*s",
                        bytes_received, bytes_received, read);
                }
                 if (FD_ISSET(0, &reads)) {

                    char read[512];
                    if (!fgets(read, 512, stdin)) break;
                    printf("Sending: %s", read);
                    int bytes_sent = send(sock, read, strlen(read), 0);
                    printf("Sent %d bytes.\n", bytes_sent);
                }
            }
            return sock;
        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
            return -1;
        }
    }
    return 0;
}
