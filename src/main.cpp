#include <iostream>

#include "gui.h"
#include "test.h"

bool pass_tests(bool verbose = true)
{
  bool passed_all = true;

  for (int r = 4; r < 10; r++)
  {
    for (int c = 4; c < 10; c++)
    {
      passed_all &= test_field(r, c, 7, verbose);
    }
  }

  return passed_all;
}

int main(int argc, char *argv[])
{
  if (!pass_tests(argc > 1))  // tests, always, verbose, if not with window.
  {
    std::cerr << "Test(s) failed. (exit(1))" << std::endl;
    return 1;
  }

  return argc < 2 ? start_window(5 , 5, 15 /*second per turn*/) : 0;
}
