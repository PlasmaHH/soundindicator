
#ifndef __SCI_INDICATOR_H_
#define __SCI_INDICATOR_H_


#include "boost/units/systems/si.hpp"
#include "boost/units/systems/si/prefixes.hpp"
#include "boost/units/systems/si/io.hpp"
#include "boost/units/io.hpp"

#include <chrono>
#include <string>

class indicator
{
public:
//	typedef quantity<length,uint64_t> length_type;
	typedef decltype(boost::units::si::micro * boost::units::si::meter * 1ll) length_type;

	struct reading
	{
		length_type length;
		std::chrono::nanoseconds read_at;

		void set( const reading& nrd )
		{
			reading tmp;
			*(__int128_t*)&tmp= __sync_add_and_fetch( (__int128_t*)this, 0 );
			while( !__sync_bool_compare_and_swap( (__int128_t*)this,*(const __int128_t*)&tmp, *(const __int128_t*)&nrd) );
		}

		reading get( ) const
		{
			reading ret;
			*(__int128_t*)&ret = __sync_add_and_fetch( (__int128_t*)this, 0 );
			return ret;
		}

	} __attribute__((aligned(16)));
	static_assert(sizeof(reading) == 16, "Otherwise the xchhg thing doesn't work");
private:
protected:
	std::string name;

	length_type zero_offset;

public:
	indicator( const std::string& name_ = "<unnamed>" ) :
		name(name_)
	{
	}

	/**
	 * Gives back the reading as shown on the device itself
	 */
	virtual reading raw_reading( ) const = 0;

	/**
	 * Shows the reading including any internal zeroing
	 */
	virtual reading get_reading( ) const
	{
		auto tmp = raw_reading();
		return { tmp.length - zero_offset, tmp.read_at };
	}

	/**
	 * Zeroes the indicator, gives back the last get_reading() before it
	 */
	virtual reading zero( )
	{
		auto ret = get_reading();
		zero_offset = raw_reading().length;
		return ret;
	}

};

#endif // __SCI_INDICATOR_H_
// vim: tabstop=4 shiftwidth=4 noexpandtab
