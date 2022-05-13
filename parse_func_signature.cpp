#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <stdexcept>
#include <string_view>
#include <regex>
#include <unordered_map>
#include <string_view>
#include <list>

using namespace std;

template<
    class BidirIt,
    class CharT = typename std::iterator_traits<BidirIt>::value_type,
    class Traits = std::regex_traits<CharT>
>
class regex_top_level_token_iterator : std::regex_token_iterator<BidirIt,CharT,Traits> {
private:

    using Base = std::regex_token_iterator<BidirIt,CharT,Traits>;
    Base &base = *static_cast<Base*>(this);

protected:

    BidirIt const _a, _b;  // set in constructor's initialiser list

    typename Base::value_type _strided_match;  /* starts off with matched = false */

    bool Is_Top_Level(void) const
    {
        assert( base != Base() );  // Is_Top_Level should never be called on a "no more matches" token iterator

        size_t counts[4u] = {};  /* (), [], {}, <> */

        for ( BidirIt iter = _a; iter != (*base).second; ++iter )
        {
            switch ( *iter )
            {
            case '(': ++(counts[0u]); continue;
            case ')': --(counts[0u]); continue;

            case '[': ++(counts[1u]); continue;
            case ']': --(counts[1u]); continue;

            case '{': ++(counts[2u]); continue;
            case '}': --(counts[2u]); continue;
            }

            bool const process_next_angle_bracket =
                   0u == counts[0u]
                && 0u == counts[1u]
                && 0u == counts[2u];

            if ( process_next_angle_bracket )
            {
                switch ( *iter )
                {
                case '<': ++(counts[3u]); continue;
                case '>': --(counts[3u]); continue;
                }
            }
        }

        for ( auto const &count : counts )
        {
            if ( 0u != count ) return false;
        }

        return true;
    }

public:

    regex_top_level_token_iterator(void) : Base(), _a(), _b() {}

    void Keep_Searching_If_Necessary(void)
    {
        for ( _strided_match.matched = false; base != Base(); ++base )
        {
            if ( this->Is_Top_Level() )
            {
                _strided_match.second = (*base).second;  // redundant when _strided_match.matched == false
                return;
            }
            else
            {
                if ( false == _strided_match.matched )
                {
                    _strided_match.matched = true;

                    _strided_match.first = (*base).first;
                }
            }
        }

        _strided_match.matched = false;
    }

    regex_top_level_token_iterator(BidirIt const a, BidirIt const b,
                                   typename Base::regex_type const &re,
                                   int const submatch = 0,
                                   std::regex_constants::match_flag_type const m = std::regex_constants::match_default )
      : Base(a,b,re,submatch,m), _a(a), _b(b)
    {
        Keep_Searching_If_Necessary();
    }

    regex_top_level_token_iterator &operator++(void)
    {
        assert( base != Base() );  // operator++ should never be called on a "no more matches" token iterator

        ++base;

        Keep_Searching_If_Necessary();

        return *this;
    }

    bool operator==(regex_top_level_token_iterator const &rhs) const  // Since C++20 we don't need operator!=
    {
        return base == rhs;
    }

    typename Base::value_type const &operator*(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        if ( false == _strided_match.matched )
        {
            return *base;
        }

        return _strided_match;
    }

    typename Base::value_type const *operator->(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        if ( false == _strided_match.matched )
        {
            return base.operator->();
        }

        return &_strided_match;
    }
};

template<
    class BidirIt,
    class CharT = typename std::iterator_traits<BidirIt>::value_type,
    class Traits = std::regex_traits<CharT>
>
class regex_top_level_iterator : std::regex_iterator<BidirIt,CharT,Traits> {
private:

    using Base = std::regex_iterator<BidirIt,CharT,Traits>;
    Base &base = *static_cast<Base*>(this);

protected:

    bool const _disregard_normal_parentheses;
    BidirIt const _a, _b;  // set in constructor's initialiser list

