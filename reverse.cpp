namespace Continuity_Methods {

    typedef decltype(sizeof(char)) size_t;
    typedef decltype((char*)0 - (char*)0) ptrdiff_t;

    template <typename T>
    class reverse_iterator {
    protected:

        T *current;

    public:

        bool operator!=(reverse_iterator const &rhs) const
        {
            return current != rhs.current;
        }

        constexpr reverse_iterator(T *arg) : current(arg) {}

        constexpr T &operator*() const
        {
            T *tmp = current;
            return *--tmp;
        }

        constexpr reverse_iterator &operator++()
        {
            --current;
            return *this;
        }

        constexpr reverse_iterator &operator--()
        {
            ++current;
            return *this;
        }

        constexpr T &operator[](ptrdiff_t _Off) const
        {
            return current[static_cast<ptrdiff_t>(-_Off - 1)];
        }
    };

    template <typename T>
    struct reversion_wrapper { T& t; };

    template <typename T, long unsigned N>
    reverse_iterator<T> begin(reversion_wrapper<T[N]> arg)
    {
        return reverse_iterator<T>(arg.t + N);
    }

    template <typename T, long unsigned N>
    reverse_iterator<T> end(reversion_wrapper<T[N]> arg)
    {
        return reverse_iterator<T>(arg.t + 0u);
    }

    class reversed_order_t {} destructor_order;

    template <typename T, long unsigned N>
    reversion_wrapper<T[N]> operator|(T (&arg)[N], reversed_order_t)
    {
        return { arg };
    }

    class intact_order_t {} constructor_order;

    template <typename T, size_t N>
    T (&operator|(T (&arg)[N], intact_order_t))[N]
    {
        return arg;
    }
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
