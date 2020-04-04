
#ifndef __SCI_BANNER_H_
#define __SCI_BANNER_H_

#include <string>
#include <vector>
#include <ostream>
#include <map>

std::string char_0 = R"(
          
          
          
  .####.  
  ######  
 :##  ##: 
 ##:  :## 
 ##    ## 
 ## ## ## 
 ## ## ## 
 ##    ## 
 ##:  :## 
 :##  ##: 
  ######  
  .####.  
          
          
          
)";

std::string char_1 = R"(
          
          
          
  .###    
  ####    
  #:##    
    ##    
    ##    
    ##    
    ##    
    ##    
    ##    
    ##    
 ######## 
 ######## 
          
          
          
)";

std::string char_2 = R"(
          
          
          
 . ####:  
 #######: 
 #:.   ## 
       ## 
      :#  
      ##  
    .##:  
   .##:   
  :##:    
 :##:     
 ######## 
 ######## 
          
          
          
)";

std::string char_3 = R"(
          
          
          
 . ####:  
 #######: 
 #:.   ## 
       ## 
       ## 
   #####  
   #####. 
       ## 
       ## 
 #:    ## 
 #######: 
 :#####:  
          
          
          
)";
std::string char_4 = R"(
          
          
          
     ###  
    :###  
   .####  
   ##.##  
  :#: ##  
 .##  ##  
 ##   ##  
 ######## 
 ######## 
      ##  
      ##  
      ##  
          
          
          
)";
std::string char_5 = R"(
          
          
          
 #######  
 #######  
 ##       
 ##       
 ##### .  
 #######. 
 #:  .### 
       ## 
       ## 
 #:  .### 
 #######. 
 :#### .  
          
          
          
)";
std::string char_6 = R"(
          
          
          
    ###:  
  ######  
 :##. .#  
 ##:      
 ##:###:  
 #######: 
 ##    ## 
 ##    ## 
 ##    ## 
  #    ## 
  ######: 
  .####:  
          
          
          
)";
std::string char_7 = R"(
          
          
          
 ######## 
 ######## 
       #  
      ##. 
      ##  
     ##.  
    :##   
    ##:   
   :##    
   ##:    
  :##     
  ##:     
          
          
          
)";
std::string char_8 = R"(
          
          
          
  :####:  
 :######: 
 ##    ## 
 ##    ## 
 ##    ## 
  ######  
 .######. 
 ##    ## 
 ##    ## 
 ##    ## 
 :######: 
  :####:  
          
          
          
)";
std::string char_9 = R"(
          
          
          
  :####.  
 :######  
 ##    #  
 ##    ## 
 ##    ## 
 ##    ## 
 :####### 
  :###:## 
      :## 
  #. .##: 
  ######  
  :###    
          
          
          
)";

std::string char_micro = R"(
          
          
          
          
          
          
 ##    ## 
 ##    ## 
 ##    ## 
 ##    ## 
 ##    ## 
 ##    ## 
 ##   :## 
 #########
 ## ## .##
 ##       
 ##       
 ##       
)";

std::string char_m = R"(
          
          
          
          
          
          
 ## #:##: 
 ######## 
 ##.##.## 
 ## ## ## 
 ## ## ## 
 ## ## ## 
 ## ## ## 
 ## ## ## 
 ## ## ## 
          
          
          
)";

std::string char_dot = R"(
          
          
          
          
          
          
          
          
          
          
          
          
    ##    
    ##    
    ##    
          
          
          
)";
std::string char_dash = R"(
          
          
          
         
          
          
          
          
  #####   
  #####   
          
          
          
          
          
          
          
          
)";

std::string char_space = R"(
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
)";

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

class banner
{
private:
	static const size_t character_width = 10;

	std::map<char,std::vector<std::string> > mappings;

	std::vector<std::string> convert( const std::string& dchar )
	{
		std::vector<std::string> ret;
		
		split(ret,dchar,"\n",true);
		assert(ret.size() == 19);
//		std::cout << "ret.size() = " << ret.size() << "\n";
		return ret;
	}

	void init_mappings( )
	{
		mappings['0'] = convert(char_0);
		mappings['1'] = convert(char_1);
		mappings['2'] = convert(char_2);
		mappings['3'] = convert(char_3);
		mappings['4'] = convert(char_4);
		mappings['5'] = convert(char_5);
		mappings['6'] = convert(char_6);
		mappings['7'] = convert(char_7);
		mappings['8'] = convert(char_8);
		mappings['9'] = convert(char_9);
		mappings['u'] = convert(char_micro);
		mappings['m'] = convert(char_m);
		mappings['.'] = convert(char_dot);
		mappings['-'] = convert(char_dash);
		mappings[' '] = convert(char_space);
	}

	std::vector<std::string> buffer;

	const size_t max_index;
	size_t char_index = 0;

	void write_single( char c )
	{
		if( char_index >= max_index )
		{
			return;
		}
		size_t buf_idx = char_index * character_width;
//		std::cout << "'" << c <<"'" << std::endl;
		auto& chr = mappings.at(c);

		for (size_t i = 0; i < chr.size(); ++i)
		{
			auto& cs = chr[i];

			for (size_t j = 0; j < cs.size(); ++j)
			{
				buffer[i][j+buf_idx] = cs[j];
			}
		}
		++char_index;
	}
public:
	banner( size_t width ) :
		buffer(19),
		max_index(width/character_width)
	{
		init_mappings();
		for( auto& buf : buffer )
		{
			buf.resize(width,' ');
		}
	}

	void write( const std::string& s )
	{
		for( auto& c : s )
		{
			write_single(c);
		}
	}

	void put_at( size_t x, size_t y, const std::string& s )
	{
		if( y < buffer.size() )
		{
			auto& bufs = buffer[y];
			if( x < bufs.size() )
			{
				size_t rep = std::min( bufs.size() - x, s.size() );
//				std::cout << "bufs = " << bufs << "\n";
//				std::cout << "bufs.size() = " << bufs.size() << "\n";
				
//				std::cout << "x = " << x << "\n";
//				std::cout << "rep = " << rep << "\n";
//				std::cout << "s.size() = " << s.size() << "\n";
				
				
				
				bufs.replace( x, rep, s, 0,rep );
//				std::cout << "bufs = " << bufs << "\n";
//				std::cout << "bufs.size() = " << bufs.size() << "\n";
			}
		}
	}

	void flush( )
	{
		std::cout << "\033[0;0H";
		
		for( auto& buf : buffer )
		{
			std::cout << buf << "\n";
		}
		std::cout << std::flush;
		char_index = 0;
	}
};

#endif // __VWD_BANNER_H_
// vim: tabstop=4 shiftwidth=4 noexpandtab
