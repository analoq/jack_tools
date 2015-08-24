all: jack_transposr jack_basstransposr jack_splittr jack_basssplittr

jack_transposr: transposr/main.cpp transposr/Transposr.hpp
	g++ -std=c++11 transposr/main.cpp -ljack -o jack_transposr

jack_basstransposr: basstransposr/main.cpp basstransposr/BassTransposr.hpp
	g++ -std=c++11 basstransposr/main.cpp -ljack -o jack_basstransposr

jack_splittr: splittr/main.cpp splittr/Splittr.hpp
	g++ -std=c++11 splittr/main.cpp -ljack -o jack_splittr

jack_basssplittr: basssplittr/main.cpp basssplittr/BassSplittr.hpp
	g++ -std=c++11 basssplittr/main.cpp -ljack -o jack_basssplittr
