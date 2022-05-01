
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
class top_level_regex_token_iterator : regex_token_iterator<BidirIt,CharT,Traits> {
private:

    using Base = regex_token_iterator<BidirIt,CharT,Traits>;
    Base &base = *static_cast<Base*>(this);

protected:

    BidirIt const _a, _b;

    mutable bool _strided = false;
    mutable Base::value_type _match{};

    bool Is_Top_Level(void) const
    {
        assert( base != Base() );  // Is_Top_Level should never be called on a "no more matches" token iterator

        size_t counts[4u] = {};  /* (), [], {}, <> */

        std::sub_match<BidirIt> const &monkeyA = *base;

        std::pair<BidirIt, BidirIt> const &monkeyB = monkeyA;

        for ( BidirIt iter = _a; iter != monkeyB.second; ++iter )
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

    top_level_regex_token_iterator(BidirIt a, BidirIt b,
                                   regex_token_iterator<BidirIt,CharT,Traits>::regex_type const &re,
                                   int submatch = 0,
                                   std::regex_constants::match_flag_type m = std::regex_constants::match_default )
    : Base(a,b,re,submatch, m), _a(a), _b(b)
    {
        for ( _strided = false; base != Base(); ++base)
        {
            if ( this->Is_Top_Level() )
            {
                break;
            }
            else
            {
                if ( false == _strided )
                {
                    _match.first = (*base).first;
                    _strided = true;
                }
            }            
        }
    }

    top_level_regex_token_iterator &operator++(void)
    {
        assert( base != Base() );  // operator++ should never be called on a "no more matches" token iterator

        ++base;

        for ( _strided = false; base != Base(); ++base)
        {
            if ( this->Is_Top_Level() )
            {
                break;
            }
            else
            {
                if ( false == _strided )
                {
                    _match.first = (*base).first;
                    _strided = true;
                }
            }            
        }

        return *this;
    }

    bool operator==(top_level_regex_token_iterator const &rhs) const
    {
        return base == rhs;
    }


    Base::value_type const &operator*(void) const
    {
        assert( base != Base() );  // operator* should never be called on a "no more matches" token iterator

        if ( false == _strided )
        {
            return *base;
        }

        _match.matched = true;

        /* _match.first is already set */
        _match.second = (*base).second;

        return _match;
    }
};

using svtop_level_regex_token_iterator = top_level_regex_token_iterator<std::string_view::const_iterator>;

void PrintUsings(string_view const full_classname, string_view const unspecialised, string_view const specialised)
{
    regex const my_separator(",");
    svtop_level_regex_token_iterator it_unsp(unspecialised.cbegin(), unspecialised.cend(), my_separator, -1);
    svtop_level_regex_token_iterator   it_sp(  specialised.cbegin(),   specialised.cend(), my_separator, -1);

    for ( ; (it_unsp != svtop_level_regex_token_iterator()) && (it_sp != svtop_level_regex_token_iterator()); ++it_unsp, ++it_sp )
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

int main(void)
{
    string str("dog, cat, fish, (frogs,toads), monkeys, elephants, (lizards, amphibians<true,1>), sharks");

    cout << str << endl;

    regex const my_separator(",");

    svtop_level_regex_token_iterator iter( string_view(str).cbegin(), string_view(str).cend(), my_separator, -1 );
    
    for ( ; iter != svtop_level_regex_token_iterator(); ++iter )
    {
        string tmp(*iter);
        boost::trim_all(tmp);
        cout << tmp << endl;
    }
    
    return 0;

    pair<string, string> classes[] = {
        { "class T, class R", "int, double" },
        { "int, typename R, B<Monkey> *k", "5, Monkey<int,float,typename Frog::Parent>, nullptr" },
        { "typename Allocator, class Deleter, B<Monkey> *k, bool is_pod", "std::vector<double>, Monkey<int,float,typename Frog::Parent<int,float>>, nullptr, false" },
    };
    
    for ( auto const &e : classes )
    {
        PrintUsings("::std::exception", e.first, e.second);
        cout << "====================================" << endl;
    }
}