    bool Is_Top_Level(void) const
    {
        assert( base != Base() );  // Is_Top_Level should never be called on a "no more matches" token iterator

        size_t counts[4u] = {};  /* (), [], {}, <> */

        for ( BidirIt iter = _a; iter != (*base)[0u].first; ++iter )
        {
            switch ( *iter )
            {
            case '(': ++(counts[0u]); continue;
            case ')': --(counts[0u]); continue;

            case '[': ++(counts[1u]); continue;
            case ']': --(counts[1u]); continue;

            case '{': ++(counts[2u]); continue;
            case '}': --(counts[2u]); continue;
            }

            bool const process_next_angle_bracket =
                   0u == counts[0u]
                && 0u == counts[1u]
                && 0u == counts[2u];

            if ( process_next_angle_bracket )
            {
                switch ( *iter )
                {
                case '<': ++(counts[3u]); continue;
                case '>': --(counts[3u]); continue;
                }
            }
        }

        for ( auto const &count : counts )
        {
            if ( _disregard_normal_parentheses && (&count == &counts[0u]) ) continue;

            if ( 0u != count ) return false;
        }

        return true;
    }

public:

    regex_top_level_iterator(void) : Base(), _a(), _b(), _disregard_normal_parentheses(false) {}

    void Keep_Searching_If_Necessary(void)
    {
        for ( ; base != Base(); ++base )
        {
            if ( this->Is_Top_Level() )
            {
                return;
            }
        }
    }

    regex_top_level_iterator(BidirIt const a, BidirIt const b,
                             typename Base::regex_type const &re,
                             std::regex_constants::match_flag_type const m = std::regex_constants::match_default,
                             bool arg_disregard_normal_parentheses = false )
      : Base(a,b,re,m), _a(a), _b(b), _disregard_normal_parentheses(arg_disregard_normal_parentheses)
    {
        Keep_Searching_If_Necessary();
    }

    regex_top_level_iterator &operator++(void)
    {
        assert( base != Base() );  // operator++ should never be called on a "no more matches" token iterator

        ++base;

        Keep_Searching_If_Necessary();

        return *this;
    }

    bool operator==(regex_top_level_iterator const &rhs) const  // Since C++20 we don't need operator!=
    {
        return base == rhs;
    }

    typename Base::value_type const &operator*(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        return *base;
    }

    typename Base::value_type const *operator->(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        return base.operator->();
    }
};

using sregex_top_level_token_iterator    = regex_top_level_token_iterator<     string::const_iterator>;
using svregex_top_level_token_iterator   = regex_top_level_token_iterator<string_view::const_iterator>;
using sregex_top_level_iterator          = regex_top_level_iterator      <     string::const_iterator>;
using svregex_top_level_iterator         = regex_top_level_iterator      <string_view::const_iterator>;

using r_sregex_top_level_token_iterator  = regex_top_level_token_iterator<     string::const_reverse_iterator>;
using r_svregex_top_level_token_iterator = regex_top_level_token_iterator<string_view::const_reverse_iterator>;
using r_sregex_top_level_iterator        = regex_top_level_iterator      <     string::const_reverse_iterator>;
using r_svregex_top_level_iterator       = regex_top_level_iterator      <string_view::const_reverse_iterator>;

class Function_Signature {

protected:

    string _original;
    string_view _name;
    std::unordered_map<size_t,size_t> _found_decltypes;

    void Find_All_Decltypes(void)
    {
        string_view const s { _original };

        static regex const my_regex("decltype\\s*\\(");
        svregex_top_level_iterator iter(s.cbegin(), s.cend(), my_regex);

        for ( ; iter != svregex_top_level_iterator(); ++iter )
        {
            size_t const index = (*iter)[0u].second - s.cbegin();

            size_t count = 1u;

            size_t i;
            for ( i = index + 1u; i != s.size(); ++i )
            {
                if      ( ')' == s[i] ) --count;
                else if ( '(' == s[i] ) ++count;

                if ( 0u == count ) break;
            }

            if ( 0u != count ) throw runtime_error("Mismatched parentheses when trying to find decltype's");

            //cout << "=================== Found a decltype =======================" << endl;
            _found_decltypes[index] = i;
        }
    }

public:

