
#ifndef __SCI_INDICATOR_H_
#define __SCI_INDICATOR_H_


#include "boost/units/systems/si.hpp"
#include "boost/units/systems/si/prefixes.hpp"
#include "boost/units/systems/si/io.hpp"
#include "boost/units/io.hpp"

#include <chrono>
#include <string>
#include <iostream>

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
//		std::cout << "tmp.length = " << tmp.length << "\n";
		
		return { tmp.length - zero_offset, tmp.read_at };
	}

	virtual reading get_next_reading( ) const
	{
		auto current = get_reading();
		auto ret = current;
		do
		{
			usleep(100);
			ret = get_reading();
		} while( current.read_at == ret.read_at );
		return ret;
	}

	/**
	 * Sometimes the timing is a bit off, get a reading that is the same for N consecutive runs
	 */
	virtual reading get_stable_reading( size_t n = 2 ) const
	{
		auto first = get_reading();
		auto last = first;
		size_t sames = 0;
		do
		{
			last = get_next_reading();
			if( last.length == first.length )
			{
				++sames;
			}
			else
			{
				sames = 0;
				first = last;
			}
		} while( sames < n );
		return first;
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
