// 服务器端
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <ikcp.h>
#include <utils.h>

#define SERVER_PORT 8888


int udp_output(const char* buf, int len, ikcpcb* kcp, void* user) {
    int sock = (int)user;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
}


void handle_data(const char* buf, int len) {
    // 在这里处理接收到的数据
    printf("Received data: %s\n", buf);
}


int main() {
    // 创建UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return -1;
    }

    // 绑定服务器端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        return -1;
    }

    // 接收客户端连接请求
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
    if (recv_len < 0) {
        perror("Failed to receive data");
        return -1;
    }

    // 解析客户端请求，生成conv值
    uint32_t conv = generate_conv();

    char* mm = (char*) & conv;
    // 发送conv值给客户端
    if (sendto(sockfd, (void *)mm, sizeof(uint32_t), 0, (struct sockaddr*)&client_addr, client_addr_len) < 0) {
        perror("Failed to send data");
        return -1;
    }

    // 关闭socket
    // close(sockfd);

    // 新建一个 kcp 实例
    ikcpcb* kcp = ikcp_create(conv, (void*)sockfd);
    kcp->output = udp_output;

    if (kcp == NULL) {
        printf("create kcp failed");
    }

    ikcp_wndsize(kcp, 128, 128);
    ikcp_nodelay(kcp, 1, 10, 2, 1);

    // char buffer[1024];
    memset(buffer, 0, 1024);
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        if (n > 0) {
            ikcp_input(kcp, buffer, n);
        }

        ikcp_update(kcp, iclock());

        while (1) {
            int n = ikcp_recv(kcp, buffer, sizeof(buffer));
            if (n < 0) break;
            // 处理接收到的数据
            handle_data(buffer, n);
        }
    }

    // 获取要发送的数据
    char* data = "Hello, KCP!";

    // 发送数据
    int result = ikcp_send(kcp, data, strlen(data));

    // 检查发送结果
    if (result < 0) {
        printf("Failed to send data!\n");
    }
    else {
        printf("Data sent successfully!\n");
    }

    // 销毁 KCP 实例
    ikcp_release(kcp);

    return 0;
}