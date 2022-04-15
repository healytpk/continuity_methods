#include <iostream>
using std::cout;
using std::endl;

#define requires /* nothing */
#define continue /* nothing */
#define goto     /* nothing */
#define break    /* nothing */
//#define continue(x) /* nothing */

int j;

namespace My_Program {

int b;

struct Base1A {
	virtual void Set_Int(int const arg) requires continue
	{
		cout << "Base1A value = " << arg << endl;
	}
};

struct Base1B {
	void Set_Int(int const arg)
	{
		cout << "Base1B value = " << arg << endl;
	}
};

struct Derived1A : virtual Base1A {
	void Set_Int(int const arg) override break
	{
		cout << "Derived1A value = " << arg << endl;
	}
};

struct Derived1B : virtual Base1A, Base1B {
	void Set_Int(int const arg) continue /*(!Base1B)*/
	{
		cout << "Derived1B value = " << arg << endl;

		goto continue(arg / 5);
	}
};

struct Base2 {
	void Set_Int(int const arg)
	{
		cout << "Base2 value = " << arg << endl;
	}
};

struct Derived2 : virtual Derived1A, virtual Derived1B, virtual Base2 {
	void Set_Int(int const arg) override continue
	{
		cout << "Derived2 value = " << arg << endl;
	}
};

struct FurtherDerived : virtual Base2, Derived2  {
	void Set_Int(int arg) continue
	{
		cout << "FurtherDerived starts, value = " << arg << endl;

		for ( unsigned i = 0; i < (arg / 4); ++i ) cout << "monkey = " << ++arg << endl;

		cout << "FurtherDerived ends" << endl;
	}
};

namespace MyNamespace {
class Organism {

	class Mammal {
		
		class Canine {
			
			class Wolf {
				
			};
			
		};
		
	};
};

class NonOrganism {
	
};

}
}

int main(void)
{
	My_Program::FurtherDerived object;
	object.Set_Int(7);
}