    string_view Full_Param_List(void) const
    {
        string_view const s { _original };

        char const *p = _name.cend();

        assert( '(' == *p );

        ++p;

        size_t const index = p - s.cbegin();

        size_t count = 1u;

        size_t i;
        for ( i = index; i != s.size(); ++i )
        {
            if      ( ')' == s[i] ) --count;
            else if ( '(' == s[i] ) ++count;

            if ( 0u == count ) break;
        }

        if ( 0u != count ) throw runtime_error("Mismatched parentheses when trying to find decltype's");

        return string_view(s.cbegin() + index, s.cbegin() + i);
    }

    void Params(list<string_view> &params) const
    {
        string_view const full{ Full_Param_List() };

        static regex const my_regex(",");
        svregex_top_level_token_iterator iter(full.cbegin(), full.cend(), my_regex, -1);

        for ( ; iter != svregex_top_level_token_iterator(); ++iter )
        {
            params.emplace_back( iter->first, iter->second );
        }
    }

    Function_Signature(string_view const arg) : _original(arg)
    {
        string_view const s{ _original };

        Find_All_Decltypes();

        static regex const my_regex("[\\s\\&\\*]([A-z_][A-z_0-9]*)\\s*\\(");

        //cout << "Searching for function name in '" << s << "'" << endl;
        svregex_top_level_iterator iter(s.cbegin(), s.cend(), my_regex, std::regex_constants::match_default, true);

        for ( ; iter != svregex_top_level_iterator(); ++iter )
        {
            //cout << " - - - match - - - " << endl;
            size_t const index_first = (*iter)[0u].first  - s.cbegin();
            size_t const index_last  = (*iter)[0u].second - s.cbegin() - 1u;

            bool should_continue = false;

            for ( auto const &e : _found_decltypes )
            {
                if ( index_first >= e.first && index_last <= e.second )
                {
                    // Ignore this match
                    should_continue = true;
                    break;
                }
            }

            if ( should_continue ) continue;

            _name = string_view( (*iter)[1u].first, (*iter)[1u].second );

            static char const *const disregard[] = {
                "int", "double", "short", "long", "signed", "unsigned",
                "volatile", "const", "restrict"
            };

            for ( auto const &e : disregard )
            {
                if ( e == _name )
                {
                    //cout << "Disregarding '" << e << "'" << endl;
                    _name = {};
                    should_continue = true;
                    break;
                }
            }

            if ( should_continue ) continue;

            break;
        }

        if ( _name.empty() ) throw runtime_error("Couldn't determine name of function");
    }

    string_view Name(void) const
    {
        return _name;
    }
};

typedef int (*FuncPtr)(int);

int (*(&Func(void))[3u])(int)
{
    static FuncPtr local[3u];

    return local;
}

string sigs[] = {
    "void Func(void)",
    "int Func(int j)",
    "void Func(int const k)",
    "void Func(int, vector<int> &)",
    "SomeClass<T, (g_count_lasers > 4u)> Func(int a, vector<decltype(int())> b, char c)",
    "typename SomeClass<T, (g_count_lasers > 4u)>::type Func(int a, vector<decltype(int())> b, char c)",
    "long int (*(&Func(void))[3u])(int)",
    "decltype(SomeOtherFunc(arg)) Func(void)",
    "long long int const Func()",
    "SomeClass const (*const volatile (&Func(void))[3u])(int)",
};

void Remove_Unnecessary_Whitespace(string &s)
{

}


int main()
{
    FuncPtr (&ref)[3u] = Func();

    ref[0u] = nullptr;

    unsigned i = -1;
    for ( auto const &e : sigs )
    {
        Function_Signature fsig(e);

        cout << ++i << ": Name  : " << fsig.Name()            << endl;

        list<string_view> params;
        fsig.Params(params);

        cout <<   i << ": Quantity of Params: " << params.size() << endl;

        for ( auto const &e : params )
        {
            cout << "                       " << e << endl;
        }
    }
}
