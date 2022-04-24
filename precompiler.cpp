// =====================================================================
// Section 1 of 8 : Override global 'new' and 'delete' for max speed
// =====================================================================

#include <cstddef>    // size_t
#include <cstdlib>    // malloc
#include <new>        // required otherwise we get a compiler error for the 'noexcept'

using std::size_t;

inline void *Implementation_Global_New(size_t size) noexcept
{
    size += 8u - (size % 8u);

    static size_t constexpr bytes_at_a_time = 10485760;  // 10 megabytes

    if ( size > bytes_at_a_time ) return nullptr;

    static void *my_pointers[1000u] = {};  // 1,000 * 10 megabytes = 10 gigabytes

    static void **const p_over_the_edge = my_pointers + sizeof(my_pointers)/sizeof(*my_pointers);

    static void **pp = my_pointers;

    static size_t bytes_allocated_so_far = 0u;

    while ( p_over_the_edge != pp )  // Until we've exhausted 10 gigabytes
    {
        if ( nullptr == *pp )
        {
            *pp = std::malloc(bytes_at_a_time);

            if ( nullptr == *pp ) return nullptr;
        }

        if ( (bytes_allocated_so_far + size) > bytes_at_a_time )
        {
            ++pp;  // This might push it over the edge, so don't check whether it's a nullptr

            bytes_allocated_so_far = 0u;
        }
        else
        {
            break;
        }
    }

    void *const retval = static_cast<char*>(*pp) + bytes_allocated_so_far;

    bytes_allocated_so_far += size;

    return retval;
}

void *operator new  (size_t const size) noexcept { return Implementation_Global_New(size); }
void *operator new[](size_t const size) noexcept { return Implementation_Global_New(size); }
void operator delete  (void *const p) noexcept { /* Do Nothing */ }
void operator delete[](void *const p) noexcept { /* Do Nothing */ }

// =========================================================================
// Section 2 of 8 : Include standard header files, and define global objects
// =========================================================================

bool constexpr verbose = false;
bool constexpr print_all_scopes = false;

bool only_print_numbers; /* This gets set in main -- don't set it here */

#include <cstdlib>    // EXIT_FAILURE, abort
#include <iostream>   // cout, clog, endl
#include <fstream>    // ifstream
#include <algorithm>  // copy, replace, count
#include <iterator>   // next, back_inserter, istream_iterator
#include <string>     // string, to_string
#include <ios>        // ios::binary
#include <iomanip>    // noskipws
#include <stdexcept>  // out_of_range, runtime_error
#include <utility>    // pair<>
#include <cctype>     // isspace
#include <list>       // list
#include <unordered_map> // unordered_map
#include <ranges>     // views::filter
#include <array>      // array
#include <tuple>      // tuple
#include <utility>    // pair, move
#include <string_view> // string_view
#include <set>        // set
#include <regex>      // regex, regex_replace, smatch, match_results

#include <boost/algorithm/string/trim_all.hpp>  // trim_all
#include <boost/algorithm/string/replace.hpp>   // replace_all
#include <boost/algorithm/string/erase.hpp>     // erase_all

using std::cout;
using std::clog;
using std::endl;
using std::ifstream;
using std::string;
using std::to_string;
using std::string_view;
using std::list;
using std::array;
using std::tuple;
using std::pair;

using std::regex;
using std::regex_replace;
using std::smatch;
using std::match_results;

using std::istream_iterator;
using std::back_inserter;
using std::views::filter;
using std::views::split;

string g_intact;

inline void ThrowIfBadIndex(size_t const char_index)
{
    if ( char_index >= g_intact.size() )
        throw std::runtime_error("Cannot access *(p + " + std::to_string(char_index) + ") inside type char[" + std::to_string(g_intact.size()) + "]");    
}

inline size_t LastChar(void) { return g_intact.size() - 1u; }

inline size_t Lines(void) { return std::count( g_intact.begin(), g_intact.end(), '\n' ); }

inline size_t LineOf(size_t const char_index)
{
    ThrowIfBadIndex(char_index);

    return std::count( g_intact.begin(), std::next(g_intact.begin(), char_index), '\n' );
}

inline size_t StartLine(size_t char_index)
{
    ThrowIfBadIndex(char_index);

    while ( (0u != char_index) && ('\n' != g_intact[char_index]) )
    {
        --char_index;
    }
    
    return char_index;
}

