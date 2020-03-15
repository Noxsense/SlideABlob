#include "gui.h"
#include "test.h"

int main(int argc, char *argv[])
{
  return argc < 2
    ? start_window(5 , 5, 15 /*second per turn*/)
    : test_game(4, 4) && test_game(4, 5);
}
