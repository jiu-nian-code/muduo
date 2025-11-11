#include<iostream>

#include<vector>

#define DEFAULT_BUFFER_SIZE 1024

// TODO : 可以适用环形数组实现

class buffer
{
    std::vector<char> _arr;
    size_t _read_loc;
    size_t _write_loc;

public:
    buffer():
        _arr(DEFAULT_BUFFER_SIZE),
        _read_loc(0),
        _write_loc(0)
    {}

    bool wirte(char* source, size_t sz)
    {
        size_t head = _read_loc;
        size_t tail = _arr.size() - _write_loc;
        if(tail > sz) // 直接写入
        {

        }
        else if(tail + head > sz) // 对齐后写入
        {

        }
        else // 扩容后写入
        {

        }
    }

    bool read(char* destination, size_t sz)
    {
        
    }
};

int main()
{
    buffer buf;
    return 0;
}