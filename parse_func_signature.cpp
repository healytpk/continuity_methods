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
#include <cctype>  // isalpha, isdigit
#include <atomic>  // atomic
#include <algorithm>  // all_of, any_of

using namespace std;

inline bool Is_Valid_Identifier_Char(char const c)
{
    return std::isalpha(static_cast<char unsigned>(c)) || std::isdigit(static_cast<char unsigned>(c)) || ('_' == c);
}

inline bool Is_Entire_String_Valid_Identifier(string_view const sv)
{
    assert( false == sv.empty() );

    if ( std::isalpha(static_cast<char unsigned>(sv.front())) || ('_' == sv.front()) )  // First character can't be a digit
    {
        return std::all_of( sv.cbegin(), sv.cend(), [](char const c){ return Is_Valid_Identifier_Char(c); } );
    }
    else
    {
        return false;
    }
}

struct ShortStr {
    char c[4u];
};

inline short unsigned Unique_12_Bit(void)
{
    static std::atomic<short unsigned> retval = -1;

    return ++retval;
}

inline ShortStr Unique_3_Hex_Chars(void)
{
    ShortStr str;

    short unsigned const x = Unique_12_Bit();

    static char const alphabet[] = "0123456789abcdef";

    for ( unsigned i = 0u; i != 3u; ++i )
    {
        str.c[i] = alphabet[ (x >> (8u - 4u*i)) & 0xf ];
    }

    str.c[3u] = '\0';

    return str;
}

char const *const g_strs_keywords[] = {
    "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel",
    "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "char8_t", "char16_t",
    "char32_t", "class", "compl", "concept", "const", "consteval",
    "constexpr", "constinit", "const_cast", "continue", "co_await",
    "co_return", "co_yield", "decltype", "default", "delete", "do",
    "double", "dynamic_cast", "else", "enum", "explicit", "export",
    "extern", "false", "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace", "new", "noexcept", "not",
    "not_eq", "nullptr", "operator", "or", "or_eq", "private",
    "protected", "public", "reflexpr", "register", "reinterpret_cast",
    "requires", "return", "short", "signed", "sizeof", "static",
    "static_assert", "static_cast", "struct", "switch", "synchronized",
    "template", "this", "thread_local", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
};

