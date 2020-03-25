#ifndef _FIELD_H_
#define _FIELD_H_

#include <iostream>
#include <string>
#include <vector>

// reference: https://www.youtube.com/watch?v=CXQXQgVflCI

// triple of ints.
class FieldPattern
{
  public:
    int position; // starting position [0]
    int type; // negative: vertical, positive: horizontal
    int colour;

    FieldPattern(int position, int type, int colour)
      : position(position), type(type), colour(colour)
    {}

    bool is_horizontal()
    {
      return type >= 0;
    }

    int size()
    {
      return type < 0 ? -type : type;
    }

    std::string to_string()
    {
      std::string str = "";

      for (int i = 0; i < size(); i++)
      {
        str += " " + std::to_string(colour);
      }

      return "[" + std::to_string(position) + " "
        + (is_horizontal() ? "\u2192" : "\u2191")
        + str + "]";
    }
};

class Field
{
  private:
    int rows, size;
    std::vector<int> field;

    int score[2];

    int set(int row, int col, int colour);  // return old field
    int set(int index, int colour);  // return old field

  public:
    // number of rows and cols
    Field(int rows = 5, int cols = 5);
    ~Field();

    void resize(int rows, int cols);  // reset the whole field dimensions.

    // public
    int get_size();
    int get_rows();
    int get_cols();

    int get_bounds_max();  // maximum positions for colour insertion.

    void start(int field_variety = 7);  // number of different colours
    void start(std::vector<int> starting_fields);

    int colour_at(int index);
    int colour_at(int row, int col);

    void insert(int index, int colour);
    void fix_gavity(); // fill all gaps, if something is above.

    // check for every field, if they are in wining.
    std::vector<FieldPattern> *search_patterns();
    void remove_pattern(FieldPattern pattern, bool auto_gravity = true);
    void remove_patterns(std::vector<FieldPattern> *pattern, bool auto_gravity = true);
};

/** Create field size.*/
Field::Field(int rows, int cols)
{
  this->resize(rows, cols);
}

/** free field.*/
Field::~Field()
{
  // delete this->field;
}

void Field::resize(int rows, int cols)
{
  srand(time(0));
  this->size = rows * cols;
  this->rows = rows;

  this->field.clear();
  for (int i = 0; i < size; i++) this->field.push_back(0);  // fill all empty
}

/**
 * Start with random setup.
 */
void Field::start(int field_variety)
{
  /* Randomly filled.*/
  for (int i = 0; i < this->size; i++)
  {
    this->set(i, 1 + rand() % field_variety);
  }
}

/**
 * Start with customized setup.
 */
void Field::start(std::vector<int> starting_fields)
{
  /* Randomly filled.*/
  int i = 0;
  for (const int f : starting_fields)
  {
    this->set(i++, f);
  }
}

int Field::get_size()
{
  return this->size;
}

int Field::get_rows()
{
  return this->rows;
}

int Field::get_cols()
{
  return this->size / this->rows;
}

int Field::get_bounds_max()
{
  return this->get_rows() * 2 + this->get_cols();
}

/** Return colour on that position (index).
 * 0 if that field is empty.
 */
int Field::colour_at(int index)
{
  if (index < 0 || index >= size)
  {
    return -1; // invalid index, invalid colour.
  }
  return this->field[index];
}

/**
 * Get old colour of that position (index).
 * If colour is invalid, don't change the colour, return the colour argument.
 */
int Field::set(int index, int colour)
{
  if (colour < 0)
  {
    std::cerr << "Field::set("<<(index)<<", "<<(colour)<<") "
      << "- Invalid colour." << std::endl;
    return colour;
  }
  else if (index >= 0 && index < size)
  {
    int old = this->colour_at(index);
    this->field[index] = colour;
    return old;
  }
  else
  {
    std::cerr << "Field::set("<<(index)<<", "<<(colour)<<") "
      << "- Invalid arguments." << std::endl;
    return -1;  // invalid index, invalid colour
  }
}

/** Return colour on that position (row, col).
 * 0 if that field is empty.
 */
int Field::colour_at(int row, int col)
{
  if (row < 0 || col < 0 || row >= rows || col >= this->get_cols())
    return -1; // invalid.
  return this->colour_at(row*get_cols() + col);
}

/**
 * Get old colour of that position (row,col).
 * If colour is invalid, don't change the colour, return the colour argument.
 */
int Field::set(int row, int col, int colour)
{
  if (row < 0 || col < 0 || row >= rows || col >= this->get_cols())
    return -1; // invalid.
  return this->set(row*this->get_cols() + col, colour);
}

/**
 * Pos starts (0) left, row (0) and goes clockwise, puter row.
 */
