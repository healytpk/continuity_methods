#include <cstddef>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>

using std::size_t;
using std::string;
using std::vector;
using std::runtime_error;
using std::ostringstream;
using std::endl;

string GenerateMethodFirstParagraph(vector<string> const &bases, string const &derived, string const &method, string const &indent = {})
{
    if ( bases.empty() || derived.empty() || method.empty() ) throw runtime_error("Bad args to generate first paragraph");

    ostringstream s;
   
    for ( auto const &base : bases )
    {
        s << indent << "static MethodInvoker<" << base << "," << derived << "> mi_" << base
    << "( reinterpret_cast"
    << "< void(*)(void) >(&"
    << base << "::" << method << ") );" << endl;
    }

    s << endl;

    s << indent << "std::array< Invoker, "
      << bases.size()
      << "u > methods = {" << endl;
    
    for ( auto const &base : bases )
    {
        s << indent << "  Invoker(mi_"
          << base
          << ", this)," << endl;     
    }

    s << indent << "};";

    return s.str();
}

#include <iostream>
using std::cout;
using std::endl;

int main(void)
{
        cout << GenerateMethodFirstParagraph( { "Laser","Laser_Nitrogen", "Laser_PicoSecond" }, "Laser_NitrogenPicoSecond", "Trigger", "        ") << endl;
}

#if 0

static MethodInvoker<Laser,Laser_NitrogenPicoSecond> mi_Laser( reinterpret_cast<void(*)(void)>(&Laser::Elevate) );
    
    static MethodInvoker<Laser_Nitrogen,Laser_NitrogenPicoSecond> mi_Laser_Nitrogen( reinterpret_cast<void(*)(void)>(&Laser_Nitrogen::Elevate) );
    
    static MethodInvoker<Laser_PicoSecond,Laser_NitrogenPicoSecond> mi_Laser_PicoSecond( reinterpret_cast<void(*)(void)>(&Laser_PicoSecond::Elevate) );
    
    std::array<Invoker,3u> methods = {
        Invoker(mi_Laser, this),
        Invoker(mi_Laser_Nitrogen, this),
        Invoker(mi_Laser_PicoSecond, this),
    };

#endif
