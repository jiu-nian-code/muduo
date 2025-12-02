#include"http_server.hpp"

int main()
{
    Socket sk;
    sk.create_client_link(9091, "127.0.0.1");
    sk.reuse_address_port();
    char buf[1024] = {0};
    std::string str("GET / HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 100\r\n\r\n");
    // getline(std::cin, str);
    // if(str == "quit") break;
    while(1)
    {
        ssize_t ret = sk.Send((void*)str.c_str(), str.size());
        std::cout << "Send " << ret << std::endl;
        if(ret < 0) { std::cout << "send error, over." << std::endl; }
        ret = sk.Recv(buf, 1024);
        std::cout << "Recv " << ret << std::endl;
        if(ret < 0) { std::cout << "link is over, quit." << std::endl; }
        buf[ret] = 0;
        std::cout << buf << std::endl;
        sleep(3);
        return 0;
    }
    return 0;
}