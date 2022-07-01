#include <iostream>
#include <ios>
#include <string>

using namespace std;

int main(void)
{
	cin >> noskipws;

	cout << "cout << \"";

	string s;

	while ( getline(cin, s) )
	{
		cout << s;
		cout << "\\n\"\n        \"";
	}
}

