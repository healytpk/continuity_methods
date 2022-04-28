#include <iostream>
using std::cout;
using std::endl;

#define requires /* nothing */
#define continue /* nothing */
#define goto     /* nothing */
#define break    /* nothing */
//#define continue(x) /* nothing */

int j;

namespace Caga { }

namespace My_Program {

int b;

struct Base1A {
    Base1A(void) { cout << "Base1A::Constructor" << endl; }
	virtual void Set_Int(int const arg) requires continue
	{
		cout << "Base1A value = " << arg << endl;
	}
};

struct Base1B {
    Base1B(void) { cout << "Base1B::Constructor" << endl; }
	void Set_Int(int const arg)
	{
		cout << "Base1B value = " << arg << endl;
	}
};

struct Derived1A : virtual Base1A {
    Derived1A(void) { cout << "Derived1A::Constructor" << endl; }
	void Set_Int(int const arg) override break
	{
		cout << "Derived1A value = " << arg << endl;
	}
};

struct Derived1B : virtual Base1A, Base1B {
    Derived1B(void) { cout << "Derived1B::Constructor" << endl; }
	void Set_Int(int const arg) continue /*(!Base1B)*/
	{
		cout << "Derived1B value = " << arg << endl;

		goto continue(arg / 5);
	}
};

struct Base2 {
    Base2(void) { cout << "Base2::Constructor" << endl; }
	void Set_Int(int const arg)
	{
		cout << "Base2 value = " << arg << endl;
	}
};

struct Derived2 : virtual Derived1A,virtual Derived1B,virtual Base2 {
    Derived2(void) { cout << "Derived2::Constructor" << endl; }
	void Set_Int(int const arg) override continue
	{
		cout << "Derived2 value = " << arg << endl;
	}
};

struct FurtherDerived : virtual Base2, Derived2  {
    FurtherDerived(void) { cout << "FurtherDerived::Constructor" << endl; }
	void Set_Int(int arg) continue
	{
		cout << "FurtherDerived starts, value = " << arg << endl;

		for ( unsigned i = 0; i < (arg / 4); ++i ) cout << "monkey = " << ++arg << endl;

		cout << "FurtherDerived ends" << endl;
	}
};

namespace Caga { }

namespace MyNamespace {

namespace Caga { }
using Frog = ::My_Program::FurtherDerived;
typedef ::My_Program::FurtherDerived Toad;

class Organism {};
class Mammal : Organism {};
class Canine : Mammal {};
class Wolf : Canine {};

class NonOrganism {
	
};

}
}

int main(void)
{
	My_Program::FurtherDerived object;
	object.Set_Int(7);
}
