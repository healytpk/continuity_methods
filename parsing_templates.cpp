
template<class T, class R>
struct Monkey {

    //typedef T allocator_type;

    using allocator_type = R;
};


struct BabyMonkey : Monkey<int,double> {

};

struct BabyMonkey2 : Monkey<float, double> {

};



#include <iostream>
#include <string>
#include <regex>
#include <array>
#include <utility>  // pair

#include <boost/algorithm/string/trim_all.hpp>  // trim_all (REVISIT FIX - doesn't strip '\n')
#include <boost/algorithm/string/replace.hpp>   // replace_all
#include <boost/algorithm/string/erase.hpp>     // erase_all

using namespace std;

void PrintUsings(string const &unspecialised, string const &specialised)
{
    // The following finds commas except those found inside angle brackets
    std::regex const my_regex("[,](?=[^\\>]*?(?:\\<|$))");

    std::sregex_token_iterator iter_unsp(unspecialised.begin(), unspecialised.end(), my_regex, -1),
                               iter_sp  (  specialised.begin(),   specialised.end(), my_regex, -1);

    for ( ; iter_unsp != sregex_token_iterator() && iter_sp != sregex_token_iterator(); ++iter_unsp, ++iter_sp)
    {
        string unsp(*iter_unsp), sp(*iter_sp);

        boost::trim_all(unsp);
        boost::trim_all(  sp);
        
        if ( false == (unsp.starts_with("class") || unsp.starts_with("typename")) )
        {
            cout << "........skipping template parameter that isn't a type............" << endl;
            continue;
        }
        
        regex const temp_param("[A-z_][A-z_0-9<>,]*(\\s*\\*\\s*|\\s+)(.+)");

        string unspZ = regex_replace(unsp, temp_param, "$2");
          
        cout << "using " << unspZ << " = " << sp << ";" << endl;
    }
}

int main(void)
{
    pair<string, string> classes[] = {
        { "class T, class R", "int, double" },
        { "int, typename R, B<Monkey> *k", "5, Monkey<int,float,typename Frog::Parent>, nullptr" },
        { "typename Allocator, class Deleter, B<Monkey> *k, bool is_pod", "std::vector<double>, Monkey<int,float,typename Frog::Parent>, nullptr, false" },
    };
    
    for ( auto const &e : classes )
    {
        PrintUsings(e.first, e.second);
        cout << "====================================" << endl;
    }
}