inline size_t EndLine(size_t char_index)
{
    ThrowIfBadIndex(char_index);

    while ( (LastChar() != char_index) && ('\n' != g_intact[char_index]) )
    {
        ++char_index;
    }
    
    return char_index;
}



string TextBeforeOpenCurlyBracket(size_t const char_index)  // strips off the template part at the start, e.g. "template<class T>"
{
    ThrowIfBadIndex(char_index);

    if ( 0u == char_index ) return {};

    if ( '{' != g_intact[char_index] ) throw std::runtime_error("This isn't an open curly bracket!");

    size_t i = char_index;

    while ( --i )
    {
        bool break_out_of_loop = false;

        switch ( g_intact[i] )
        {
        case '}':
        case '{':
        case ';':
        //case '(':
        //case ')':
        //case '<':
        //case '>':

            break_out_of_loop = true;
            break;

        default:

            continue;
        }

        if ( break_out_of_loop ) break;
    }

    string retval = g_intact.substr(i + 1u, char_index - i - 1u);   // REVISIT FIX might overlap

    retval = std::regex_replace(retval, std::regex("\\s*::\\s*"), "::");   // Turns "__cxx11:: collate" into "__cxx11::collate"

    boost::algorithm::replace_all(retval, "::", "mOnKeY");
    boost::algorithm::replace_all(retval, ":", " : ");
    boost::algorithm::replace_all(retval, "mOnKeY", "::");
    retval = std::regex_replace(retval, std::regex("\\s"), " ");

    boost::algorithm::trim_all(retval);

    //if ( retval.contains("allocator_traits") ) clog << "1: ===================" << retval << "===================" << endl;
    retval = std::regex_replace(retval, std::regex("(template<.*>) (class|struct) (.*)"), "$2 $3");
    retval = std::regex_replace(retval, std::regex("\\s*,\\s*"), ",");
    //if ( retval.contains("allocator_traits") ) clog << "2: ===================" << retval << "===================" << endl;

    return retval;
}

class CurlyBracketManager {
public:

    struct CurlyPair {
    protected:

        CurlyPair *_parent;
        pair<size_t,size_t> _indices;
        std::list<CurlyPair> _nested;

    public:

        CurlyPair(size_t const first, CurlyPair *const arg_parent)
        {
            _indices.first  =      first;
            _indices.second =         -1;
                    _parent = arg_parent;

            //_nested.reserve(64u);  // We get memory corruption without this - this only applied to vector when it resized, and relocated objects (invalidating iterators)
        }

        void clear(void) { _nested.clear(); }
        
        size_t First(void) const noexcept { return _indices.first ; }

        size_t Last (void) const noexcept { return _indices.second; }

        CurlyPair const *Parent(void) const;
        
        std::list<CurlyPair> const &Nested(void) const noexcept { return _nested; }

        CurlyPair *Add_New_Inner_Scope(size_t const first)
        {
            if ( nullptr == this ) throw std::runtime_error("The 'this' pointer in this method is a nullptr!");

            if ( (-1 != _indices.first) && (first <= _indices.first) ) throw std::runtime_error("Open bracket of inner scope must come after open bracket of outer scope");

            _nested.emplace_back(first, this);

            return &_nested.back();
        }
        
        CurlyPair *Close_Scope_And_Go_Back(size_t const last)
        {
            if ( nullptr == this ) throw std::runtime_error("The 'this' pointer in this method is a nullptr!");

            if ( nullptr ==        _parent ) throw std::runtime_error("The root pair is NEVER supposed to get closed");
            if (    last <= _indices.first ) throw std::runtime_error("Closing backet of inner scope must come after open bracket");

            _indices.second = last;

            return _parent;
        }
    };

protected:

    CurlyPair _root_pair{ (size_t)-1, nullptr};

    void Print_CurlyPair(CurlyPair const &cp, size_t const indentation = 0u) const
    {
        verbose && clog << "- - - - - Print_CurlyPair( *(" << &cp << "), " << indentation << ") - - - - -" << endl;

        string str;

        extern tuple< string,string, list< array<string,3u> >  > Intro_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp);

        if ( false == only_print_numbers )
        {
            str = std::get<1u>(Intro_For_Curly_Pair(cp));
        }

