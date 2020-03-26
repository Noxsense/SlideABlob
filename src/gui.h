#ifndef _GUI_H_
#define _GUI_H_

#include<iostream>
#include<vector>
#include<sys/time.h>

#include "SDL.h"

#include "field.h"
#include "game.h"
#include "gui_blob_handler.h"

#define SCREEN_WIDTH 350
#define SCREEN_HEIGHT 480

#define BLOB_SIZE 32
#define BLOB_FRAMES 2
#define BLOB_FRAME_SETS 6

#define FIELD_SIZE 32
#define FIELD_FRAMES 7

// ----

const bool DEBUG = false;

const int BLOB_COUNT = 10;
const int COLOUR_WAITING_LIST = 3;

// ----

int pattern_score(FieldPattern p)
{
  int multiplier = p.size() - 2;  // x1 x2 x3 ...

  switch (p.colour)
  {
    case 1: return 10*multiplier;
    case 2: return 20*multiplier;
    case 3: return 30*multiplier;
    case 4: return 40*multiplier;
    case 5: return 70*multiplier;
    case 6: return 100*multiplier;
    case 7: return 150*multiplier;
    default: return 0;
  }
}

SDL_Surface *load_picture(
    std::string path,
    SDL_Surface *screen_for_transparency = NULL,
    int r = 0, int g = 0xff, int b = 0,  // green
    int a = 0xff)
{
  SDL_Surface *tmp, *surface;

  if ((tmp = SDL_LoadBMP(&*path.begin())) == NULL)
  {
    std::cerr << "Loading picture failed. (Return NULL)" << std::endl;
    return NULL;
  }
  surface = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  if (screen_for_transparency != NULL)
  {
    if (DEBUG) std::cout << "SetColourKey" << std::endl;
    int colourKey = SDL_MapRGB(screen_for_transparency->format, r, g, b);
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, colourKey);
  }

  return surface;
}

void set_frame_square(SDL_Rect *rect, int size, int frame_x = 0, int frame_y = 0)
{
  rect->w = size;
  rect->h = size;

  rect->y = frame_y * size;
  rect->x = frame_x * size;
}

void set_index(
    int index,
    int rows, int cols,
    int *ltr,  // is_left:4, is_top:2, is_right:1
    int *bound_top,
    int *bound_left,
    int *bound_right,
    int *display_index = NULL
    )
{
  bool is_left = index < rows;
  bool is_top = !is_left && index < (rows + cols);
  bool is_right = !is_left && !is_top && index < rows * 2 + cols;

  *ltr = (is_left << 2) + (is_top << 1) + (is_right);

  /* If index is invalid, set bounds to normal == no wrapping bounds. */

  if (!bound_top || !bound_left || !bound_right)
  {
    std::cerr
      << "Warning: Cannot set bounds. No destionation given." << std::endl;
    return; // cannot set bounds.
  }

  /* Set bounds for drawing. in
   * for (0,-1..bound_top] {
   *   for ([bound_left,..bound_right);
   * }
   */
  *bound_left = is_left ? -1 : 0;
  *bound_top = is_top ? - rows - 1 : -rows;
  *bound_right = is_right ? cols + 1 : cols;

  if (display_index)  // set display_index, if requested.
  {
    // display_index indicastes ...
    *display_index
      = is_left ? index % rows  // the row (left side)
      : is_top ? index - rows  // the column (on top)
      : is_right ? rows*2 + cols - 1 - index  // the row (right side)
      : -1;
  }
}

void display_number(
    int p,
    SDL_Surface *surf, SDL_Rect *src, SDL_Surface *screen, SDL_Rect *pos)
{
  if (p < 0)
  {
    return display_number(0, surf, src, screen, pos);
  }

  if (src == NULL || pos == NULL || surf == NULL || screen == NULL)
    return;

  // H = 32,  W=19;

  int div =  p, mod = 0;

  for (div = p; div > 0 || p == 0; div /= 10)
  {
    mod = div % 10;
    src->x = mod * src->w;
    pos->x -= src->w;

    SDL_BlitSurface(surf, src, screen, pos);

    if (!p) break;  // display P:0 at least once.
  }
}

