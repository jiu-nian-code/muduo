#include<iostream>

#include<functional>

#include<string>

void PRINT(std::string a, std::string b)
{
    std::cout << a << " " << b << std::endl;
}

int main()
{
    std::function<void(std::string)> func = std::bind(PRINT, "hello", std::placeholders::_1);
    // func("linux");
    func("thx");
    return 0;
}