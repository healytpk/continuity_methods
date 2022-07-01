namespace Continuity_Methods {

    typedef decltype(sizeof(char)) size_t;
    typedef decltype((char*)0 - (char*)0) ptrdiff_t;

    template<typename T>
    class reverse_iterator {
    protected:

        T *p;

    public:

        constexpr bool operator!=(reverse_iterator const &rhs) const
        {
            return p != rhs.p;
        }

        constexpr reverse_iterator(T *arg) : p(arg) {}

        constexpr T &operator*(void) const
        {
            T *tmp = p;
            return *--tmp;
        }

        constexpr reverse_iterator &operator++(void)
        {
            --p;
            return *this;
        }

        constexpr reverse_iterator &operator--(void)
        {
            ++p;
            return *this;
        }

        constexpr T &operator[](ptrdiff_t const arg) const
        {
            return p[static_cast<ptrdiff_t>(-arg - 1)];
        }
    };

    template<typename T,size_t N>
    struct reversion_wrapper {
        
        T (&t)[N];

        constexpr reverse_iterator<T> begin(void)
        {
            return reverse_iterator<T>(t + N);
        }

        constexpr reverse_iterator<T> end(void)
        {
            return reverse_iterator<T>(t + 0u);
        }

    };

    class reversed_order_t {};

    template<typename T, size_t N>
    constexpr reversion_wrapper<T,N> operator|(T (&arg)[N], reversed_order_t)
    {
        return { arg };
    }

    class intact_order_t {} ;

    template<typename T, size_t N>
    constexpr T (&operator|(T (&arg)[N], intact_order_t))[N]
    {
        return arg;
    }

    intact_order_t   constexpr constructor_order;
    reversed_order_t constexpr destructor_order ;
} // close nameapace

#include <iostream>
using std::cout;
using std::endl;

int main(void)
{
    int arr[] = { 1, 2, 3, 4, 5 };

    int const arr_fixed[] = { 1, 2, 3, 4, 5 };

    using namespace Continuity_Methods;

    for ( auto const &e : arr | constructor_order )
    {
        cout << e << endl;
    }

    for ( auto &e : arr | constructor_order )
    {
        e += 4u;
        cout << e << endl;
    }

    for ( auto const &e : arr | destructor_order )
    {
        cout << e << endl;
    }

    for ( auto &e : arr | destructor_order )
    {
        e *= 3u;
        cout << e << endl;
    }
}