        if ( false == str.empty() || print_all_scopes )
        {
            for ( size_t i = 0u; i != indentation; ++i )
            {
                clog << "    ";
            }

            clog << cp.First() << " (Line #" << LineOf(cp.First())+1u << "), " << cp.Last() << " (Line #" << LineOf(cp.Last())+1u << ")";

            verbose && clog << "  [Full line: " << TextBeforeOpenCurlyBracket(cp.First()) << "]";

            if ( false == only_print_numbers )
            {
                extern string GetNames(CurlyBracketManager::CurlyPair const &);

                //clog << "    " << GetNames(cp);

                clog << "  [" << GetNames(cp) << "]";
            }

            clog << endl;
        }

        for ( CurlyPair const &e : cp.Nested() )
        {
            verbose && clog << "    - - - About to recurse" << endl;
            Print_CurlyPair(e, indentation + 1u);
        }
    }

public:

    struct ParentError : std::exception { };

    void Process(void)
    {
        verbose && clog << "========= STARTING PROCESSING ===========" << endl;

        _root_pair.clear();

        CurlyPair *current = &_root_pair;

        for ( size_t i = 0u; i != g_intact.size(); ++i )
        {
            char const c = g_intact[i];

            if ( '{' == c )
            {
                current = current->Add_New_Inner_Scope(i);
            }
            else if ( '}' == c )
            {
                current = current->Close_Scope_And_Go_Back(i);
            }
        }

        verbose && clog << "========= ENDING PROCESSING ===========" << endl;
    }
    
    
    void Print(void) const
    {
        for ( CurlyPair const &e : _root_pair.Nested() )
        {
            verbose && clog << "    - - - About to recurse" << endl;
            Print_CurlyPair(e);
        }
    }

} g_curly_manager;

CurlyBracketManager::CurlyPair const *CurlyBracketManager::CurlyPair::Parent(void) const
{
    if ( nullptr == this          ) throw std::runtime_error("The 'this' pointer in this method is a nullptr!");
    if ( nullptr == this->_parent ) throw std::runtime_error("Parent() should never be invoked on the root pair");

    if ( &g_curly_manager._root_pair == this->_parent ) throw CurlyBracketManager::ParentError();

    return this->_parent;
}

std::unordered_map< string, tuple< list<CurlyBracketManager::CurlyPair const *>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation
/* For example:

get<0>    class
get<1>    Laser_NitrogenPicoSecond
get<2>    {
              { "" "public" "Laser_Nitrogen"   }
              { "" "public" "Laser_PicoSecond" }
          }
*/

list< array<string,3u> > Parse_Bases_Of_Class(string const &str)
{
    array<string,3u> tmp;

    list< array<string,3u> > retval;

    std::regex const my_regex ("[,](?=[^\\>]*?(?:\\<|$))");  // Need an L-value for some reason (even if it's const)
    std::regex const my_regex2("[\\s](?=[^\\>]*?(?:\\<|$))");  // Need an L-value for some reason (even if it's const)

    for (std::sregex_token_iterator iter(str.begin(), str.end(), my_regex, -1);
         iter != std::sregex_token_iterator();
         ++iter)
    {
        string base_info_str{ *iter };

        for (std::sregex_token_iterator iter2(base_info_str.begin(), base_info_str.end(), my_regex2, -1);
             iter2 != std::sregex_token_iterator();
             ++iter2)
        {
            string word { *iter2 };

            boost::trim_all(word);

            if ( "virtual" == word )
            {
                std::get<0u>(tmp) = "virtual";
            }
            else if ( "public" == word || "protected" == word || "private" == word )
            {
                std::get<1u>(tmp) = word;
            }
            else
            {
                std::get<2u>(tmp) = word;  // This is the name of the base class
            }
        }

        //clog << "[ADD RECORD : " << std::get<0u>(tmp) << " : " << std::get<1u>(tmp) << " : " << std::get<2u>(tmp) << "]";
        retval.push_back(tmp);
    }

    return retval;
}

