#ifndef _TESTS_H_
#define _TESTS_H_

#include<iostream>
#include<string>
#include<vector>

#include "field.h"

std::string field_to_string(Field *g)
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

bool test_field(int rows, int cols, int field_variety = 7, bool verbose = true)
{

  if (rows < 3 || cols < 3)
  {
    if (verbose) std::cout
      << "Too small ("<<(rows)<<","<<(cols)<<")"
      << "for valid playing field, at least 4x4!" << std::endl;
    return 1;
  }

  int colour = 0, index = 0;
  int passed_tests = 0, summed_tests = 0;
  bool passed;
  std::vector<int> custom_fields, old;
  std::vector<FieldPattern> *winning_region;

  Field field(rows, cols);  // classical 4x4

  // --- Start with size == rows * cols

  if (verbose) std::cout
    << "Starting field(" << (rows) << "," << (cols) << "). "
      << "Test if size() == " << (rows*cols) << std::endl;

  passed = rows*cols == field.get_size();
  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  field.start();

  // --- Start randomized, everyone is not empty.

  if (verbose) std::cout << "## Staring field: Every field is not empty." << std::endl;

  passed = true;
  for (int i = 0; i < field.get_size(); i++)
  {
    colour = field.colour_at(i);
    passed &= colour != 0;
    custom_fields.push_back(colour);
  }

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Fill customized

  field.start();  // again randomized.

  if (verbose) std::cout
    << "## Starting field with custom fields is filling correctly."
      << std::endl;

  field.start(custom_fields);

  passed = true;
  if (verbose) std::cout << "custom: [ ";
  for (int i = 0; i < field.get_size(); i++)
  {
    passed &= field.colour_at(i) == custom_fields[i];
    if (verbose) std::cout << custom_fields[i] << " ";
  }
  if (verbose) std::cout << "]" << std::endl;

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Fill with empty cell

  if (verbose) std::cout
    << "## Create a field with empty cell in first (lower) row."
      << std::endl;

  index = rand() % field.get_cols();  // row[0]
  custom_fields[index] = 0;
  field.start(custom_fields);

  passed = field.colour_at(0, index) == 0;
  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Fix Gravity
  if (verbose) std::cout << "## Fix Gravity." << std::endl;

  passed = field.colour_at(index) == 0;

  old.clear();
  if (verbose) std::cout << " .. will fall down: [ ";
  for (int row = 1, tmp = 0; row < rows; row++)
  {
    tmp = field.colour_at(row, index);
    old.push_back(tmp);
    if (verbose) std::cout << tmp << " ";
  }
  if (verbose) std::cout << "]" << std::endl;

  field.fix_gavity();

  passed = true;
  old.push_back(0);  // top of this column should be empty.
  for (int row = 0; row < rows; row++)
  {
    passed &= field.colour_at(row, index) == old[row];
  }

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Slide in LEFT <-

  for (index = 0; index < rows; index++)
  {
    colour = rand() % field_variety + 1;

    if (verbose) std::cout << "## Slide (left), row " << index << " ::  "
      << "[ " << colour << " | ";

    // old row
    old.clear();
    for (int col = 0, tmp = 0; col < cols; col++)
    {
      tmp = field.colour_at(index, col);
      old.push_back(tmp);
      if (verbose) std::cout << tmp << " ";
    }
    if (verbose) std::cout << "]" << std::endl;

    field.insert(index, colour);

    // check if everything is pushed to the right (+1)
    passed = true;
    old.insert(old.begin(), colour);  // insert in front.
    for (int col = 0, filled_gap = 0; passed && col < cols; col++)
    {
      filled_gap += old[col] == 0;
      passed &= field.colour_at(index, col) == old[col + filled_gap];
    }

    summed_tests += 1;
    passed_tests += passed;

    if (verbose) std::cout
      << "[" << (summed_tests) << "] "
        << (passed ? "Passed" : "Failed") << std::endl << std::endl;
    if (verbose) std::cout << field_to_string(&field) << std::endl
      << "-------" << std::endl << std::endl;
  }

  // --- Slide in RIGHT

  for (index = 0; index < rows; index++)
  {
    colour = rand() % field_variety + 1;

    if (verbose) std::cout << "## Slide (right), row " << index << " ::  "
      << "[ ";

    // old row
    old.clear();
    for (int col = 0, tmp = 0; col < cols; col++)
    {
      tmp = field.colour_at(index, col);
      old.push_back(tmp);
      if (verbose) std::cout << tmp << " ";
    }
    if (verbose) std::cout << "| " << colour << " ]" << std::endl;

    field.insert(2*rows + cols - 1 - index, colour);

    // check if everything is pushed to the right (+1)
    passed = true;
    old.push_back(colour);  // insert in front.
    for (int col = 0, filled_gap = 0; passed && col < cols; col++)
    {
      filled_gap += old[col] == 0;
      passed &= field.colour_at(index, col) == old[col +1 - filled_gap];
    }

    summed_tests += 1;
    passed_tests += passed;

    if (verbose) std::cout
      << "[" << (summed_tests) << "] "
        << (passed ? "Passed" : "Failed") << std::endl << std::endl;
    if (verbose) std::cout << field_to_string(&field) << std::endl
      << "-------" << std::endl << std::endl;
  }

  // --- Insert Top

  // where the gap is.
  for (index = 0; index < cols; index++)
  {
    colour = rand() % field_variety + 1;

    if (verbose) std::cout << "## Slide (Top), col " << index << " ::  "
      << colour << " \u2192 [ ";

    // old row
    old.clear();
    for (int row = 0, tmp = 0; row < rows; row++)  // top to bottom
    {
      tmp = field.colour_at(rows - 1 - row, index);
      old.push_back(tmp);
      if (verbose) std::cout << tmp << " ";
    }
    if (verbose) std::cout << "]" << std::endl;

    field.insert(index + rows, colour);

    // check if everything is pushed down (+1)
    passed = true;
    old.insert(old.begin(), colour);  // insert in front.
    for (int row = 0, filled_gap = 0; passed && row < rows; row++)
    {
      filled_gap += old[row] == 0;
      passed
        &= field.colour_at(rows-1-row, index) == old[row + filled_gap];
    }

    summed_tests += 1;
    passed_tests += passed;

    if (verbose) std::cout
      << "[" << (summed_tests) << "] "
        << (passed ? "Passed" : "Failed") << std::endl << std::endl;
    if (verbose) std::cout << field_to_string(&field) << std::endl
      << "-------" << std::endl << std::endl;
  }

  // --- Winning region.

  if (verbose) std::cout << "## Create winning region"  << std::endl;
  // bottom horizontal
  custom_fields.clear();
  for (int f = 0; f < field.get_size(); f++)
  {
    custom_fields.push_back(1 + rand() % 7);
  }
  custom_fields[0] = (3);
  custom_fields[1] = (3);
  custom_fields[2] = (3);
  field.start(custom_fields);
  winning_region = field.search_patterns();

  if (verbose) std::cout << "- Should contain: (pos:0, +3, 3)\n";

  passed = false;

  for (const FieldPattern &pattern : *winning_region)
  {
    passed |= (pattern.position == 0 && pattern.type >= 3);  // contains 0;
    if (verbose) std::cout << pattern.position << " ";
  }
  if (verbose) std::cout << std::endl;

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // right vertical (additional)
  if (verbose) std::cout << "## Insert additional winning region (vertical)"  << std::endl;
  custom_fields[(cols-1)+0*cols] = (2);
  custom_fields[(cols-1)+1*cols] = (2);
  custom_fields[(cols-1)+2*cols] = (2);
  custom_fields[(cols-1)+3*cols] = (2); // pattern of four.
  custom_fields[(cols-2)+0*cols] = (3); // pattern of three, before (horizontal).
  field.start(custom_fields);
  winning_region = field.search_patterns();

  if (verbose) std::cout << "- Should contain: (pos:0, +3, 3)\n";
  if (verbose) std::cout << "- Should contain: (pos:"<< (cols - 1) << ", -4, 2)\n";

  passed = false;

  for (const FieldPattern &pattern : *winning_region)
  {
    passed |= (pattern.position == 0 && pattern.type >= 3);  // contains 0;
    passed |= (pattern.position == cols - 1 && pattern.type <= -4);  // and contains cols -1 ;
    if (verbose) std::cout << pattern.position << " ";
  }
  if (verbose) std::cout << std::endl;

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // should not append: vertical and horizontal (additional)
  if (verbose) std::cout << "## Insert additional winning region (vertical)"  << std::endl;
  custom_fields[2+0*cols] = (3);  // already in [0 1 2]
  custom_fields[2+1*cols] = (3);
  custom_fields[2+2*cols] = (3);
  custom_fields[2+3*cols] = (4);  // avoid it is it's own pattern of three
  custom_fields[2+1+0*cols] = (2);  // avoid it is it's own pattern of three
  field.start(custom_fields);
  winning_region = field.search_patterns();

  if (verbose) std::cout << "- Should contain: (pos:0, +3, 3)\n";
  if (verbose) std::cout << "- Should contain: (pos:"<< (cols - 1) << ", -4, 2)\n";
  if (verbose) std::cout << "- Should not contain: (pos:"<< (2) << ", -3, 3)\n";

  passed = false;
  bool tmp = true;

  for (const FieldPattern &pattern : *winning_region)
  {
    passed |= (pattern.position == 0 && pattern.type >= 3);  // contains 0;
    passed |= (pattern.position == cols - 1 && pattern.type <= -4);  // and contains cols -1 ;

    tmp &= !(pattern.position == 2 && pattern.type > 2); // contains not 2

    if (verbose) std::cout << pattern.position << " ";
  }
  passed &= tmp;
  if (verbose) std::cout << std::endl;

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
      << (passed ? "Passed" : "Failed") << std::endl;
  if (verbose) std::cout << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // --- Deloete Winning regions.

  if (verbose) std::cout
    << "## Delete Winning region." << std::endl;

  field.remove_patterns(winning_region, false);

  if (verbose) std::cout
    << "### At least bot [0,1,2] are now empty" << std::endl;

  passed = !field.colour_at(0,0) && !field.colour_at(0,1) && !field.colour_at(0,2);

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
    << (passed ? "Passed" : "Failed") << std::endl;

  if (verbose) std::cout
    << "### At least bot [col-1,2*col-1,3*col-1,4*col-1] are now empty"
    << std::endl
    << "[" << field.colour_at(0,cols-1)
    << " " << field.colour_at(1,cols-1)
    << " " << field.colour_at(2,cols-1)
    << " " << field.colour_at(3,cols-1) << "]" << std::endl;

  passed
    = !field.colour_at(0,cols-1)
    && !field.colour_at(1,cols-1)
    && !field.colour_at(2,cols-1)
    && !field.colour_at(3,cols-1);

  summed_tests += 1;
  passed_tests += passed;

  if (verbose) std::cout
    << "[" << (summed_tests) << "] "
    << (passed ? "Passed" : "Failed") << std::endl;

  if (verbose) std::cout
    << field_to_string(&field) << std::endl
    << "-------" << std::endl << std::endl;

  // ----- Final results.
  passed = passed_tests == summed_tests;

  if (verbose) std::cout
    << std::endl << "======"
    << std::endl << "[Result] rows: " << rows << ", cols: " << cols
    << " -- Passed/Summed: "
    << passed_tests << "/" << summed_tests
    << " -- " << (passed ? "PASSED" : "FAILED") << "!"
    << std::endl << "-------"
    << std::endl << std::endl << std::endl;

  return passed;
}

#endif // _TESTS_H_
