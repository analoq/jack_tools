#include "BassSplittr.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <set>
#include <stdexcept>

using namespace std;

int main(int argc, char *argv[]) try
{
  BassSplittr bass_splittr {"bass_splittr"};
  bass_splittr.start();

  while ( true )
    this_thread::sleep_for(chrono::seconds(1));

  return 0;
}
catch(exception &e)
{
  cerr << "Error: " << e.what() << endl;
}