tuple< string, string, list< array<string,3u> >  > Intro_For_Curly_Pair(CurlyBracketManager::CurlyPair const &cp)
{
    if ( cp.First() >= g_intact.size() || cp.Last() >= g_intact.size() )
    {
        throw std::runtime_error( string("Curly Pair is corrupt [") + to_string(cp.First()) + "," + to_string(cp.Last()) + "]" );
    }

    string intro = TextBeforeOpenCurlyBracket(cp.First());

    if ( intro.starts_with("namespace") )
    {
        string str;

        unsigned i = 0u;
        for ( auto word : intro | std::views::split(' ') )
        {
            if ( 1u != i++ ) continue;

            for (char ch : word)
            {
                str += ch;
            }

            break;
        }

        return { "namespace", str, {} };
    }

    if (   !(intro.starts_with("class") || intro.starts_with("struct"))   ) return {};

    boost::erase_all( intro, " final" );   // careful it might be "final{" REVISIT FIX any whitespace not just space

    // The following finds spaces except those found inside angle brackets
    std::regex const my_regex("[\\s](?=[^\\>]*?(?:\\<|$))");  // Need an L-value for some reason (even if it's const)

    std::sregex_token_iterator iter(intro.begin(), intro.end(), my_regex, -1);

    assert( iter != std::sregex_token_iterator() );  // This should never happen (takes "class" from "class __cxx11::collate : public locale::facet")

    ++iter;

    if ( iter == std::sregex_token_iterator() )
    {
        // Control reaches here if we have an anonymous struct (e.g. inside a function or inside a parent struct)
        return {};
    }

    string const str{ *iter }; // takes "__cxx11::collate" from "class __cxx11::collate : public locale::facet"

    if ( ++iter == std::sregex_token_iterator() ) return { "class", str, {} };  // This bring us to the sole colon
    if ( ++iter == std::sregex_token_iterator() ) return { "class", str, {} };  // This brings it to the first word after the colon (e.g. virtual)

    return { "class", str, Parse_Bases_Of_Class( string( &*(iter->first) ) ) };  // REVISIT FIX might be 'struct' instead of 'class' (public Vs private)
}

string GetNames(CurlyBracketManager::CurlyPair const &cp)
{
    tuple< string, string, list< array<string,3u> >  > tmppair = Intro_For_Curly_Pair(cp);

    string retval = std::get<1u>(tmppair);  /* e.g. 0 = class, 1 = Laser, 2 = [ ... base classes ... ] */

    if ( retval.empty() ) return {};

    size_t iteration = 0u;

    try
    {
        for ( CurlyBracketManager::CurlyPair const *p = &cp; p = p->Parent(); /* no post-processing */ )  // will throw exception when root pair is reached
        {
            //clog << "Iteration No. " << iteration << endl;

            verbose && clog << " / / / / / / About to call Word_For_Curly_Pair(" << p->First() << ", " << p->Last() << ")" << endl;
            string const tmp = std::get<1u>( Intro_For_Curly_Pair(*p) );
            verbose && clog << " / / / / / / Finished calling Word_For_Curly_Pair" << endl;

            if ( tmp.empty() ) continue;

            retval.insert(0u, tmp + "::");
        }
    }
    catch(CurlyBracketManager::ParentError const &)
    {
        //clog << "********** ParentError ************";
    }

    retval.insert(0u, "::");

    //std::unordered_map< string, tuple< list<CurlyBracketManager::CurlyPair*>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation

    std::get<0u>( g_scope_names[retval] ).push_back(&cp);  // Note that these are pointers

    std::get<1u>( g_scope_names[retval] ) = std::get<0u>(tmppair);

    std::get<2u>( g_scope_names[retval] ) = std::get<2u>(tmppair);

    return retval;
}

size_t Find_Last_Double_Colon_In_String(string_view const s)
{
    static regex const r("[:](?=[^\\<]*?(?:\\>|$))");  // matches a semi-colon so long as it's not enclosed in angle brackets

    match_results<string_view::const_reverse_iterator> my_match;

    if ( regex_search(s.crbegin(), s.crend(), my_match, r) )  // returns true if there is at least one match
    {
        size_t const index_of_second_colon_in_last_double_colon = &*(my_match[0u].first) - &s.front(); // REVISIT FIX - consider using my_match.position() here
        assert( ':' == s[index_of_second_colon_in_last_double_colon] );

        size_t const index_of_first_colon_in_last_double_colon  = index_of_second_colon_in_last_double_colon - 1u;
        if ( ':' != s[index_of_first_colon_in_last_double_colon] ) throw std::runtime_error("String should only contain a double-colon pair, but it contains a lone colon");

        //cout << "============ POSITION = " << index_of_first_colon_in_last_double_colon << " =================" << endl;

        return index_of_first_colon_in_last_double_colon;
    }

    return -1;
}

