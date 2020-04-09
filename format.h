#ifndef __SCI_FORMAT_H_
#define __SCI_FORMAT_H_

#include "boost/format.hpp"
#include <string>

namespace sci
{

namespace __detail
{
	template<class T>
	void format( boost::format& bf, const T& t )
	{
		bf % t;
	}

	template<class T0, class... T>
	void format( boost::format& bf, const T0& t0, const T&... t )
	{
		bf % t0;
		format(bf,t...);
	}

} // of namespace __detail

template<class... T>
std::string format( const char* fmt, const T&... t )
{
	boost::format bf(fmt);
	__detail::format(bf,t...);
	return bf.str();
}


template<class... T>
std::string format( const std::string& fmt, const T&... t )
{
	boost::format bf(fmt);
	__detail::format(bf,t...);
	return bf.str();
}

}
#endif
// vim: tabstop=4 shiftwidth=4 noexpandtab
