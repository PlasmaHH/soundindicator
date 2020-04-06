#ifndef __SCI_SOUNDCARD_INDICATOR_H_
#define __SCI_SOUNDCARD_INDICATOR_H_

#include "indicator.h"
#include "soundcard2c.h"
#include "clocktime.h"

#include "boost/thread.hpp"

#include <memory>

class soundcard_indicator :
	public indicator
{
private:
protected:
	reading current_raw_reading;

	sci::soundcard2c s2c;

	std::unique_ptr<boost::thread> thread;

	void run( )
	{
		s2c.run();
	}
public:
	soundcard_indicator( ) :
		s2c( [this](uint64_t value)
				{
					reading r;
					int64_t len = value & 0xFFFFF;
					if( value & 0x100000 )
					{
						len *= -1;
					}
//					std::cout << "len = " << len << "\n";
					
//					r.length = len * si::nano * si::meter;
					auto rlen = len * boost::units::si::micro * boost::units::si::meter;
//					rlen.foo();
//					r.length.foo();

//					std::cout << "rlen = " << rlen << "\n";
					
					r.length = rlen;
					r.read_at = now();
					current_raw_reading.set(r);
//					std::cout << "r.length = " << r.length << "\n";
//					std::cout << "r.length = " << boost::units::engineering_prefix << r.length << "\n";
					
				},-1,96000,true )
	{
	}

	virtual reading raw_reading( ) const
	{
		auto ret = current_raw_reading.get();
		return ret;
	}

	void start( )
	{
		s2c.init();
		thread.reset( new boost::thread( &soundcard_indicator::run, this ) );
	}

};
#endif // __SCI_SOUNDCARD_INDICATOR_H_
// vim: tabstop=4 shiftwidth=4 noexpandtab
