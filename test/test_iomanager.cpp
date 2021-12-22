#include "kvell.h"
#include "iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>

kvell::Logger::ptr g_logger = KV_LOG_ROOT();

int sock = 0;

void test_fiber() {
    KV_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //kvell::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        KV_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        kvell::IOManager::GetThis()->addEvent(sock, kvell::IOManager::READ, [](){
            KV_LOG_INFO(g_logger) << "read callback";
        });
        kvell::IOManager::GetThis()->addEvent(sock, kvell::IOManager::WRITE, [](){
            KV_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            kvell::IOManager::GetThis()->cancelEvent(sock, kvell::IOManager::READ);
            close(sock);
        });
    } else {
        KV_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    kvell::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

kvell::Timer::ptr s_timer;
void test_timer() {
    kvell::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        KV_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000, true);
            //s_timer->cancel();
        }
    }, true);
}

int main(int argc, char** argv) {
    //test1();
    test_timer();
    return 0;
}