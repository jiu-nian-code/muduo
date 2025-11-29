#include"util.hpp"

#include"http_requst.hpp"

#include"http_response.hpp"

void test_str_split()
{
    std::string str = "thxc++13355553430c++0210c++c++c++0721";
    std::string inter = "c++";
    std::vector<std::string> arr;
    Util::str_split(str, inter, arr);
    for(auto& e : arr)
        std::cout << e << std::endl;
}

void test_read_file()
{
    std::string filename("./log.text");
    Buffer buf;
    Util::read_file(filename, buf);
    char buff[1024];
    // buf.read(buff, 1024);
    buf.PRINT();
}

void test_write_file()
{
    std::string filename("./log.text");
    Buffer buf;
    char arr[] = "Good moring china, now I have an ice cream.\n";
    buf.wirte(arr, sizeof(arr));
    Util::write_file(filename, buf);
}

void test_url_encoding()
{
    std::string str(":}{>@$&*(<>^?:@*())}");
    // std::cout << str << std::endl;
    std::cout << Util::url_encoding(str, true) << std::endl;
}

void test_url_decodeing()
{
    std::string str("https%3a%2f%2fwww.bilibili.com%2fvideo%2fBV1WgkQBdEYK%2f%3fspm_id_from%3d333.1387.favlist.content.click%26vd_source%3dc3bda6cd5dd528c949934deddc5b7f41");
    std::cout << Util::url_decodeing(str, true) << std::endl;
}

void test_get_status_message()
{
    std::cout << Util::get_status_message(301) << std::endl;
}

void test_get_file_mime()
{
    std::cout << Util::get_file_mime("log.zip") << std::endl;
}

void test_is_directory()
{
    std::cout << Util::is_directory("log.txt") << std::endl;
    std::cout << Util::is_directory("test") << std::endl;
}

void test_is_regular()
{
    std::cout << Util::is_regular("log.txt") << std::endl;
    std::cout << Util::is_regular("test") << std::endl;
}

void test_is_valid_path()
{
    std::cout << Util::is_valid_path("./..") << std::endl;
}

int main()
{
    // test_str_split();
    // test_read_file();
    // test_write_file();
    // test_url_encoding();
    // test_url_decodeing();
    // test_get_status_message();
    // test_get_file_mime();
    // test_is_directory();
    // test_is_regular();
    test_is_valid_path();
    return 0;
}