void Find_And_Erase_All_Keywords(string &s)
{
    for ( auto const &e : g_strs_keywords )
    {
        for ( size_t i = 0u; i < s.size(); ++i )
        {
            i = s.find(e, i);  // e.g. find the word "volatile"

            if ( -1 == i ) break;

            //cout << "Found keyword: " << e << endl;

            size_t const one_past_last = i + strlen(e);

            if ( (one_past_last < s.size()) && Is_Valid_Identifier_Char(s[one_past_last]) )
            {
                //cout << "disregarding" << endl;
                continue;
            }

            if (     (0u != i)       && Is_Valid_Identifier_Char(s[i-1u]) )
            {
                //cout << "disregarding" << endl;
                continue;
            }

            //cout << "= = = = ERASING = = = =" << endl;

            s.erase(i,one_past_last - i);

            --i;  // Because it will be incremented automatically

#if 0
            // Now if there is "\(.*\)" then delete it too

            while ( i < s.size() && std::isspace(s[i]) ) ++i;

            if ( i >= s.size() || '(' != s[i] ) { --i; continue; }

            size_t count = 1u;
            for ( size_t j = i + 1u; j != s.size(); ++j )
            {
                if      ( ')' == s[j] ) --count;
                else if ( '(' == s[j] ) ++count;

                if ( 0u == count )
                {
                    s.erase(i, j - i);
                    break;
                }
            }

            if ( 0u != count ) throw runtime_error("unmatched parentheses after a keyword");
#endif
        }
    }
}

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

        std::array<size_t,4u> counts{};  /* (), [], {}, <> */

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

        return std::all_of( counts.cbegin(), counts.cend(), [](size_t const z){ return 0u == z; } );
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

    class Parameter {
    protected:

        string _original;
        string _name;  // REVISIT FIX - Maybe see about a string_view here

    public:

        explicit Parameter(string_view const arg) : _original(arg)
        {
            if ( _original.empty() ) throw runtime_error("Function parameter shouldn't be blank here");

            _name = _original;

            while ( false == _name.empty() && false == Is_Entire_String_Valid_Identifier(_name) )
            {
                //_name.remove_prefix(1u);
                _name.erase(0u,1u);
            }

            if ( _name.empty() )
            {
                _name  = "param_";
                _name += Unique_3_Hex_Chars().c;

                _original += ' ';
                _original += _name;
            }
        }

        string_view Name(void) const
        {
            return _name;
        }

        string_view Full(void) const
        {
            return _original;
        }
    };

    string const _original;
    string_view _name;
    std::unordered_map<size_t,size_t> _found_decltypes;
    list<Parameter> _params;

    void Find_All_Decltypes(string_view const s)
    {
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

    string_view Full_Param_List(void) const
    {
        string_view const s { _original };

        char const *p = _name.cend();  // REVISIT FIX - watch out for whitespace, e.g. "int Func (void)"

        //cout << "------------- BAD CHAR = " << *p << "  (name = " << _name << ")" << endl;
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

    void ProcessParams(void)
    {
        _params.clear();

        string_view const full{ Full_Param_List() };

        static regex const my_regex(",");
        svregex_top_level_token_iterator iter(full.cbegin(), full.cend(), my_regex, -1);

        for ( ; iter != svregex_top_level_token_iterator(); ++iter )
        {
            if ( "void" ==  *iter || "" == *iter ) return;  // REVISIT FIX - watch out for whitespace

            _params.emplace_back( string_view(iter->first, iter->second) );

            //cout << "  Parameter Name: " << _params.back().Name() << endl;
        }
    }

public:

    explicit Function_Signature(string_view const arg) : _original(arg)
    {
        string without_keywords{ _original };

        //cout << "Before: " << without_keywords << endl;
        Find_And_Erase_All_Keywords(without_keywords);
        without_keywords.insert(0u," ");
        Find_All_Decltypes(without_keywords);
        //cout << "After: " << without_keywords << endl;

        string_view const s{without_keywords};
        static regex const my_regex("[\\s\\&\\*]([A-z_][A-z_0-9]*)\\s*\\(");

        //cout << "Searching for function name in '" << s << "'" << endl;
        svregex_top_level_iterator iter(s.cbegin(), s.cend(), my_regex, std::regex_constants::match_default, true);

        unsigned count = 0u;
        for ( ; iter != svregex_top_level_iterator(); ++iter )
        {
            //cout << " - - - match - - - " << endl;
            size_t const index_first = (*iter)[0u].first  - s.cbegin();
            size_t const index_last  = (*iter)[0u].second - s.cbegin() - 1u;

            if ( std::any_of(_found_decltypes.cbegin(),
                             _found_decltypes.cend(),
                             [index_first,index_last](auto const &e){ return index_first >= e.first && index_last <= e.second; } ) )
            {
                continue;
            }

            string_view const found { (*iter)[1u].first, (*iter)[1u].second };

            for ( size_t location = 0u; location < _original.size(); ++location)
            {
                //cout << "Searching for '" << found << "' inside '" << _original << "'" << endl;

                location = _original.find(found, location);

                assert( -1 != location );

                //cout << "Found at location " << location << endl;

                size_t const one_past_last = location + found.size();

                if ( (one_past_last < _original.size()) && Is_Valid_Identifier_Char(_original[one_past_last]) )
                {
                    //cout << "disregarding AAA" << endl;
                    continue;
                }

                if (     (0u != location)       && Is_Valid_Identifier_Char(_original[location-1u]) )
                {
                    //cout << "disregarding BBB because previous char at location " << location - 1u << " is " << s[location - 1u] << endl;
                    continue;
                }

                _name = string_view( _original.cbegin() + location, _original.cbegin() + location + found.size() );

                break;
            }

            //cout << "SECOND " << _name << endl;

            ++count;

            if ( 1u == count ) continue;

            break;
        }

        if ( _name.empty() ) throw runtime_error("Couldn't determine name of function");

        ProcessParams();
    }

    void Original_Function_Signature_Renamed(ostream &os) const
    {
        os << string_view( _original.cbegin().base(), _name.cend() );

        os << "____WITHOUT_CONTINUITY";

        os << string_view( _name.cend(), _original.cend().base() );
    }

    void Signature_Of_Replacement_Function(ostream &os) const
    {
        os << string_view( _original.cbegin().base(), _name.cend() );

        os << '(';

        for ( auto &e : _params )
        {
            os << e.Full();

            if ( &e != &_params.back() ) os << ", ";
        }

        os << string_view( Full_Param_List().cend(), _original.cend().base() );
    }

    void Invocation_Of_Original_Function(ostream &os) const
    {
        os << _name << "____WITHOUT_CONTINUITY(";

        for ( auto const &e : _params )
        {
            os << e.Name();

            if ( &e != &_params.back() )
            {
                os << ", ";
            }
        }

        os << ")";
    }

    string_view Name(void) const
    {
        return _name;
    }

    string_view Original(void) const
    {
        return _original;
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
    "void Func(int i, vector<int> &vec)",
    "SomeClass<T, (g_count_lasers > 4u)> Func(int a, vector<decltype(int())> b, char c)",
    "typename SomeClass<T, (g_count_lasers > 4u)>::type Func(int a, vector<decltype(int())>&, char c)",
    "long int (*(&Func(void))[3u])(int count)",
    "decltype(SomeOtherFunc(arg)) Func(void)",
    "long long int const Func()",
    "SomeClass const (*const volatile (&Func(void))[3u])(int)",
};

#if 0
void Remove_Unnecessary_Whitespace(string &s)
{

}
#endif

int main(void)
{
    //string str("const I am a voltile restricted restrict pointer that has an if statement in between if you believe me const");
#if 0
    string str("pconst*const*const*consted*const");

    Function_Signature::Find_And_Erase_All_Keywords(str);

    cout << str << endl;

    return 0;
#endif

    FuncPtr (&ref)[3u] = Func();

    ref[0u] = nullptr;

    unsigned i = -1;
    for ( auto const &e : sigs )
    {
        ++i;

        cout << "===================================================" << endl;

        //cout << i << ": Intact Signature : " << e << endl;

        Function_Signature fsig(e);

        cout << i << ": Address of Method : &MyClass::" << fsig.Name() << endl;
        cout << i << ": Original Signature Renamed : "; fsig.Original_Function_Signature_Renamed(cout); cout << endl;
        cout << i << ": Signature of replacement method : "; fsig.Signature_Of_Replacement_Function(cout); cout << endl;
        cout << i << ": Invoke original renamed method : p->"; fsig.Invocation_Of_Original_Function(cout); cout << ";" << endl;
    }
}
