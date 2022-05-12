#include <cstddef>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

string sigs[] = {
    "void Func(void)",
    "int Func(int j)",
    "void Func(int const k)",
    "void Func(int, vector<int> &)",
    "typename SomeClass<T, (g_count_lasers > 4u)>::type Func(int a, vector<decltype(int())> b, char c)",
};

size_t Find_Parenthesis_For_Func_Params(string_view const s, bool const forward = true)
{
    size_t counts[4u] = {};  /* (), [], {}, <> */

    size_t          i = forward ?       0u : s.size() - 1u;
    size_t const stop = forward ? s.size() :            -1;

    auto Change = [forward](size_t &i) -> void
    {
        if ( forward ) ++i;
                  else --i;
    };

    for ( ; i != stop; Change(i) )
    {
        switch ( s[i] )
        {
        case '(': ++(counts[0u]); break;
        case ')': --(counts[0u]); break;

        case '[': ++(counts[1u]); break;
        case ']': --(counts[1u]); break;

        case '{': ++(counts[2u]); break;
        case '}': --(counts[2u]); break;
        }

        bool const process_next_angle_bracket =
               0u == counts[0u]
            && 0u == counts[1u]
            && 0u == counts[2u];

        if ( process_next_angle_bracket )
        {
            switch ( s[i] )
            {
            case '<': ++(counts[3u]); break;
            case '>': --(counts[3u]); break;
            }
        }

        if ( 0u == counts[1u] && 0u == counts[2u] && 0u == counts[3u] )
        {
            if (    ( 1 == counts[0u] &&  true == forward)
                 || (-1 == counts[0u] && false == forward) )
            {
               return i;
            }
        }
    }

    throw runtime_error("Couldn't find open parenthesis for func params");
}

void ParseFuncSig(string s)
{
    size_t i_open  = Find_Parenthesis_For_Func_Params(s,  true),
           i_close = Find_Parenthesis_For_Func_Params(s, false);

    string other(s.cbegin() + i_open + 1u, s.cbegin() + i_close);
    
    cout << other << endl;
}

typedef int (*FuncPtr)(int);

int (*(&Func(void))[3u])(int)
{
    static FuncPtr local[3u];

    return local;
}

int main()
{
    FuncPtr (&ref)[3u] = Func();

    ref[0u] = nullptr;

    for ( auto const &e : sigs )
    {
        ParseFuncSig(e);
    }
}
