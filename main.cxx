
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
	{}

	void write( std::string data )
	{
		if( data[data.size()-1] != '\n' )
		{
			data += "\n";
		}
		auto wlen = ::write( fd, data.data(), data.size() );
		if( wlen != data.size() )
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
		size_t rd = ::read( fd, &buffer[buffer_level], buffer.size() - buffer_level );
		buffer_level += rd;

		auto nlpos = std::find( buffer.begin(), buffer.begin() + buffer_level, '\n' );
		if( nlpos == buffer.begin() + buffer_level )
		{
			return "";
		}
		std::string ret = { buffer.begin(), nlpos };
		std::copy( nlpos+1,buffer.end(), buffer.begin() );
		buffer_level -= ret.size();

		return ret;
	}
};



using namespace std::chrono_literals;

//using namespace boost::units;
//using namespace boost::units::si;

std::chrono::nanoseconds now( )
{
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );

	return 1ns * (ts.tv_sec * 1'000'000'000 + ts.tv_nsec);
}



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
	serialport sp("/dev/ttyACM0");
	sp.write("G1 Z2 F0");
	sp.write("M114");
	for (size_t i = 0; i < 100; ++i)
	{
		std::string line = sp.readline();
		std::cout << "line = " << line << "\n";
		if( line.empty() ) continue;
		if( line == "ok" )
		{
			sp.write("M114");
		}
		else
		{
			break;
		}
	}
	return 4;
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
