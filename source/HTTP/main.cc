#include"util.hpp"

void test_str_split()
{
    std::string str = "thxc++13355553430c++0210c++c++c++0721";
    std::string inter = "c++";
    std::vector<std::string> arr;
    Util::str_split(str, inter, arr);
    for(auto& e : arr)
        std::cout << e << std::endl;
}

int main()
{
    test_str_split();
    return 0;
}