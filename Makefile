all: jack_transposr jack_splittr

jack_transposr: transposr/main.cpp transposr/Transposr.hpp
	g++ -std=c++11 transposr/main.cpp -ljack -o jack_transposr

jack_splittr: splittr/main.cpp splittr/Splittr.hpp
	g++ -std=c++11 splittr/main.cpp -ljack -o jack_splittr
