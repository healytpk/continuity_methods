#include <cassert>    // cassert
#include <iterator>   // iterator_traits
#include <regex>      // regex_token_iterator, regex, regex_traits, regex_constants

template<
    class BidirIt,
    class CharT = typename std::iterator_traits<BidirIt>::value_type,
    class Traits = std::regex_traits<CharT>
>
class top_level_regex_token_iterator : std::regex_token_iterator<BidirIt,CharT,Traits> {
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
            case '(': ++(counts[0u]); break;
            case ')': --(counts[0u]); break;
            
            case '[': ++(counts[1u]); break;
            case ']': --(counts[1u]); break;
            
            case '{': ++(counts[2u]); break;
            case '}': --(counts[2u]); break;
            
            case '<': ++(counts[3u]); break;
            case '>': --(counts[3u]); break;
            }
        }

        for ( auto const &count : counts )
        {
            if ( 0u != count ) return false;
        }

        return true;
    }

public:

    top_level_regex_token_iterator(void) : Base(), _a(), _b() {}

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

    top_level_regex_token_iterator(BidirIt const a, BidirIt const b,
                                   typename Base::regex_type const &re,
                                   int const submatch = 0,
                                   std::regex_constants::match_flag_type const m = std::regex_constants::match_default )
      : Base(a,b,re,submatch,m), _a(a), _b(b)
    {
        Keep_Searching_If_Necessary();
    }

    top_level_regex_token_iterator &operator++(void)
    {
        assert( base != Base() );  // operator++ should never be called on a "no more matches" token iterator

        ++base;

        Keep_Searching_If_Necessary();

        return *this;
    }

    bool operator==(top_level_regex_token_iterator const &rhs) const  // Since C++20 we don't need operator!=
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
};

#include <string>
#include <string_view>

using s_top_level_regex_token_iterator = top_level_regex_token_iterator<std::string::const_iterator>;

using sv_top_level_regex_token_iterator = top_level_regex_token_iterator<std::string_view::const_iterator>;

#include <iostream>
#include <string>
using std::cout;
using std::endl;

auto main(void) -> int
{
    std::string const str("dog, cat, fish, (frogs,toads), monkeys, elephants, (lizards, amphibians<true,1>), sharks");

    cout << str << endl;

    std::regex const my_separator(",");

    for ( s_top_level_regex_token_iterator iter( str.cbegin(), str.cend(), my_separator, -1 );
          iter != s_top_level_regex_token_iterator();
          ++iter )
    {
        cout << (*iter) << endl;
    }

    return 0;
}
