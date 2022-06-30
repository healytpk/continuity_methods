#include <cstdlib> /* EXIT_FAILURE */

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
	if ( 2 != argc )
		return EXIT_FAILURE;

	string func_name(argv[1]);

	size_t const i_first_colon = func_name.find_first_of(':'),
		     i_first_open_parenthesis = func_name.find_first_of('('),
		     i_first_open_angle = func_name.find_first_of('<');

	if ( string::npos == i_first_colon )
		return 0;

	if ( string::npos != i_first_open_parenthesis )
		if ( i_first_open_parenthesis < i_first_colon )
			return 0;

	if ( string::npos != i_first_open_angle )
		if ( i_first_open_angle < i_first_colon )
			return 0;

	// If control reaches here, we have the scope
	
	cout << func_name.substr(0,i_first_colon) << endl;

	return 0;
}