bool Strip_Last_Scope(string &s)
{
    // Change the prefix from "::std::__cxx11" into "::std", so that "::std::__cxx11::locale::facet" becomes "::std::locale::facet"

    if ( s.empty() ) return false;

    size_t const i = Find_Last_Double_Colon_In_String(s);

    switch ( i )
    {
    case (size_t)-1: return false;
    case 0u: s.clear(); return true;
    default: s.resize(i); return true;
    }
}

std::unordered_map<string,string> g_psuedonyms;

bool Recursive_Print_All_Bases_PROPER(string_view const arg_prefix, string classname, std::set<string> &already_recorded, bool is_virtual, string &retval)
{
    decltype(g_scope_names)::mapped_type const *p = nullptr;

    auto Adjust_Class_Name = [](string &arg) -> void
    {
        try
        {
            arg = g_psuedonyms.at(arg);
        }
        catch(std::out_of_range const &e) {}
    };

    Adjust_Class_Name(classname);

    string full_name, prefix(arg_prefix);

    for (  ; /* ever */ ;)
    {
        if ( std::count(prefix.cbegin(),prefix.cend(),'>') != std::count(prefix.cbegin(),prefix.cend(),'<') )
        {
            string tmp("Aborting because prefix has uneven angle brackets - (");
            tmp += prefix;
            tmp += ")\n";

            cout << tmp;
            clog << tmp;
            std::cerr << tmp;
            std::abort();
        }

        full_name  = prefix;
        full_name += "::";
        full_name += classname;

        try
        {
            p = &( g_scope_names.at(full_name) );
        }
        catch (std::out_of_range const &)
        {
            clog << " - - - FIRST FAILED - - - Prefix='" << prefix << "', Classname='" << classname << "' - Fullname='" << full_name << "'" << endl;

            string duplicate_original_full_name{ full_name };  // not const because we std::move() from it later

            string const class_name_without_template_specialisation = std::regex_replace( string(classname), std::regex("<.*>"), "");  // REVISIT FIX -- gratuitous memory allocations

            if ( class_name_without_template_specialisation != classname )
            {
                full_name  = prefix;
                full_name += "::";
                full_name += class_name_without_template_specialisation;

                try
                {
                     p = &( g_scope_names.at(full_name) );
                }
                catch (std::out_of_range const &)
                {
                    clog << " - - - SECOND FAILED - - - Prefix='" << prefix << "', Classname='" << class_name_without_template_specialisation << "' - Fullname='" << full_name << "'" << endl;

                    full_name = std::move(duplicate_original_full_name);
                }
            }
        }

        if ( nullptr != p )
        {
            //clog << "Success: found '" << string(classname) << "') as ('" << full_name << "')";
            break;  // If we already have found the class then no need to keep searching
        }

        // Last resort: Change the prefix from "::std::__cxx11" into "::std", so that "::std::__cxx11::locale::facet" becomes "::std::locale::facet"
        if ( false == Strip_Last_Scope(prefix) ) break;
    }

    if ( nullptr == p )
        throw std::runtime_error("Encountered a class name that hasn't been defined ('" + string(classname) + "') referenced inside ('" + string(arg_prefix) + "')");

    bool const is_new_entry = already_recorded.insert(full_name).second;  // set::insert returns a pair, the second is a bool saying if it's a new entry

    if ( false == is_new_entry && true == is_virtual ) return false;  // if it's not a new entry and if it's virtual

    for ( auto const &e : std::get<2u>(*p) )
    {
        string const &base_name = std::get<2u>(e);

        if constexpr ( verbose )
        {
            clog << " [ALREADY_SEEN=";
            for ( auto const &e : already_recorded ) clog << e << ", ";
            clog << "] ";
            clog << " [[[VIRTUAL=" << (("virtual" == std::get<0u>(e)) ? "true]]]" : "false]]] ") << endl;
        }

        size_t const index_of_last_colon = Find_Last_Double_Colon_In_String(classname);

        if ( -1 != index_of_last_colon && (':' != classname[0u]) )  /* Maybe it's like this:   Class MyClass : public ::SomeClass {}; */
        {
            // This deals with the case of a class inheriting from 'std::runtime_error', which inherits from 'exception' instead of 'std::exception'
            prefix += "::";
            prefix += classname.substr(0u, index_of_last_colon);
        }

        if ( Recursive_Print_All_Bases_PROPER(prefix, base_name, already_recorded, "virtual" == std::get<0u>(e), retval) )
        {
            retval += base_name;
            retval += ", ";
        }
    }

    return true;
}

