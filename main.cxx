
#include "soundcard_indicator.h"

#include "banner.h"


#include "boost/algorithm/string.hpp"

//#include "boost/units/scale.hpp"

#include "boost/function.hpp"
#include "boost/program_options.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <string>
#include <memory>
#include <vector>




#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>


void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}

class serialport
{
private:
	int fd = -1;

	int set_attributes( int speed )
	{
		struct termios tty;

		if (tcgetattr(fd, &tty) < 0) {
			printf("Error from tcgetattr: %s\n", strerror(errno));
			return -1;
		}

		cfsetospeed(&tty, (speed_t)speed);
		cfsetispeed(&tty, (speed_t)speed);

		tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
		tty.c_cflag &= ~CSIZE;
		tty.c_cflag |= CS8;         /* 8-bit characters */
		tty.c_cflag &= ~PARENB;     /* no parity bit */
		tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
		tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

		/* setup for non-canonical mode */
		tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		tty.c_oflag &= ~OPOST;

		/* fetch bytes as they become available */
		tty.c_cc[VMIN] = 1;
		tty.c_cc[VTIME] = 1;

		if (tcsetattr(fd, TCSANOW, &tty) != 0) {
			printf("Error from tcsetattr: %s\n", strerror(errno));
			return -1;
		}
		set_mincount( fd, 0 );
		return 0;
	}

public:
	serialport( const std::string& device ) :
		buffer(512)
	{
		fd = ::open(device.c_str(),O_RDWR | O_NOCTTY | O_SYNC );
		if( fd < 0 )
		{
			throw std::runtime_error ("Failed to open serial port " + device );
		}
		set_attributes( B115200);
	}
		
	~serialport()
	{
		if( fd >= 0 )
		{
			::close(fd);
		}
	}

	void write( std::string data )
	{
		if( data[data.size()-1] != '\n' )
		{
			data += "\n";
		}
		auto wlen = ::write( fd, data.data(), data.size() );
		if( size_t(wlen) != data.size() )
		{
			throw std::runtime_error("Could not write '" + data + "' to serial port" );
		}
	}

	std::vector<char> buffer;

	size_t buffer_level = 0;

	std::string readline( )
	{
		if( buffer_level == buffer.size() )
		{
			buffer.resize(buffer.size() * 1.5 );
		}
		auto nlpos = std::find( buffer.begin(), buffer.begin() + buffer_level, '\n' );
		if( nlpos == buffer.begin() + buffer_level )
		{
			size_t rd = ::read( fd, &buffer[buffer_level], buffer.size() - buffer_level );
			buffer_level += rd;
		}

//		std::cout << "After ::read() buffer level " << buffer_level << "\n";
//		memdump(std::cout,buffer.data(), buffer_level );

		nlpos = std::find( buffer.begin(), buffer.begin() + buffer_level, '\n' );
		if( nlpos == buffer.begin() + buffer_level )
		{
			return "";
		}
		std::string ret = { buffer.begin(), nlpos };
		std::copy( nlpos+1,buffer.end(), buffer.begin() );
		buffer_level -= ret.size();
		--buffer_level; // We also cut off the \n

//		std::cout << "buffer_level = " << buffer_level << "\n";
		
//		std::cout << "readline():\n";
//		memdump(std::cout,ret.data(),ret.size());
//		std::cout << "Remaining in buffer (" << buffer_level << ")\n";
//		memdump(std::cout,buffer.data(), buffer_level );

		return ret;
	}

	std::string readline_wait( )
	{
		std::string ret;
		do
		{
			ret = readline();
		} while( ret.empty() );

		return ret;
	}
};

class printer
{
private:
	serialport sp;

	std::map<std::string,float> steps_per_mm;

	void init( )
	{
		sp.write("M92");
		std::string line = sp.readline_wait();
		auto okline = sp.readline_wait();
		std::vector<std::string> vec;
		split( vec, line, " ", true );
		for( auto elem : vec )
		{
			if( elem == "M92" ) continue;
			if( elem == "echo:" ) continue;
			std::string axis ( 1, elem[0] );
			elem.erase(elem.begin());
			auto smm = std::stof( elem );
			steps_per_mm[axis] = smm;
			std::cout << axis << " : " << smm << " steps/mm\n";
		}
	}

public:
	printer ( const std::string& dev ) :
		sp(dev)
	{
		init();
	}

