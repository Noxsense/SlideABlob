#ifndef _GAME_H_
#define _GAME_H_

#include <iostream>
#include <vector>

#include "field.h"
// #include "blob_handler.h"

class Game
{
  private:
    // field
    Field field;
    int colours;  // count of different colours.

    // players and scores.
    int score[2] = {0, 0};
    int blobs[2] = {0, 0};
    int blobs_size;
    // BlobGuiHandler blob_handler;  // also for non gui

    // colour and current turn.
    bool player1 = true;  // indicates, it is player 0's turn
    long unsigned int insert_index; // index for insertion.
    long unsigned int waiting_size;  // size of colour waiting list "colours_waiting".
    std::vector<int> colours_waiting;  // waiting list for the colours.

    // removing patterns: step by step
    std::vector<FieldPattern> *waiting_patterns;

    std::vector<long unsigned int> colour_scores;

  public:
    /* Create a new game.
     * Field dimension:  rows*cols.
     * Colour count: colours.
     * Summing Blob count: blobs.
     * Size of colour waiting list: colours_waiting.
     */
    Game(int rows=5, int cols=5, int colours=5, int blobs=10, int waiting=3);

    /* Clear the lists. Print the last winner.*/
    ~Game();

    /* Get the game field. ATTENTION: Changes will apply in the game. */
    Field *get_field();
    // TODO wrap for safety/security?

    void set_colour_score(long unsigned int  colour, int score);

    /* Start the game, player 0 starts.. */
    void start();

    /* Pop the first colour and append a new colour. */
    void new_colour();

    /* Change active player. [0,1]. */
    void next_turn();

    /* Get current player.  Return player (0/false) and player (1/true). */
    bool get_current_player();

    /* Get the player who currently has all blobs or the highest score. */
    bool get_current_winner();

    /* Get the current colour which will be inserted. */
    int get_waiting_colour(int i = 0);

    /* Get the number of colours waiting (and about) to be inserted. */
    long unsigned int count_colours_waiting();

    /* Set the insertion index. */
    void set_index(int);
    void inc_index();
    void dec_index();

    /* Get the insertion index. */
    int get_index();

    /* Insert current colour at given index. */
    void insert_colour();

    /* New list of patterns to remove, if nothing is waiting yet. */
    void update_pattern_waiting_list();

    /* Check if the game has patterns, which are about to be removed. */
    bool has_waiting_patterns();

    /* Get the indices of the first pattern, if there are patterns waiting. */
    std::vector<int> *get_first_pattern();

    /* Remove the first pattern of the waiting list, return its value.*/
    int remove_first_pattern();

    /* Add score to the given player. */
    void add_score(bool player1, int score);

    /* Add score to the player of the current turn. */
    void add_score_to_current_player(int score);

    /* Return the score of the requesting player. */
    int get_score_of_player(bool player1);

    /* Return the lamas of the requesting player. */
    int get_blobs_of_player(bool player1);
};

//=============================================================================
//-----------------------------------------------------------------------------
//new game and end game

Game::Game(int rows, int cols, int colours, int blobs, int waiting_size)
{
  /* Don't allow values, smaller than 1.*/
  this->field.resize(rows < 1 ? 1 : rows, cols < 1 ? 1 : cols);
  this->colours = colours < 1 ? 1 : colours;
  this->waiting_size = waiting_size < 1 ? 1 : waiting_size;
  this->blobs_size = blobs < 2 ? 2 : blobs;
}

Game::~Game()
{
  if (waiting_patterns)
  {
    delete waiting_patterns;
    waiting_patterns = NULL;
  }

  // print the last winner.
  std::cout << "WINNER: Player " << (get_current_winner()) << std::endl;
}

Field *Game::get_field()
{
  return &(this->field);
}

void Game::set_colour_score(long unsigned int colour, int score)
{
  while (this->colour_scores.size() < colour + 1)
  {
    this->colour_scores.push_back(0); // fill with at least 0
  }
  this->colour_scores[colour] = score < 0 ? 0 : score;
}

//-----------------------------------------------------------------------------
// switch turns and get new colours.

