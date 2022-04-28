
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

string_view::iterator Find_Top_Level_Comma(string_view const sv, size_t const which_comma)
{
    assert( -1 != which_comma );

    size_t commas_found = 0u;

    size_t count_open_angle  = 0u;
    size_t count_close_angle = 0u;

    for ( size_t i = 0u; i != sv.size(); ++i )
    {
        if ( count_open_angle < count_close_angle ) throw runtime_error("We've got a close angle bracket out of place here");

        switch ( sv[i] )
        {
        case '<': ++count_open_angle ; break;
        case '>': ++count_close_angle; break;
        case ',':
        
            if ( count_open_angle == count_close_angle )
            {
                if ( which_comma == commas_found++ ) return std::next(sv.begin(), i);
            }
        }
    }

    return sv.end();
}

void PrintUsings(string_view const full_classname, string_view const unspecialised, string_view const specialised)
{
    string_view::iterator i_comma_unsp = unspecialised.begin(),
                            i_comma_sp =   specialised.begin();

    string_view::iterator i_comma_unsp_end,
                            i_comma_sp_end;

    for ( size_t i = 0u; ; ++i )
    {
        if ( string_view(unspecialised).end() == (i_comma_unsp_end = Find_Top_Level_Comma(unspecialised, i)) ) break;
        if ( string_view(  specialised).end() == (i_comma_sp_end   = Find_Top_Level_Comma(  specialised, i)) ) break;

        string impersonator( i_comma_unsp, i_comma_unsp_end );
        
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


        string original(i_comma_sp, i_comma_sp_end);
        boost::trim_all(original);

        cout << "using " << impersonator << " = " << original << ";" << endl;
        
        cout << "g_psuedonyms[\"" << full_classname << "::" << impersonator << "\"] = Find_Class_Relative_To_Scope(\"" << full_classname << "::\", " << original << ");\n";

        i_comma_unsp = ++i_comma_unsp_end;
        i_comma_sp   = ++i_comma_sp_end;
    }
}

int main(void)
{
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