	std::string execute( const std::string& gcode )
	{
		std::string ret;
		sp.write(gcode);
		do
		{
			ret += sp.readline();
//			std::cout << "ret = " << ret << "\n";
			
		} while(ret.empty());

		std::string more;
		do
		{
			more = sp.readline();
			ret += more;
//			std::cout << "2ret = " << ret << "\n";
		} while( !more.empty() );

		return ret;
	}

	/**
	 * Goes to and waits...
	 */
	void go_to( const std::string& axis, float position_mm, size_t speed = 100 )
	{
		sp.write("G1 " + axis + std::to_string(position_mm) + " F" + std::to_string(speed) );
		std::string line = sp.readline_wait(); // The "ok" 

		float expected_steps = position_mm * steps_per_mm[axis];

		std::string exstr = axis + ":" + std::to_string(expected_steps);
		size_t repeats = 0;
		std::string lastline;

//		std::cout << "Expecting to find '" << exstr << "' somewhere in the answer\n";

		int cnt = 0;
		std::string position_line;
		std::string target_line;
		do
		{
			++cnt;
			sp.write("M114");

			position_line = sp.readline_wait();
//			std::cout << "\rposition_line = " << position_line << "     " << std::flush;
			target_line = sp.readline_wait();
//			std::cout << "target_line = " << target_line << "\n";
			auto okline = sp.readline_wait();
//			std::cout << "okline = " << okline << "\n";
			
			
			
			
			if( lastline == position_line )
			{
				++repeats;
			}
			else
			{
				repeats = 0;
			}
			lastline = position_line;
		}while( position_line.find(target_line) == std::string::npos || repeats > 2 );
//		std::cout << "\n";
//		std::cout << "repeats = " << repeats << "\n";
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

struct result
{
	float g_value;
	float measurement_sum = 0;
	std::vector<float> measured_values;
};

int plot_axis( const std::string& device, const std::string& gcode, const std::string& axis, float start, float end, float steps, bool bidirectional, size_t runs, float prepos, float speed, size_t stable )
{
	if( axis.size() != 1 )
	{
		throw std::runtime_error("Axis is expected to be a single character, not '" + axis + "'\n");
	}
	std::cout << "Initializing indicator...\n";
	soundcard_indicator zsi;
	zsi.start();
	std::cout << "Connecting to printer...\n";
	printer pr(device);

	if( !gcode.empty() )
	{
		std::cout << "Executing priming gcode '" << gcode << "'\n";
		auto exres = pr.execute(gcode);
		std::cout << "Printer responded with '" << exres << "'\n";
	}

	std::vector<result> readings;

	size_t cnt = 0;
	for( double zp = start; zp <= end; zp += steps )
	{
		++cnt;
	}

//	readings.resize( size_t(std::abs((end-start) / steps)+0.5) );
	readings.resize( cnt ); // Too late to do the proper math, let me do it tomorrow

	std::cout << "readings.size() = " << readings.size() << "\n";
	
	for (size_t run = 0; run < runs; ++run)
	{
		if( runs > 1 )
		{
			std::cout << "Run " << run+1 << "/" << runs << "\n";
		}
		std::cout << "Getting printer into start position\n";
		if( prepos >= 0 )
		{
			pr.go_to( axis, prepos, 100 );
		}
		pr.go_to(axis,start, 100 );
		auto r = zsi.get_stable_reading();

		zsi.zero();

		steps = std::abs(steps);
		if( start > end )
		{
			steps = -steps;
		}

		size_t ridx = 0;

		std::cout << "Driving measurement, please do not disturb the printer\n";
		
		for( double zp = start; zp <= end; zp += steps )
		{
			std::cout << "\rDriving to : " <<  zp*1000 << "   " << std::flush;
			pr.go_to( axis, zp, speed );

			if( stable )
			{
				r = zsi.get_stable_reading(stable);
			}
			else
			{
				r = zsi.get_next_reading();
			}
//			std::cout << "ridx = " << ridx << " ";
			
			std::cout << "measured " << r.length;
			std::cout << ", deviation " << (r.length.value() - zp*1000) << "µm        " << std::flush;
			readings[ridx].g_value = zp;
			readings[ridx].measurement_sum += r.length.value();
			readings[ridx].measured_values.push_back( r.length.value() );
			++ridx;
		}
		std::cout << "\n";
	}
	std::cout << "\n";

	std::ofstream aout { "axis_" + axis + ".dat" };

	aout << "# g_value avg ";
	for (size_t i = 0; i < runs; ++i)
	{
		aout << " run" << i;
	}
	aout << "\n";

	for( auto& res : readings )
	{
		auto gv = res.g_value * 1000;
		aout << gv << " " << res.measurement_sum/runs << " ";
		for( auto& mv : res.measured_values )
		{
			aout << mv << " ";
		}

		aout << ((res.measurement_sum/runs)-gv) << " ";
		for( auto& mv : res.measured_values )
		{
			aout << (mv-gv) << " ";
		}
		aout << "\n";
	}


	return 0;
}

int main(int argc, const char *argv[])
{
	bool banner_mode = false;
	std::string mode;
	size_t average = 0;
	std::string axis;
	float start = 0;
	float end = 0;
	float steps = 0;
	bool bidir = false;
	float preposition = -1;
	float speed = 0;
	size_t stable = 0;
	std::string device;
	std::string gcode;

	boost::program_options::options_description desc("Valid options");

	desc.add_options()
	("help,h", "produce this help message")
	("version,V", "Show the program version")

	("verbose,v",boost::program_options::value<std::vector<std::string> >(), "Increase the initial logging level (may be overridden by a config file)")
	("banner,b",boost::program_options::value<bool>(&banner_mode)->implicit_value(true), "Indicator output in banner mode")
	("mode,m",boost::program_options::value<std::string>(&mode), "Operating mode: plot" )
	("average,a",boost::program_options::value<size_t>(&average)->default_value(1), "Average over that many runs in plot mode")
	("axis,A", boost::program_options::value<std::string>(&axis), "The axis to use in plot mode")
	("start,s", boost::program_options::value<float>(&start)->default_value(0), "Start movement at this point (mm)" )
	("end,e", boost::program_options::value<float>(&end)->default_value(2), "End movement at this point (mm)" )
	("steps,S", boost::program_options::value<float>(&steps)->default_value(0.01), "The steps to be done per measurement (mm), the absolute value will be used in the proper direction")
	("bidirectional,B", boost::program_options::value<bool>(&bidir)->default_value(false)->implicit_value(true), "Do the movement in both directions")
	("preposition,p",boost::program_options::value<float>(&preposition), "Before driving to the start, drive to this position (simulates auto homing and hop movements)")
	("speed,F",boost::program_options::value<float>(&speed)->default_value(10),"The speed to move with, given to the F parameter of G1")
	("stable,r",boost::program_options::value<size_t>(&stable)->default_value(0), "For each reading wait to stabilize for that many readings first. Makes measurements slower, can prevent them totally if you have a too high value")
	("device,d",boost::program_options::value<std::string>(&device)->default_value("/dev/ttyACM0"), "The serial port device the printer is attached to")
	("gcode,g",boost::program_options::value<std::string>(&gcode), "Some gcode to execute before everyhting else" )
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

	if( mode == "plot" )
	{
		return plot_axis( device, gcode, axis, start, end, steps, bidir, average, preposition, speed, stable );
	}
	else if( !mode.empty() )
	{
		std::cout << "Unknown operation mode '" << mode << "'\n";
		return 5;
	}


	soundcard_indicator si;
	si.start();

	std::chrono::nanoseconds last = 0s;
	std::chrono::nanoseconds started = now();

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
				std::cout << (when-started).count() << "ns ";
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
