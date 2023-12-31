#include "ikcp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <uuid/uuid.h>

#define SERVER_PORT 8888

typedef struct {
    ikcpcb* kcp;
    FILE* file;
    uuid_t uuid;
} Client;

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    // 假设我们有一个哈希表来存储每个客户端的信息
    HashMap<Client> clients;

    char buffer[1024];
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        if (n > 0) {
            // 使用客户端的地址作为键
            Client* client = clients.get(client_addr);
            if (client == NULL) {
                // 如果这是一个新的客户端，创建一个新的KCP对象和文件
                client = new Client;
                client->kcp = ikcp_create(0x11223344, (void*)sock);
                client->kcp->output = udp_output;
                uuid_generate(client->uuid);
                char filename[37];
                uuid_unparse(client->uuid, filename);
                client->file = fopen(filename, "wb");
                clients.put(client_addr, client);
            }

            ikcp_input(client->kcp, buffer, n);
        }

        // 更新每个客户端的KCP状态，并接收数据
        for (Client* client : clients.values()) {
            ikcp_update(client->kcp, iclock());

            while (1) {
                int n = ikcp_recv(client->kcp, buffer, sizeof(buffer));
                if (n < 0) break;
                // 将数据写入到文件
                fwrite(buffer, 1, n, client->file);
            }
        }
    }

    // 释放每个客户端的KCP对象和文件
    for (Client* client : clients.values()) {
        ikcp_release(client->kcp);
        fclose(client->file);
        delete client;
    }

    close(sock);

    return 0;
}

int udp_output(const char* buf, int len, ikcpcb* kcp, void* user) {
    int sock = (int)user;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return sendto(sock, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
}