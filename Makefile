CXX=g++


all: readdial

LANG=C
#BOOST=-lboost_thread
#CXXFLAGS=-static
#CXXFLAGS=-O3
#CXXFLAGS=-Wno-unused-local-typedefs -Wno-deprecated-copy

#LFLAGS=$(firstword $(wildcard $(HOME)/svn/REPMDPS/trunk/tests/libs_mdps/externals/vwd/bin_Linux*/vwd/libvwd.a)) -lboost_system
LFLAGS=

main.o: main.cxx soundcard2c.h  soundcard_indicator.h indicator.h clocktime.h

%.s: %.cxx Makefile
	$(CXX) -Wsign-promo -mcx16 -Wall -Wextra -g3 -ggdb3 -std=gnu++2a -pthread -I. $(CXXFLAGS) $< $(BOOST) -lpthread -lrt -ldl $(LFLAGS) -o $@ -S

%.o: %.cxx Makefile
	$(CXX) -Wsign-promo -mcx16 -Wall -Wextra -g3 -ggdb3 -std=gnu++2a -pthread -I. $(CXXFLAGS) -c $< $(BOOST) -o $@

readdial: main.o
	$(CXX) -lportaudio -lpthread -lboost_thread -lboost_program_options $< -o $@
