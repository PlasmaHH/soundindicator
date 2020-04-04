
#include "soundcard2c.h"
#include "banner.h"

#include "boost/units/systems/si.hpp"
#include "boost/units/systems/si/prefixes.hpp"
#include "boost/units/systems/si/io.hpp"
#include "boost/units/io.hpp"

#include "boost/algorithm/string.hpp"

//#include "boost/units/scale.hpp"

#include "boost/function.hpp"
#include "boost/thread.hpp"
#include "boost/program_options.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <string>
#include <memory>
#include <vector>


using namespace std::chrono_literals;

//using namespace boost::units;
//using namespace boost::units::si;

std::chrono::nanoseconds now( )
{
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );

	return 1ns * (ts.tv_sec * 1'000'000'000 + ts.tv_nsec);
}

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

static int verbosity = 0;
std::pair<std::string,std::string> verbosity_parser( const std::string& s )
{
	if( s == "--verbose" || s == "-v" )
	{
		verbosity++;
		return { "verbose", "." };
	}
	return { "", "" };
}

int main(int argc, const char *argv[])
{
//	std::cout << "\033[2J";
//	banner banx {120};
//	banx.write("-0.1234589um");
//	banx.flush();
//
//	return 5;
	bool banner_mode = false;

	boost::program_options::options_description desc("Valid options");

	desc.add_options()
	("help,h", "produce this help message")
	("version,V", "Show the program version")

	("verbose,v",boost::program_options::value<std::vector<std::string> >(), "Increase the initial logging level (may be overridden by a config file)")
	("banner,b",boost::program_options::value<bool>(&banner_mode)->implicit_value(true), "Indicator output in banner mode")
	;


	boost::program_options::variables_map vm;


	const boost::program_options::positional_options_description p; //make sure that unknown options throw
	boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).positional(p).extra_parser(verbosity_parser).run(), vm);
	boost::program_options::notify(vm);

	if( vm.count("help") )
	{
		std::cout << desc << "\n";
		return 0;
	}

	if( vm.count("verbose") )
	{
	}



	soundcard_indicator si;
	si.start();

	std::chrono::nanoseconds last = 0s;
	std::chrono::nanoseconds start = now();

	std::cout << "\033[2J";

	banner ban {120};
	std::ostringstream os;
	while(true)
	{
		if( banner_mode )
		{
			os.str("");
			os.clear();
		}
		auto [len,when] = si.get_reading();
		if( last != when )
		{
			if( banner_mode )
			{
					
				os  << std::setw(6);
				os << len;
				std::string lens = os.str();
				boost::replace_all(lens,"µ","u");

				ban.write( lens );
				os.str("");
				os << when.count() << "ns";
				ban.put_at(0,0,os.str());
				ban.flush();
			}
			else
			{
				std::cout << "\033[0;0H";
				std::cout << "\r";
				std::cout << (when-start).count() << "ns ";
				std::cout << len;
				std::cout << "               ";
				std::cout << std::flush;
			}
			last = when;
		}
		usleep(1000);
	}
	return -1;
}

// vim: tabstop=4 shiftwidth=4 noexpandtab
