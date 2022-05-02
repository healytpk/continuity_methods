
template<class T, class R>
struct Monkey {

    //typedef T allocator_type;

    using allocator_type = R;
};


struct BabyMonkey : Monkey<int,double> {

};

struct BabyMonkey2 : Monkey<float, double> {

};


#include <cassert>
#include <iostream>
#include <string>
#include <regex>
#include <array>
#include <utility>  // pair
#include <exception> // runtime_error
#include <iterator>  // std::next

#include <boost/algorithm/string/trim_all.hpp>  // trim_all (REVISIT FIX - doesn't strip '\n')
#include <boost/algorithm/string/replace.hpp>   // replace_all
#include <boost/algorithm/string/erase.hpp>     // erase_all

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
            if ( 0u != count ) return false;
        }

        return true;
    }

public:

    regex_top_level_iterator(void) : Base(), _a(), _b() {}

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
                             std::regex_constants::match_flag_type const m = std::regex_constants::match_default )
      : Base(a,b,re,m), _a(a), _b(b)
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

#if 0

void PrintUsings(string_view const full_classname, string_view const unspecialised, string_view const specialised)
{
    regex const my_separator(",");
    svregex_top_level_token_iterator it_unsp(unspecialised.cbegin(), unspecialised.cend(), my_separator, -1);
    svregex_top_level_token_iterator   it_sp(  specialised.cbegin(),   specialised.cend(), my_separator, -1);

    for ( ; (it_unsp != svregex_top_level_token_iterator()) && (it_sp != svregex_top_level_token_iterator()); ++it_unsp, ++it_sp )
    {
        string impersonator( *it_unsp );

        boost::trim_all(impersonator);

        if ( impersonator.starts_with("class") )
        {
            impersonator = impersonator.substr(5u);
            boost::trim_all(impersonator);
        }
        else if ( impersonator.starts_with("typename") )
        {
            impersonator = impersonator.substr(8u);
            boost::trim_all(impersonator);
        }
        else
        {
            //cout << ".......................Skipping template parameter that isn't a type..................." << endl;
            continue;
        }

        string original( *it_sp );
        boost::trim_all(original);

        cout << "using " << impersonator << " = " << original << ";" << endl;
        
        cout << "g_psuedonyms[\"" << full_classname << "::" << impersonator << "\"] = Find_Class_Relative_To_Scope(\"" << full_classname << "::\", " << original << ");\n";
    }
}

bool Remove_Last_Scope(string_view &str)  // returns false when 'str' is or becomes "::", otherwise returns true
{
    char const separator[] = "::";

    if ( separator == str ) return false;

    if( (str.size() < (2*sizeof(separator)+1u)) || (false == str.starts_with(separator)) || (false == str.ends_with(separator)) )  // minimum = "::A::"
    {
        throw runtime_error("Remove_Last_Scope: Invalid string");
    }

    regex const double_colon(separator);

    r_svregex_top_level_iterator iter( str.crbegin(), str.crend(), double_colon );  // reverse

    assert( r_svregex_top_level_iterator() != iter );

    ++iter;  // skip the first match because it's the trailing "::"

    str.remove_suffix( std::distance( str.crbegin(), (*iter)[0u].first ) );

    return separator != str;
}

#endif

int main(void)
{

#if 1

    string str("dog, cat, fish, (frogs,toads), monkeys, elephants, (lizards, amphibians<true,1,(g_max_elements > 5u)>), sharks");

    cout << str << endl;

    regex const my_separator(",");

    svregex_top_level_token_iterator iter( string_view(str).cbegin(), string_view(str).cend(), my_separator, -1 );
    
    for ( ; iter != svregex_top_level_token_iterator(); ++iter )
    {
        string tmp(*iter);
        boost::trim_all(tmp);
        cout << tmp << endl;
    }

#else

    string str("::std::__allocator_traits_base::__rebind<_Tp,_Up,__void_t<typename _Tp::template rebind<_Up>::other>>::");

    cout << "Original: " << str << endl;

    string_view sv(s_str);

    for (; /* ever */; )
    {
        bool retval = Remove_Last_Scope(sv);

        cout << sv << endl;

        if ( false == retval ) break;
    }

#endif

    return 0;

#if 0

    pair<string, string> classes[] = {
        { "class T, class R", "int, double" },
        { "int, typename R, B<Monkey> *k", "5, Monkey<int,float,typename Frog::Parent>, nullptr" },
        { "typename Allocator, class Deleter, B<Monkey> *k, bool is_pod", "std::vector<double>, Monkey<int,float,typename Frog::Parent<int,float,(g_max_elements > 7u)>>, nullptr, false" },
    };
    
    for ( auto const &e : classes )
    {
        PrintUsings("::std::exception", e.first, e.second);
        cout << "====================================" << endl;
    }

#endif
}
