

#ifndef __SCI_SOUNDCARD2C_H_
#define __SCI_SOUNDCARD2C_H_

#include <portaudio.h>
#include <iostream>
#include <iomanip>
#include <string>

#include "boost/function.hpp"




namespace sci
{
	
class soundcard2c
{
private:
	enum line_state 
	{
		low,
		high,
		highz
	};

	unsigned current_bit = 0;
	uint64_t current_value = 0;
	uint64_t zero_count = 0;

	line_state data_state = highz;
	line_state clock_state = highz;

	float last_data = 0;
	float last_clock = 0;

	float bitval = 0;
	int bitcount = 0;

	float high_limit = -0.25;
	float low_limit = -0.75;

	bool msb = true;

	void print_state( std::ostream& out, line_state state )
	{
		switch(state )
		{
			case high:
				out << 1;
				break;
			case low:
				out << -1;
				break;
			default:
				out << 0;
		}
	}

	void add_bit( line_state data_state )
	{
//		std::cout << "Adding data state " << current_bit << " : " << data_state << "\n";
		if( data_state == high )
		{
			if( msb )
			{
				current_value |= (1<<current_bit);
			}
			else
			{
				current_value <<= 1;
				current_value |= 1;
			}
	//		current_value |= (1<<(24-current_bit));
	//		std::cout << "current_value = " << current_value << "\n";
			
//			std::cout << "1";
		}
		else
		{
			if( !msb )
			{
				current_value <<= 1;
			}
//			std::cout << "0";
		}
		++current_bit;
	//	std::cout << "current_bit = " << current_bit << "\n";
		
//		if( current_bit > 24 )
//		{
//			std::cout << "\n";
//			current_bit = 0;
//			std::cout << current_value << "\n";
//			current_value = 0;
//		}
	}


	line_state calc_state( line_state old_state, float old_, float new_ )
	{
	//	if( std::abs(old_) < 0.01 && std::abs(new_) < 0.01 )
	//	{
	//		return highz;
	//	}

		if( new_ > high_limit )
		{
			return high;
		}

		if( new_ < low_limit )
		{
			return low;
		}

		return old_state;
	}



	template<bool swap_cd>
	static int readout_callback( const void *inputBuffer, void* /*outputBuffer*/, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/, void* userData )
	{
		soundcard2c* si = static_cast<soundcard2c*>(userData);
		return si->readout<swap_cd>( reinterpret_cast<const float*>(inputBuffer), framesPerBuffer );
	}
	
	template<bool swap_cd>
	int readout( const float* samples, size_t framesPerBuffer )
	{
	//	std::cout << "timeInfo->inputBufferAdcTime = " << timeInfo->inputBufferAdcTime << "\n";
		
	//	std::cout << "statusFlags = " << statusFlags << "\n";
		
//		std::cout << __PRETTY_FUNCTION__ << '\n';
//		const float* samples = reinterpret_cast<const float*>(inputBuffer);


//		uint32_t cnt = 0;
//		for (size_t i = 0; i < framesPerBuffer*2; ++i)
//		{
//			auto sample = samples[i];
//			if( sample < -0.25 )
//			{
//				++cnt;
//			}
//		}

//		if( cnt < 0.1 * framesPerBuffer )
//		{
	//		std::cout << "(double(cnt)/framesPerBuffer) = " << (double(cnt)/framesPerBuffer) << "\n";
	//		
	//		current_bit = 0;
	//		current_value = 0;
	//		std::cout << "R\n";
	//		
	//		return paContinue;
//		}

	//	std::ofstream dout { "audio.dat" };


		for (size_t i = 0; i < framesPerBuffer*2; i+= 2 )
		{
			float data;
			float clock;
			if( swap_cd )
			{
				data = samples[i+1];
				clock = samples[i];
			}
			else
			{
				data = samples[i];
				clock = samples[i+1];
			}

			data_state = calc_state( data_state, last_data, data );
			auto nclock = calc_state( clock_state, last_clock,clock );

			++zero_count;
			if( nclock != clock_state )
			{
				zero_count = 0;
				clock_state = nclock;
				if( clock_state == high )
				{
					add_bit( data_state );
//					bitcount = 5;
//					if( data_state == high )
//					{
//						bitval = 0.5;
//					}
//					else
//					{
//						bitval = -0.5;
//					}
				}
			}

//			if( bitcount )
//			{
//				--bitcount;
//			}
//			else
//			{
//				bitval = 0;
//			}


	//		dout << data << " " << clock << " ";
	//		print_state(dout,data_state);
	//		dout << " ";
	//		print_state(dout,clock_state);
	//		dout << " ";
	//		dout << bitval;
	//		dout << " ";

	//		dout << "\n";

			last_data = data;
			last_clock = clock;

	//		std::cout << "data = " << data << "\n";
			
			if( zero_count > 1000 )
			{
				static uint32_t lost_measurements = 0;
//				std::cout << "current_bit = " << current_bit << "\n";
				
//				char neg = ' ';
				if( current_bit != 0 )
				{
//					std::cout << " ";
//					if( current_value & 0x100000 )
//					{
//						std::cout << "-";
//						neg = '-';
//					}
//					std::cout << __PRETTY_FUNCTION__ << '\n';
					callback(current_value);
//					current_value &= 0xFFFFF;
//					std::cout << current_value;
//					std::cout << " [" << lost_measurements << "]";
//					std::cout << "                  ";

//					static std::chrono::nanoseconds start = now();
//					auto t = now() - start;
//					dro << t.count()/1'000'000'000.0 << " ";
//					dro << neg << current_value << std::endl;
				}
//				std::cout << std::flush;
				if( current_bit == 24 )
				{
					lost_measurements = 0;
	//				std::cout << std::endl;
//					std::cout << "\r" << std::flush;
				}
				else
				{
					if( current_bit != 0 )
					{
						++lost_measurements;
					}
//					std::cout << "\r" << std::flush;
				}
	//			std::cout << "\nRESET\n";
	//			std::cout << "current_bit = " << current_bit << "\n";
				
	//			std::cout << "current_value = " << current_value << "\n";
				
				current_bit = 0;
				current_value = 0;
				zero_count = 0;

	//			return paComplete;
			}
		}
	//	return paComplete;
		return paContinue;
	}