string Get_All_Bases(string_view arg)
{
    size_t const index_of_last_colon = Find_Last_Double_Colon_In_String(arg);

    if ( -1 == index_of_last_colon ) throw std::runtime_error("There's no double-colon in the argument to Get_All_Bases");

    string_view prefix    = arg.substr(0u, index_of_last_colon);
    string_view classname = arg.substr(index_of_last_colon +  2u);

    std::set<string> already_recorded;

    string retval;

    Recursive_Print_All_Bases_PROPER(prefix, string(classname), already_recorded, false, retval);

    return retval;
}

void Print_Helper_Classes_For_Class(string_view const classname, list<string> const &func_signatures)
{
    cout <<
    "class IMethodInvoker_" << classname << " {\n"
    "protected:\n"
    "\n"
    "    // All methods have one extra\n"
    "    // parameter for 'this' as 'void*'\n";

    for ( auto const &method : func_signatures )
    {
        cout << "    virtual " << method << " = 0;\n";
    }

    cout <<
    "\n    friend class Invoker_" << classname << ";\n"
    "};\n\n";

    cout <<
    "template<class Base, class Derived>\n"
    "class MethodInvoker_" << classname << " final : public IMethodInvoker_" << classname << " {\n"
    "protected:\n"
    "\n"
    "    // All methods have one extra\n"
    "    // parameter for 'this' as 'void*'\n";

    for ( auto const &method : func_signatures )
    {
        std::regex const my_regex("(.+?) (.+?)\\((.*)\\)");

        string const tmp1 = std::regex_replace(method, my_regex, "$1 $2(void *const arg_this,$3)");
        string const tmp2 = std::regex_replace(method, my_regex, "$2");

        cout << "    " << tmp1 << " override\n" <<
                "    {\n"
                "        Base *const p = static_cast<Base*>(static_cast<Derived*>(arg_this))\n"
                "\n"
                "        return p->Base::" << tmp2 << "();\n"
                "    }\n\n";
    }

    cout << "};\n";
}

list< pair<size_t,size_t> > GetOpenSpacesBetweenInnerCurlyBrackets(CurlyBracketManager::CurlyPair const &cp)
{
    assert( cp.Last() > cp.First() );

    // Check 1: To see if there's nothing between the curly brackets
    if ( cp.Last() == (cp.First() + 1u) ) return {};   // e.g. if we have "class Monkey {};"

    // Check 2: To see if there's no inner curly brackets
    if ( cp.Nested().empty() )
    {
        return { { cp.First() + 1u, cp.Last() - 1u }  }; // e.g. if we have "class Monkey { int i; };"
    }

    // If control reaches here, we have nested curly brackets

    list< pair<size_t,size_t> > retval;

    list<CurlyBracketManager::CurlyPair>::const_iterator iter = cp.Nested().cbegin();

    size_t begin_from = cp.First() + 1u;

    for ( auto const &e : cp.Nested() )
    {
        assert( '{' == g_intact[e.First()] );
        assert( '}' == g_intact[e.Last() ] );

        assert( e.First() >= begin_from );

        // Check 3: If we have class Monkey { void Func(void) {{}} };
        if ( begin_from == e.First() ) continue;

        // If control reaches here, there's an open space to record
        retval.push_back( { begin_from, e.First() - 1u } );  // fist and second can be equal here if just one char between '}' and '{'

        begin_from = e.Last() + 1u;

        if ( begin_from >= g_intact.size() )
        {
            cout << "About to abort because begin_from >= g_intact.size()\n" << endl;
            clog << "About to abort because begin_from >= g_intact.size()\n" << endl;
            std::cerr << "About to abort because begin_from >= g_intact.size()\n" << endl;
            std::abort();
        }
    }

    assert( cp.Last() >= begin_from );

    if ( cp.Last() > begin_from ) // must rule out "namespace Dog { namespace Cat { int i; }}"
    {
        retval.push_back( { begin_from, cp.Last() - 1u } );
    }

    return retval;
}