/* Draw the given field with the given SDL resources.*/
void display_field(
    SDL_Surface *field_colours,
    SDL_Rect *rcColourSrc,
    SDL_Surface *screen,
    Field *field,
    int index = -1,
    int insertion_colour = -1,
    int anchor_x = 0,
    int anchor_y = 0,
    int offset = 4)
{
  if (!field)
  {
    std::cerr << "Cannot draw field: No Field!" << std::endl;
    return;
  }
  if (!field_colours|| !screen || !rcColourSrc)
  {
    std::cerr << "Cannot draw field: No drawing resources!" << std::endl;
    return;
  }

  int rows = field->get_rows();
  int cols = field->get_cols();

  int ltr, bound_top, bound_left, bound_right, i_;

  set_index(insertion_colour < 1 ? -1 : index, rows, cols,
      &ltr, &bound_top, &bound_left, &bound_right, &i_);

  SDL_Rect rcColourPos;

  for (int r = 0; r > bound_top; r--)
  {
    rcColourPos.y = anchor_y + (rows+r) * (rcColourSrc->h + offset);

    for (int c = bound_left; c < bound_right; c++)
    {
      rcColourPos.x = (c) * (rcColourSrc->w + offset) + anchor_x;

      // draw field, if inside of [0,cols) and [0,rows)
      if (c >= 0 && c < cols && r <= 0 && r > -rows)
      {
        // field colour
        rcColourSrc->x = rcColourSrc->w * field->colour_at(-r, c);
        SDL_BlitSurface(field_colours, rcColourSrc, screen, &rcColourPos);
      }
      // Indicate chosen position (index) for insertion.
      else if (((ltr & 0b101) && -r == i_)  // left or right, and chosen row
          || (ltr == 2 && c == i_))  // top, and chosen column
      {

        if (DEBUG) std::cout
          << "Insert into LTR:" << (ltr&4) << (ltr&2) << (ltr&1)
            << ", i'" << i_
            << ", colour:" << insertion_colour
            << std::endl;

        // inserting colour
        rcColourSrc->x = rcColourSrc->w * insertion_colour;
        SDL_BlitSurface(field_colours, rcColourSrc, screen, &rcColourPos);
      }
    }
  }
}


long get_current_time_millis()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

