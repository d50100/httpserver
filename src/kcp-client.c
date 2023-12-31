// 客户端
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <ikcp.h>
#include <utils.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888



int udp_output(const char* buf, int len, ikcpcb* kcp, void* user) {
    int sock = (int)user;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    return sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
}



int main() {
    // 创建UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return -1;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Failed to set server address");
        return -1;
    }

    // 发送连接请求给服务器
    char connect_msg[] = "connect";
    if (sendto(sockfd, connect_msg, sizeof(connect_msg), 0, (struct sockaddr *) &server_addr, sizeof(server_addr)) <
        0) {
        perror("Failed to send data");
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);


    uint32_t conv;
    int recv_len = recvfrom(sockfd, &conv, sizeof(uint32_t), 0, (struct sockaddr*)&client_addr, &client_addr_len);

    printf("conv %u\n\n", conv);

    // 新建一个 kcp实例
    ikcpcb* kcp = ikcp_create(conv, (void*)sockfd);
    kcp->output = udp_output;

    //if (kcp == null) {
    //    printf("create kcp failed");
    //}

    ikcp_wndsize(kcp, 128, 128);
    ikcp_nodelay(kcp, 1, 10, 2, 1);


    FILE* file = fopen("/root/a.txt", "rb");
    char buffer[1024];
    while (!feof(file)) {
        int n = fread(buffer, 1, sizeof(buffer), file);
        if (n > 0) {
            ikcp_send(kcp, buffer, n);
        }

        ikcp_update(kcp, iclock());
    }

    fclose(file);

    //// 获取要发送的数据
    //char* data = "hello, kcp!";

    //// 发送数据
    //int result = ikcp_send(kcp, data, strlen(data));

    //// 检查发送结果
    //if (result < 0) {
    //    printf("failed to send data!\n");
    //}
    //else {
    //    printf("data sent successfully!\n");
    //}

    // 销毁 kcp 实例
    ikcp_release(kcp);

    close(sockfd);

    return 0;

}