void Field::insert(int index, int colour)
{
  if (colour < 1) return;  // invalid colour.
  if (index < 0) return;  // invalid position

  int cols = this->get_cols();
  int waiting = colour;

  // [left] ++ [top] ++ [right]
  int left_end = this->rows;
  int cols_end = left_end + cols;
  int right_end = cols_end + rows;

  if (index < left_end)  // insert left
  {
    int row = index;

    /* Push all (of that row)
     * one field to the right.
     * Stop at empty field or end!
     */
    for (int col = 0; col < cols; col ++)
    {
      if (waiting < 1) break;

      /*updated*/ waiting = this->set(row, col, waiting);  // right of current
    }
    // "waiting may fall out."
  }

  else if (left_end <= index && index < cols_end)  // insert top
  {
    int col = index - left_end;

    /* Push all (of that coloum)
     * one field down.
     */
    for (int row = rows - 1; row >= 0; row--)
    {
      if (waiting < 1) break;

      /*updated*/ waiting = this->set(row, col, waiting);  // below of current
    }
    // "waiting may fall out."
  }
  else if (cols_end <= index && index < right_end)
  {
    int row = rows - index + cols_end - 1;

    /* Push all (of that row)
     * one field to the left.
     * Stop at empty field or end!
     */
    for (int col = cols - 1; col >= 0; col --)
    {
      if (waiting < 1) break;

      /*updated*/ waiting = this->set(row, col, waiting);  // left of current
    }
  }
  else
  {
    std::cout
      << "[Error] Tried to insert on "
      << "invalid insertion position (" << index << ")" << std::endl;
    std::cerr
      << "[Error] Tried to insert on "
      << "invalid insertion position (" << index << ")" << std::endl;
    return;
  }

  /* Update gravity, maybe holes with side or top insertion.*/
  this->fix_gavity();
}

void Field::fix_gavity()
{
  int above = 0, colour_above = 0;

  // for all cols
  for (int col = 0; col < this->get_cols(); col++)
  {
    // for all fields in that col
    for (int row = 0; row < rows-1; row++)  // bottom -> almost top
    {
      if (colour_at(row, col)) continue;  // skip if not empyt.

      // if  this is empty, get next not empty
      above = row;
      colour_above = 0;

      while(!(colour_above = colour_at(above, col)) && above < rows)
      {
        above ++;
      }
      this->set(row, col, colour_above > 0 ? colour_above : 0);
      this->set(above, col, 0);  // empty next.
    }
  }
}

std::vector<FieldPattern> *Field::search_patterns()
{
  std::vector<FieldPattern> *winning_regions = new std::vector<FieldPattern>();
  std::vector<int> backup(this->field);  // if pattern later removed

  int cols = this->get_cols();
  int col, row, colour;

  for (int i = 0; i < size; i++)
  {
    colour = colour_at(i);

    col = i % cols;
    row = i / cols;

    // horizontal: after (+1)
    if (col != cols-2 && colour && colour > 0
        && colour_at(row, col+1) == colour && colour && colour_at(row, col+2) == colour
        && colour_at(row, col-1) != colour)  // avoid overlapping
    {
      int type = +3;

      // act, like it's deleted later
      this->set(row,col + 0, 0);
      this->set(row,col + 1, 0);
      this->set(row,col + 2, 0);

      // increase pattern, if still on same row.
      while (colour_at(row, col+type) == colour)
      {
        this->set(row,col + type, 0);
        type += 1;
      }

      winning_regions->push_back(FieldPattern(i, type, colour));
    }

    // vertical: -row:above, 0:this, +row:below
    if (i <= (this->size - 2*this->rows) && colour > 0
        && colour_at(row+1, col) == colour && colour_at(row+2, col) == colour
        && colour_at(row-1, col) != colour)  // avoid overlapping
    {
      int type = -3;

      // act, like it's deleted later
      this->set(row + 0,col, 0);
      this->set(row + 1,col, 0);
      this->set(row + 2,col, 0);

      // increase pattern, if still on same row (!= -1)
      while (colour_at(row - type,col) == colour)
      {
        this->set(row - type,col, 0);
        type -= 1;
      }

      winning_regions->push_back(FieldPattern(i, type, colour));
    }
  }

  this->field = backup;  // restore backup.

  /* Unique */
  return winning_regions;
}

void Field::remove_pattern(FieldPattern p, bool auto_gravity)
{
  bool horizontal = p.is_horizontal();
  int form_max = p.size();
  int form_skip = horizontal ? 1 : this->get_cols();

  for (int i = 0; i < form_max; i++)
  {
    this->set(p.position + i*form_skip, 0);
  }

  if (auto_gravity)
  {
    this->fix_gavity();
  }
}

void Field::remove_patterns(std::vector<FieldPattern> *pattern, bool auto_gravity)
{
  if (pattern == NULL) return;

  for (FieldPattern p : *pattern)
  {
    this->remove_pattern(p, false);
  }

  if (auto_gravity)
  {
    this->fix_gavity();
  }

  delete pattern;
}

#endif