int start_window(int rows, int cols, int time_per_turn = 15)
{
  SDL_Surface *screen, *blob_icon, *bg,
              *blob, *numbers, *player_indicator, *field_colours;

  SDL_Rect rcColourPos, rcColourSrc,
           rcBGPos,
           rcNumPos, rcNumSrc;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption("Slide a Blob", "Slide a Lama (Clone) by Nox");

  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
  SDL_EnableKeyRepeat(70, 70); // set keyboard repeat.

  blob_icon = load_picture("res/blobs_icon.bmp", screen, 0xff, 0x0, 0xff, 0xff);
  if (blob_icon == NULL)
  {
    std::cerr << "Loading Blobs (icons) failed. Exit (1)" << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_WM_SetIcon(blob_icon, NULL);

  // ----

  /* Load Blob.
   * pink as colour key. */
  blob = load_picture("res/blobs.bmp", screen, 0xff, 0x0, 0xff, 0xff);
  if (blob == NULL)
  {
    std::cerr << "Loading Blobs failed. Exit (1)" << std::endl;
    SDL_Quit();
    return 1;
  }

  /* Load Field colour. */
  field_colours = load_picture("res/field_colours.bmp",
      screen, 0xff, 0x0, 0xff, 0xff);

  if (field_colours == NULL)
  {
    std::cerr << "Loading Colours failed. Exit (1)" << std::endl;
    SDL_FreeSurface(blob);
    SDL_Quit();
    return 1;
  }

  /* Define frame and sprite from texture.*/
  set_frame_square(&rcColourSrc, FIELD_SIZE, 0, 0);

  /* Load Numbers (for score). */
  numbers = load_picture("res/numbers.bmp",
      screen, 0x33, 0x33, 0x33, 0xff);

  /* Define frame and sprite from texture.*/
  set_frame_square(&rcNumSrc, FIELD_SIZE, 0, 0);
  rcNumSrc.w = 19;  // custom width!

  /* Load Numbers (for score). */
  player_indicator = load_picture("res/indicator.bmp",
      screen, 0x33, 0x33, 0x33, 0xff);

  /* Load Background.*/
  SDL_Surface *tmp = SDL_LoadBMP("res/bg_tile.bmp");
  bg = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);
  // bg = load_picture("res/bg_tile.bmp", NULL);  // freeing problems.
  if (bg == NULL)
  {
    std::cerr << "Loading Background failed. Exit (1)" << std::endl;
    SDL_FreeSurface(blob);
    SDL_FreeSurface(field_colours);
    SDL_Quit();
    return 1;
  }
  rcBGPos.x = 0;
  rcBGPos.y = 0;

  // ----

  bool window_open = true;
  int index = 0;
  int offset = 4;

  int anchor_x
    = (SCREEN_WIDTH - (rcColourSrc.w+offset)*(cols+2)) / 2
    + rcColourSrc.w;
  int anchor_y = rcColourSrc.h * 3.5;

  SDL_Event event;

  int colours_on_field = 7;
  int colours_waiting = 3;

  int number_places = 5;

  BlobGuiHandler blobs_h(SCREEN_WIDTH/2, BLOB_SIZE, BLOB_COUNT);
  blobs_h.set_texture(blob, BLOB_SIZE, -1, BLOB_FRAMES, BLOB_FRAME_SETS);
  blobs_h.set_velocity(BLOB_SIZE / 3);

  Game game(rows, cols, colours_on_field, blobs_h.max_blobs(), colours_waiting);
  game.start();

  int score[8] = { 0, 10, 20, 30, 40, 70, 100, 150 };
  for (int i = 0; i < colours_on_field + 1; i++)
  {
    game.set_colour_score(i, score[i]);
  }

  bool confirm = false, is_removing_pattern = false;

  long now, last_update;  // in ms
  now = get_current_time_millis();  // in ms.
  last_update = now;

  /* Update-Loop: Field and frames, etc.*/
  while (window_open)
  {
    if (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          window_open = false;
          break;

        case SDL_KEYDOWN:
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE: case SDLK_q:
              window_open = false;
              break;

            case SDLK_SPACE:
              if (DEBUG) std::cout << "Confirm (" << index << ")" << std::endl;
              if (!confirm) confirm = true;
              break;

            case SDLK_RIGHT:
              if (!confirm) game.inc_index();
              if (DEBUG) std::cout << "Increase, now " << index << std::endl;
              // update = true;
              break;

            case SDLK_LEFT:
              if (!confirm) game.dec_index();
              if (DEBUG) std::cout << "Decrease, now " << index << std::endl;
              // update = true;
              break;
            default: break;
          }
          break;

        default: break;
      }
    }

    /* ======= Draw. === */
    SDL_FillRect(screen, NULL, 0xffffff); // fill white.

    /* ===== Draw background. =============================================== */
    rcBGPos.x = anchor_x - (rcColourPos.w + offset);
    rcBGPos.y = anchor_y;

    SDL_BlitSurface(bg, NULL, screen, &rcBGPos);

    /* ===== Draw the field and the insertion indicator.. =================== */
    // Update chosen index for display.
    display_field(
        field_colours, &rcColourSrc, screen,  // SDL resources.
        game.get_field(),  // field
        game.get_index(), game.get_waiting_colour(),  // index to insert colour.
        anchor_x, anchor_y, offset  // positioning.
        );

    /* ===== Update: Insert at position. ==================================== */
    if (is_removing_pattern)
    {
      if (game.has_waiting_patterns())
      {
        /* Show the field stones to remove. */
        for (int i : *game.get_first_pattern()) i = i;

        game.add_score_to_current_player(game.remove_first_pattern());
        blobs_h.new_blob_for_player(game.get_current_player());
      }
      else  // if no combo, then new turn: next player, next colour, etc.
      {
        /* Check, if new patterns were built. */
        game.update_pattern_waiting_list();

        /* New turn: next player, next colour, etc. */
        if (!game.has_waiting_patterns())
        {
          confirm = false;
          is_removing_pattern = false;

          game.next_turn();
          game.new_colour();
        }
        // else continue removing pattern.
      }
    }
    else if (confirm)
    {
      game.insert_colour();
      game.update_pattern_waiting_list();
      is_removing_pattern = true;
    }

    /* ===== Draw points and blobs. ========================================= */
    // indicate current player, reuse number rectangle (will be overridden later)
    rcNumPos.y = offset;

    rcNumPos.x = !game.get_current_player()
      ? offset + number_places*rcNumSrc.w
      : SCREEN_WIDTH - offset;
    rcNumSrc.x = 0; rcNumSrc.y = 0;
    for (int i = 0; i < number_places; i ++)
    {
      rcNumPos.x -= rcNumSrc.w;
      SDL_BlitSurface(player_indicator, &rcNumSrc, screen, &rcNumPos);
    }

    rcNumPos.x = offset + number_places*rcNumSrc.w;  // it will grow to the left.
    display_number(game.get_score_of_player(0),
        numbers, &rcNumSrc, screen, &rcNumPos);

    rcNumPos.x = SCREEN_WIDTH - offset;  // it will grow to the left.
    display_number(game.get_score_of_player(1),
        numbers, &rcNumSrc, screen, &rcNumPos);

    // show next insertion colour (waiting list)
    rcColourPos.x
      = (SCREEN_WIDTH - (rcColourSrc.w+offset)*game.count_colours_waiting())
      / 2;
    rcColourPos.y = offset;

    for (long unsigned int i = 0; i < game.count_colours_waiting(); i++)
    {
      rcColourSrc.x = rcColourSrc.w * game.get_waiting_colour(i);
      SDL_BlitSurface(field_colours, &rcColourSrc, screen, &rcColourPos);

      rcColourPos.x += (rcColourSrc.w + offset);
    }

    // Draw lively blobs.
    now = get_current_time_millis();
    if (now - last_update > 500)  // every 0.5 second, not more
    {
      blobs_h.update_all_blobs(true /*random*/);
      last_update = now;
    }

    blobs_h.draw_all_blobs(screen,
        anchor_y + (rows + 3)*(rcColourSrc.h + offset) + 2*offset);

    SDL_UpdateRect(screen, 0, 0, 0, 0);  // update screen.
  }

  std::cout << "Free bg." << std::endl;
  SDL_FreeSurface(bg);
  std::cout << "Free blob." << std::endl;
  SDL_FreeSurface(blob);
  std::cout << "Free colours." << std::endl;
  SDL_FreeSurface(field_colours);
  std::cout << "Free numbers." << std::endl;
  SDL_FreeSurface(numbers);

  SDL_Quit();

  return 0;
}

#endif // _GUI_H_
