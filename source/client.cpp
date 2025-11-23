#include"socket.hpp"

int main()
{
    Socket sk;
    sk.create_client_link("127.0.0.1", 9091);
    sk.reuse_address_port();
    char buf[1024] = {0};
    while(1)
    {
        std::string str;
        getline(std::cin, str);
        if(str == "quit") break;
        sk.Send_Block((void*)str.c_str(), str.size());
        ssize_t ret = sk.Recv_Block(buf, 1024);
        buf[ret] = 0;
        std::cout << buf << std::endl;
    }
    return 0;
}