void Find_All_Usings_In_Open_Space(size_t const first, size_t const last, string_view const scope_name)
{
    assert( last >= first );  // It's okay for them to be equal if we have "{ }"

    std::regex r("using[\\s]+(.+)[\\s]*=[\\s]*(.+)[\\s]*;");

    for(std::sregex_iterator i  = std::sregex_iterator(g_intact.begin() + first, g_intact.begin() + last + 1u, r);  // Note the +1 on this line
                             i != std::sregex_iterator();
                             ++i )
    {
#if 1
        string impersonator = regex_replace( i->str(), r, "$1" );
        string original     = regex_replace( i->str(), r, "$2" );

        boost::replace_all(impersonator,"\n"," ");
        boost::replace_all(original    ,"\n"," ");
        boost::trim_all(impersonator);
        boost::trim_all(original    );

        //cout << "Old = " << original << ", New = " << impersonator << endl;

        g_psuedonyms[impersonator] = original;
#else
        string tmp( i->str() );

        boost::replace_all(tmp, "\n", " ");
        boost::trim_all(tmp);

        cout << tmp << endl;
#endif
    }
}

int main(int const argc, char **const argv)
{
    if ( false )
    {
        Print_Helper_Classes_For_Class("MyClass", { "void Trigger(void)", "bool Elevate(float)" });

        return 0;
    }

    std::ifstream f(argv[1u], std::ios::binary);

    f >> std::noskipws;

    if ( false == f.is_open() ) { clog << "Cannot open file" << endl; return EXIT_FAILURE; }

    g_scope_names.reserve(5000u);

    std::copy( istream_iterator<char>(f), istream_iterator<char>(), back_inserter(g_intact) );

    clog << "Bytes: " << g_intact.size() << endl;
    clog << "Lines: " << Lines() << endl;

    std::replace(g_intact.begin(), g_intact.end(), '\0', ' ');

    boost::replace_all(g_intact, "\r\n", " \n");

    boost::replace_all(g_intact, "\r", "\n");

    //std::copy( g_intact.begin(), g_intact.end(), std::ostream_iterator<char>(clog) );
    
    g_curly_manager.Process();

    clog << "====================================== ALL PROCESSING DONE ===========================================" << endl;

    only_print_numbers = false;
    g_curly_manager.Print();

    //std::unordered_map< string, tuple< list<CurlyBracketManager::CurlyPair*>, string, list< array<string,3u> > > > g_scope_names;  // see next lines for explanation

    clog << "====================================== All scope names ==============================================" << endl;
    for ( auto const &e : g_scope_names )
    {
        clog << e.first << " [" << std::get<1u>(e.second) << "] ";

        for ( CurlyBracketManager::CurlyPair const *const  &my_curly_pair_pointer : std::get<0u>(e.second) )  // For classes, just one iteration. For namespaces, many iterations.
        {
            list< pair<size_t,size_t> > const my_list = GetOpenSpacesBetweenInnerCurlyBrackets(*my_curly_pair_pointer);

            for ( auto const my_pair : my_list )
            {
                clog << " Open[" << LineOf(my_pair.first)+1u << "-" << LineOf(my_pair.second)+1u << "]";
                Find_All_Usings_In_Open_Space(my_pair.first, my_pair.second, e.first);
            }

            clog << endl;
        }
    }

    clog << "====================================== Now the pseudonyms (using X = Y;) ==============================================" << endl;
    for ( auto const &e : g_psuedonyms )
    {
        clog << e.first << " == " << e.second << endl;
    }

    clog << "====================================== Now the namespaces ==============================================" << endl;
    for ( auto const &e : g_scope_names | filter([](auto const &arg){ return "namespace" == std::get<1u>(arg.second); }) )
    {
        clog << e.first << endl;
    }

    clog << "====================================== Now the classes ==============================================" << endl;
    for ( auto const &e : g_scope_names | filter([](auto const &arg){ return "class" == std::get<1u>(arg.second) || "struct" == std::get<1u>(arg.second); }) )
    {
        clog << e.first << " - Line#" << LineOf(std::get<0u>(e.second).front()->First())+1u << " to Line#" << LineOf(std::get<0u>(e.second).front()->Last())+1u << " | Bases = " << Get_All_Bases(e.first) << endl;
    }
}
