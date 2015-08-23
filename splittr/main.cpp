#include "Splittr.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <set>
#include <stdexcept>

using namespace std;

int main(int argc, char *argv[]) try
{
  if ( argc < 2 )
    throw runtime_error{"Insufficient arguments"};

  set<uint8_t> keys;

  for (int i = 1; i < argc; i ++)
    keys.emplace(atoi(argv[i]));

  Splittr splittr {"splittr", keys};
  splittr.start();

  while ( true )
    this_thread::sleep_for(chrono::seconds(1));

  return 0;
}
catch(exception &e)
{
  cerr << "Error: " << e.what() << endl;
}
