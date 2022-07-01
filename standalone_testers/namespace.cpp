namespace Virjacode {
    
    struct A { static int constexpr i = 5u; };

    namespace Ganga { struct A { static int constexpr i = 6u; }; };

    class B {};

    class C : public A {};

    using namespace Ganga;

    //class D : public A {};
};

#include <iostream>

int main(void)
{
    std::cout << Virjacode::A::i << std::endl;
}
