
#ifndef __SCI_STRINGUTIL_H_
#define __SCI_STRINGUTIL_H_

#include <string>
#include <ostream>

template<class V>
void split( V& v, const std::string& s, const std::string& d, bool collapse = false )
{
	if( s.empty() )
	{
		return;
	}
	size_t oldpos = 0;
	size_t pos = 0;
	do
	{
		pos = s.find(d,oldpos);
		if( !collapse || (pos-oldpos) > 0 )
		{
			std::string part = s.substr(oldpos,pos-oldpos);
			v.insert(v.end(),part);
		}
		oldpos = pos + d.size();
	} while ( pos != std::string::npos );
}







void do_memdump( std::ostream& o, const void* data, size_t len, uint64_t offset )
{
	const unsigned char* ptr = static_cast<const unsigned char*>(data);

	for( size_t i = 0; i < len; i += 16 )
	{
		o << std::noshowbase;
		o << std::setw(8);
		o << std::setfill('0');
		o << std::hex << i+offset << ' ';
		size_t to = std::min(len,i+16);
		for( size_t j = i; j < to; ++j )
		{
			o << ' ';
			o << std::setw(2);
			o << std::setfill('0');
			o << std::hex;
			o << (unsigned)ptr[j];

			if( (j+1) % 8 == 0 )
			{
				o << ' ';
			}
		}
		o << "  ";
		for( size_t j = to; j < i+16; ++j )
		{
			o << "   ";
			if( (j+1) % 8 == 0 )
			{
				o << ' ';
			}
		}

		for( size_t j = i; j < to; ++j )
		{
			const unsigned char c = ptr[j];
			if(isprint(c) && c != '\n' && c != '\t' )
			{
				o << ptr[j];
			}
			else
			{
				o << '.';
			}
			if( (j+1) % 8 == 0 )
			{
				o << ' ';
			}
		}
		o << '\n';
	}
	o << std::dec;
}









void memdump( std::ostream& o, const void* data, size_t len, const void* offset )
{
	do_memdump(o,data,len,(uint64_t)offset);
}

void memdump( std::ostream& o, const void* data, size_t len )
{
	do_memdump(o,data,len,0);
}



#endif // __SCI_STRINGUTIL_H_
// vim: tabstop=4 shiftwidth=4 noexpandtab
