#ifndef __SCI_CLOCKTIME_H_
#define __SCI_CLOCKTIME_H_

#include <chrono>
#include <ctime>

using namespace std::chrono_literals;

std::chrono::nanoseconds now( )
{
	struct timespec ts;
	clock_gettime( CLOCK_REALTIME, &ts );

	return 1ns * (ts.tv_sec * 1'000'000'000 + ts.tv_nsec);
}

#endif // __SCI_CLOCKTIME_H_
// vim: tabstop=4 shiftwidth=4 noexpandtab
