#ifndef _TESTS_H_
#define _TESTS_H_

#include<iostream>
#include<string>
#include<vector>

#include "game.h"

std::string game_to_string(Game *g)
{
  int colour = 0, row_count = 0;

  std::string str = "", top = "", row = "";

  top = "";
  for (int i = 0; i < 2*g->get_cols(); i ++)
  {
    if (i == g->get_cols()) top += "\n";
    top += i < g->get_cols() ? " " + std::to_string(i) : " \u2193";
  }
  top += "\n";

  for (int i = 0; i < g->get_size(); i ++)
  {
    colour = g->colour_at(i);
    row += colour ? std::to_string(colour) : " ";

    // next position.
    if (i > 0 && (i % g->get_cols()) == g->get_cols() - 1)
    {
      // append above.
      str = "\u2192" + row + "\u2190 " + std::to_string(row_count++) + "\n" + str;
      row = "";
    }
    else
    {
      row += " ";
    }
  }

  return top + str;
}

bool test_game(int rows, int cols, int field_variety = 7)
{

  if (rows < 3 || cols < 3)
  {
    std::cout
      << "Too small for valid playing field, at least 4x4!" << std::endl;
    return 1;
  }

  int colour = 0, index = 0;
  int passed_tests = 0, summed_tests = 0;
  bool passed;
  std::vector<int> custom_fields, old;
  std::vector<FieldPattern> *winning_region;

  Game game(rows, cols);  // classical 4x4

  // --- Start with size == rows * cols

  std::cout
    << "Starting game(" << (rows) << "," << (cols) << "). "
    << "Test if size() == " << (rows*cols) << std::endl;

  passed = rows*cols == game.get_size();
  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  game.start();

  // --- Start randomized, everyone is not empty.

  std::cout << "## Staring game: Every field is not empty." << std::endl;

  passed = true;
  for (int i = 0; i < game.get_size(); i++)
  {
    colour = game.colour_at(i);
    passed &= colour != 0;
    custom_fields.push_back(colour);
  }

  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Fill customized

  game.start();  // again randomized.

  std::cout
    << "## Starting game with custom fields is filling correctly."
    << std::endl;

  game.start(custom_fields);

  passed = true;
  std::cout << "custom: [ ";
  for (int i = 0; i < game.get_size(); i++)
  {
    passed &= game.colour_at(i) == custom_fields[i];
    std::cout << custom_fields[i] << " ";
  }
  std::cout << "]" << std::endl;

  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Fill with empty cell

  std::cout
    << "## Create a field with empty cell in first (lower) row."
    << std::endl;

  index = rand() % game.get_rows();  // row[0]
  custom_fields[index] = 0;
  game.start(custom_fields);

  passed = game.colour_at(index) == 0;
  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Fix Gravity
  std::cout << "## Fix Gravity." << std::endl;

  passed = game.colour_at(index) == 0;

  old.clear();
  std::cout << " .. will fall down: [ ";
  for (int row = 1, tmp = 0; row < rows; row++)
  {
    tmp = game.colour_at(row, index);
    old.push_back(tmp);
    std::cout << tmp << " ";
  }
  std::cout << "]" << std::endl;

  game.fix_gavity();

  passed = true;
  old.push_back(0);  // top of this column should be empty.
  for (int row = 0; row < rows; row++)
  {
    passed &= game.colour_at(row, index) == old[row];
  }

  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Slide in LEFT <-

  for (index = 0; index < rows; index++)
  {
    colour = rand() % field_variety + 1;

    std::cout << "## Slide (left), row " << index << " ::  "
      << "[ " << colour << " | ";

    // old row
    old.clear();
    for (int col = 0, tmp = 0; col < cols; col++)
    {
      tmp = game.colour_at(index, col);
      old.push_back(tmp);
      std::cout << tmp << " ";
    }
    std::cout << "]" << std::endl;

    game.insert(index, colour);

    // check if everything is pushed to the right (+1)
    passed = true;
    old.insert(old.begin(), colour);  // insert in front.
    for (int col = 0, filled_gap = 0; passed && col < cols; col++)
    {
      filled_gap += old[col] == 0;
      passed &= game.colour_at(index, col) == old[col + filled_gap];
    }

    std::cout << "[" << (summed_tests++) << "] ";
    std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
    passed_tests += passed;
    std::cout << game_to_string(&game) << std::endl
      << "-------" << std::endl << std::endl;
  }

  // --- Slide in RIGHT

  for (index = 0; index < rows; index++)
  {
    colour = rand() % field_variety + 1;

    std::cout << "## Slide (right), row " << index << " ::  "
    << "[ ";

    // old row
    old.clear();
    for (int col = 0, tmp = 0; col < cols; col++)
    {
      tmp = game.colour_at(index, col);
      old.push_back(tmp);
      std::cout << tmp << " ";
    }
    std::cout << "| " << colour << " ]" << std::endl;

    game.insert(2*rows + cols - 1 - index, colour);

    // check if everything is pushed to the right (+1)
    passed = true;
    old.push_back(colour);  // insert in front.
    for (int col = 0, filled_gap = 0; passed && col < cols; col++)
    {
      filled_gap += old[col] == 0;
      passed &= game.colour_at(index, col) == old[col +1 - filled_gap];
    }

    std::cout << "[" << (summed_tests++) << "] ";
    std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
    passed_tests += passed;
    std::cout << game_to_string(&game) << std::endl
      << "-------" << std::endl << std::endl;
  }

  // --- Insert Top

  // where the gap is.
  for (index = 0; index < cols; index++)
  {
    colour = rand() % field_variety + 1;

    std::cout << "## Slide (Top), col " << index << " ::  "
      << colour << " \u2192 [ ";

    // old row
    old.clear();
    for (int row = 0, tmp = 0; row < rows; row++)  // top to bottom
    {
      tmp = game.colour_at(rows - 1 - row, index);
      old.push_back(tmp);
      std::cout << tmp << " ";
    }
    std::cout << "]" << std::endl;

    game.insert(index + rows, colour);

    // check if everything is pushed down (+1)
    passed = true;
    old.insert(old.begin(), colour);  // insert in front.
    for (int row = 0, filled_gap = 0; passed && row < rows; row++)
    {
      filled_gap += old[row] == 0;
      passed
        &= game.colour_at(rows-1-row, index) == old[row + filled_gap];
    }

    std::cout << "[" << (summed_tests++) << "] ";
    std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
    passed_tests += passed;
    std::cout << game_to_string(&game) << std::endl
      << "-------" << std::endl << std::endl;
  }

  // --- Winning region.

  std::cout << "## Create winning region"  << std::endl;
  // bottom horizontal
  custom_fields.clear();
  for (int f = 0; f < game.get_size(); f++)
  {
    custom_fields.push_back(1 + rand() % 7);
  }
  custom_fields[0] = (3);
  custom_fields[1] = (3);
  custom_fields[2] = (3);
  game.start(custom_fields);
  winning_region = game.search_patterns();

  passed = false;

  for (const FieldPattern &pattern : *winning_region)
  {
    std::cout << pattern.position << " ";
  }
  std::cout << std::endl;

  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  // right vertical (additional)
  std::cout << "## Insert additional winning region (vertical)"  << std::endl;
  custom_fields[(cols-1)+0*rows] = (2);
  custom_fields[(cols-1)+1*rows] = (2);
  custom_fields[(cols-1)+2*rows] = (2);
  game.start(custom_fields);
  winning_region = game.search_patterns();

  passed = false;

  for (const FieldPattern &pattern : *winning_region)
  {
    std::cout << pattern.position << " ";
  }
  std::cout << std::endl;

  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  // should not append: vertical and horizontal (additional)
  std::cout << "## Insert additional winning region (vertical)"  << std::endl;
  custom_fields[2+0*rows] = (3);  // already in [0 1 2]
  custom_fields[2+1*rows] = (3);
  custom_fields[2+2*rows] = (3);
  custom_fields[2+3*rows] = (4);  // avoid it is it's own pattern of three
  game.start(custom_fields);
  winning_region = game.search_patterns();

  passed = false;

  for (const FieldPattern &pattern : *winning_region)
  {
    std::cout << pattern.position << " ";
  }
  std::cout << std::endl;

  std::cout << "[" << (summed_tests++) << "] ";
  std::cout << (passed ? "Passed" : "Failed") << std::endl;
  passed_tests += passed;
  std::cout << game_to_string(&game) << std::endl
    << "-------" << std::endl << std::endl;

  //delete winning_region;

  // ----- Final results.
  std::cout
    << std::endl << "======"
    << std::endl << "[Result]"
    << std::endl << "Passed/Summed: "
    << passed_tests << "/" << summed_tests << std::endl;

  return passed_tests == summed_tests;
}

#endif // _TESTS_H_
