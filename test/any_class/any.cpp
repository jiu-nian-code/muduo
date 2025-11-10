#include<iostream>

#include<string>

class Any
{
    struct placeholder
    {
         virtual ~placeholder(){};
    };

    template<class T>
    struct hole : public placeholder
    {
        hole(const T& con):
            _content(con)
        {
        }

        ~hole()
        {
        }

        T _content;
    };

    placeholder* _ph;
public:
    template<class T>
    Any(const T& everything)
    {
        hole<T>* ptr = new hole<T>(everything);
        _ph= ptr;
    }

    template<class T>
    Any& operator=(const T& everything)
    {
        Any tmp(everything);
        std::swap(_ph, tmp._ph);
        return *this;
    }

    template<class T>
    T* Get_Value()
    {
        return _ph ? &((hole<T>*)_ph)->_content : nullptr;
    }

    ~Any()
    {
        delete _ph;
    }
};

int main()
{
    Any a(1);
    std::string str("hello world");
    a = str;
    std::cout << *a.Get_Value<std::string>() << std::endl;
    return 0;
}