#include <list>       // list
#include <utility>    // pair
#include <algorithm>  // find
#include <stdexcept>  // out_of_range

template <class key_type, class mapped_type>
class fifo_map {
protected:

	struct value_type : std::pair<key_type,mapped_type> {

        value_type(key_type const &k)
        {
            this->first = k;
        }

		bool operator==(key_type const &rhs) const
		{
			return this->first == rhs;  // We don't compare the second
		}
	};

	std::list<value_type> data;

public:

	mapped_type &operator[](key_type const &k)
	{
		typename decltype(data)::iterator iter = std::find( data.begin(), data.end(), k );

		if ( iter != data.end() ) return iter->second;

		data.emplace_back(k);

		return data.back().second;
	}

    mapped_type const &at(key_type const &k) const
    {
        typename decltype(data)::const_iterator iter = std::find( data.cbegin(), data.cend(), k );

        if ( iter == data.end() ) throw std::out_of_range("blah blah");

        return iter->second;
    }

    mapped_type &at(key_type const &k)
    {
        return const_cast<mapped_type&>( const_cast<fifo_map const *>(this)->at[k] );
    }

    typename decltype(data)::const_iterator cbegin(void) const { return data.cbegin(); }
    typename decltype(data)::const_iterator cend  (void) const { return data.cend  (); }

    typename decltype(data)::iterator begin(void) { return data.begin(); }
    typename decltype(data)::iterator end  (void) { return data.end  (); }
};

int main(void)
{
	fifo_map<int,int> obj;

	obj[5];
}