void Game::start()
{
  /* fill field */
  this->field.start(this->colours);  // field_variety := colours

  /* set insertion colour */
  if (!this->colours_waiting.size())
    this->new_colour();

  this->insert_index = 0;

  /* Remove possible first field pattern (no points) */
  waiting_patterns = field.search_patterns();
  while (waiting_patterns->size())
  {
    field.remove_patterns(waiting_patterns);
    waiting_patterns = field.search_patterns();
  }
  if (waiting_patterns)
  {
    delete waiting_patterns;
    waiting_patterns = NULL;
  }

  this->player1 = false;
  this->score[0] = 0;
  this->score[1] = 0;

  /* Blobs equally divided by two */
  this->blobs[0] = blobs_size / 2;
  this->blobs[1] = blobs_size - blobs[0];
}

void Game::new_colour()
{
  if (colours_waiting.size())
  {
    this->colours_waiting.erase(colours_waiting.begin());
  }

  while (colours_waiting.size() < waiting_size)
  {
    this->colours_waiting.push_back(rand() % this->colours + 1);
  }
}

void Game::next_turn()
{
  this->player1 = !player1;  // toggle.
}

bool Game::get_current_player()
{
  return this->player1;
}

bool Game::get_current_winner()
{
  /* Check if someone has all blobs. */
  if (blobs[0] == 0 || blobs[1] == 0)
  {
    return this->blobs[0] < this->blobs[1];  // p1 with more blobs == true / 1
  }

  /* Otherwise check, who has the current highscore. */
  return this->score[0] < this->score[1];  // score1 bigger == return 1/player 1
}

//-----------------------------------------------------------------------------
//colour inserted, maybe get patterns and update scores.

int Game::get_waiting_colour(int i)
{
  return this->colours_waiting[i];  // if not assigned, return 0 == empty.
}

long unsigned int Game::count_colours_waiting()
{
  return this->colours_waiting.size();
}

int Game::get_index()
{
  return this->insert_index;
}

void Game::set_index(int i)
{
  int bounds_max = this->field.get_bounds_max();

  this->insert_index
    = (i < 0) ? 0
    : (i >= bounds_max) ? bounds_max - 1
    : i;
}

void Game::inc_index()
{
  this->set_index(this->insert_index + 1);
}

void Game::dec_index()
{
  this->set_index(this->insert_index - 1);
}

void Game::insert_colour()
{
  if (colours_waiting.size() < 1)
  {
    this->new_colour();
  }
  this->field.insert(insert_index, colours_waiting[0]);
}

void Game::update_pattern_waiting_list()
{
  if (has_waiting_patterns())
    return;

  this->waiting_patterns = this->field.search_patterns();
}

bool Game::has_waiting_patterns()
{
  return waiting_patterns != NULL && this->waiting_patterns->size();
}

int Game::remove_first_pattern()
{
  if (!has_waiting_patterns())  // nothing to remove.
    return 0;

  FieldPattern p = this->waiting_patterns->at(0);

  /* size 3 => 1x score
   * size 4 => 2x score
   * size 5 => 3x score ... */
  int pattern_score
    = colour_scores[p.colour] * (p.size() - 2);

  /* Pop the first.*/
  this->waiting_patterns->erase(this->waiting_patterns->begin());

  this->field.remove_pattern(p);

  /* Emptied. */
  if (!this->waiting_patterns->size())
  {
    delete waiting_patterns;
    waiting_patterns = NULL;
  }

  return pattern_score;
}

std::vector<int> *Game::get_first_pattern()
{
  if (!has_waiting_patterns()) return NULL;

  std::vector<int> *indices;
  indices = new std::vector<int>();

  FieldPattern p = waiting_patterns->at(0);

  int direction = p.is_horizontal() ? 1 : field.get_cols();
  int start = p.position;

  indices->push_back(start);

  for (int i = 1; i < p.size(); i++)
  {
    indices->push_back(start + i*direction);
  }

  return indices;
}

void Game::add_score(bool player1, int score)
{
  this->score[player1] += score;

  if (score > 0)  // add also a blob to the winner.
  {
    // Add, if the other can give. Remove from other, if not null.
    this->blobs[player1] += this->blobs[!player1] > 0 ? 1 : 0;
    this->blobs[!player1] -= this->blobs[!player1] > 0 ? 1 : 0;
  }
}

void Game::add_score_to_current_player(int score)
{
  this->add_score(get_current_player(), score);
}

int Game::get_score_of_player(bool player1)
{
  return this->score[player1];
}

int Game::get_blobs_of_player(bool player1)
{
  return this->blobs[player1];
}

#endif  //  _GAME_H_