	static int volume_callback( const void *inputBuffer, void* /*outputBuffer*/, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/, void *userData )
	{
		soundcard2c* si = static_cast<soundcard2c*>(userData);
		return si->volume( reinterpret_cast<const float*>(inputBuffer), framesPerBuffer );
	}

	int volume( const float* samples, unsigned long framesPerBuffer )
	{
	//	std::cout << __PRETTY_FUNCTION__ << '\n';
//		const float* samples = reinterpret_cast<const float*>(inputBuffer);

		float sum = 0;
		float min = 10;
		float max = -10;

		for (size_t i = 0; i < framesPerBuffer*2; ++i)
		{
			min = std::min(min,samples[i]);
			max = std::max(max,samples[i]);
			sum += samples[i];
		}
		float avg = sum/framesPerBuffer;
		std::cout << "\r";

		auto minok = []( float min ) { return min < -0.9;  };
		auto avgok = []( float avg ) { return std::abs(avg) < 0.2; };
		auto maxok = []( float max ) { return max > 0 && max < 0.2; };

		if( minok(min) && avgok(avg) && maxok(max) )
		{
			std::cout << "[GOOD]";
		}
		else
		{
			std::cout << "[BAD ] ";
		}

		std::cout << std::fixed;

		if( minok(min ) )
		{
			std::cout << " ";
		}
		else
		{
			std::cout << "*";
		}
		std::cout << std::setw(7) << std::setprecision(4) << min;
		std::cout << "|";


		if( avgok(avg ) )
		{
			std::cout << " ";
		}
		else
		{
			std::cout << "*";
		}
		std::cout << std::setw(7) << std::setprecision(4) << avg;
		std::cout << "|";


		if( maxok(max ) )
		{
			std::cout << " ";
		}
		else
		{
			std::cout << "*";
		}
		std::cout << std::setw(7) << std::setprecision(4) << max;
		std::cout << "|";

		std::cout << std::flush;

		return paContinue;
	}

	int device_index = 0;
	bool initialized = false;
	size_t sample_rate;
	bool swap_cd = false;

	boost::function<void(uint64_t)> callback;
public:
	soundcard2c( const boost::function<void(uint64_t)>& callback_, int dev = -1, size_t sample_rate_ = 96000, bool swap_cd_ = false) :
		device_index(dev),
		sample_rate(sample_rate_),
		swap_cd(swap_cd_),
		callback(callback_)
	{
	}

	~soundcard2c( )
	{
		if( initialized )
		{
			Pa_Terminate();
		}
	}

	void pa_info( std::ostream& out )
	{
		out << "PA " << Pa_GetVersionInfo()->versionText << " is running...\n";

		auto numdev = Pa_GetDeviceCount();
		out << "Found " << numdev << " devices\n";
		auto defin = device_index;
		if( defin == -1 )
		{
			defin = Pa_GetDefaultInputDevice();
		}

		for (size_t i = 0; i < numdev; ++i)
		{
			auto devinfo = Pa_GetDeviceInfo( i );
			if( i == defin )
			{
				out << "*";
			}
			else
			{
				out << " ";
			}
			out << "[" << i << "] : " << Pa_GetHostApiInfo(devinfo->hostApi)->name << " ";
			out << devinfo->name << "\n";
		}

	}

	PaStream* instream = nullptr;
	PaStreamParameters indev;

	bool init( bool volume = false )
	{
		auto err = Pa_Initialize();
		if( err != paNoError )
		{
			std::cerr << "Failed to initialze PA : " << err << "\n";
			return false;
		}
		initialized = true;

		pa_info(std::cout);

		auto defin = Pa_GetDefaultInputDevice();

		indev.device = defin;
		indev.channelCount = 2;
		indev.sampleFormat = paFloat32;
		indev.suggestedLatency = Pa_GetDeviceInfo( indev.device )->defaultLowInputLatency;
		indev.hostApiSpecificStreamInfo = nullptr;


		if( volume )
		{
			err = Pa_OpenStream( &instream, &indev, nullptr, 96000, 16*1024, paClipOff, volume_callback, this);
		}
		else
		{
			if( swap_cd )
			{
				err = Pa_OpenStream( &instream, &indev, nullptr, sample_rate, 4096, paClipOff, readout_callback<true>, this);
			}
			else
			{
				err = Pa_OpenStream( &instream, &indev, nullptr, sample_rate, 4096, paClipOff, readout_callback<false>, this);
			}
		}

		if( err != paNoError )
		{
			std::cerr << "Failed to open stream: " << err << "\n";
			return false;
		}

		err = Pa_StartStream( instream );

		if( err != paNoError )
		{
			std::cerr << "Failed to start recording: " << err << "\n";
			return false;
		}
		return true;
	}

	void run( )
	{

//		std::cout << "Stream started..." << std::endl;
		int err = 1;
		while( err == 1 )
		{
			err = run_once();
		}
		std::cout << "err = " << err << "\n";
	}

	int run_once( )
	{
		Pa_Sleep(1000);
		return Pa_IsStreamActive(instream);
	}

};

} // of namespace si

#endif // __SCI_SOUNDCARD2C_H_
// vim: tabstop=4 shiftwidth=4 noexpandtab
