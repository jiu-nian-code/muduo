#include<iostream>

#include<regex>

#include<string>

int main()
{
    std::string str("GET /bitejiuyeke HTTP/1.1\r\n");
    // std::cout << 1 << std::endl;
    std::regex e("(GET|POST|PUT|HEAD|DELETE|OPTIONS|TRACE|CONNECT|LINK|UNLINK) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)?");
    std::smatch matchs;
    bool ret = std::regex_match(str, matchs, e);
    if(!ret)
    {
        return -1;
    }
    // std::cout << 1 << std::endl;

    for(auto& s : matchs)
    {
        std::cout << 1 << s << std::endl;
    }
    return 0;
}