#include "BassTransposr.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <stdexcept>

using namespace std;

int main(int argc, char *argv[]) try
{
  if ( argc != 2 )
    throw runtime_error{"Insufficient arguments"};

  uint8_t reference_note = atoi(argv[1]);

  BassTransposr transposr {"basstransposr", reference_note};
  transposr.start();

  while ( true )
    this_thread::sleep_for(chrono::seconds(1));

  return 0;
}
catch(exception &e)
{
  cerr << "Error: " << e.what() << endl;
}
