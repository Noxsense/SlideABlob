#ifndef _GUI_H
#define _GUI_H

#include<iostream>
#include<vector>
#include "SDL.h"

#include "game.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 512

#define LAMA_SIZE 96
#define LAMA_FRAMES 4

#define FIELD_SIZE 32
#define FIELD_FRAMES 7

const bool DEBUG = false;

int score_for(FieldPattern p)
{
  int multiplier = p.get_size() - 2;  // x1 x2 x3 ...

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
    std::cout << "SetColourKey" << std::endl;
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

  if (!ltr)
  {
    std::cout
      << "Warning: Invalid index " << index << " "
      << "for dimensions (r:" << rows << ", c:" << cols << ")"
      << std::endl;
    return;
  }

  /* Set bounds for drawing. in
   * for (0,-1..bound_top] {
   *   for ([bound_left,..bound_right);
   * }
   */
  *bound_left = is_left ? -1 : 0;
  *bound_top = is_top ? - rows - 1 : -rows;
  *bound_right = is_right ? cols + 1 : cols;

  if (display_index)
  {
    // display_index indicastes ...
    *display_index
      = is_left ? index % rows  // the row (left side)
      : is_top ? index - rows  // the column (on top)
      : is_right ? rows*2 + cols - 1 - index  // the row (right side)
      : 0;
  }
}

void display_points(int p, SDL_Surface *surf, SDL_Rect *src, SDL_Surface *screen, SDL_Rect *pos)
{
  if (p < 0)
  {
    return display_points(0, surf, src, screen, pos);
  }

  if (src == NULL || pos == NULL || surf == NULL)
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

int start_window(int rows, int cols, int time_per_turn = 15)
{
  SDL_Surface *screen, *lama, *numbers, *bg, *field_colours;
  SDL_Rect rcLamaPos, rcLamaSrc,
           rcColourPos, rcColourSrc,
           rcBGPos,
           rcNumPos, rcNumSrc;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption("Slide a Lama (Clone)", "Slide a Lama by Nox");

  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_WIDTH, 0, 0);
  SDL_EnableKeyRepeat(70, 70); // set keyboard repeat.

  // ----

  int ck; // colour key for alpha

  /** Load Lama.
   * pink as colour key. */
  lama = load_picture("res/llama_eat_0.bmp", screen, 0xff, 0x0, 0xff, 0xff);
  if (lama == NULL)
  {
    std::cerr << "Loading Lamas failed. Exit (1)" << std::endl;
    SDL_Quit();
    return 1;
  }

  /* Define frame and sprite from texture.*/
  set_frame_square(&rcLamaSrc, LAMA_SIZE, 0, 3);

  /** Load Field colour. */
  field_colours = load_picture("res/field_colours.bmp",
      screen, 0xff, 0x0, 0xff, 0xff);

  if (field_colours == NULL)
  {
    std::cerr << "Loading Colours failed. Exit (1)" << std::endl;
    SDL_FreeSurface(lama);
    SDL_Quit();
    return 1;
  }

  /* Define frame and sprite from texture.*/
  set_frame_square(&rcColourSrc, FIELD_SIZE, 0, 0);

  /** Load Numbers (for score). */
  numbers = load_picture("res/numbers.bmp",
      screen, 0xff, 0x0, 0xff, 0xff);

  /* Define frame and sprite from texture.*/
  set_frame_square(&rcNumSrc, FIELD_SIZE, 0, 0);
  rcNumSrc.w = 19;  // custom width!

  /** Load Background.*/
  SDL_Surface *tmp = SDL_LoadBMP("res/bg_tile.bmp");
  bg = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);
  // bg = load_picture("res/bg_tile.bmp", NULL);  // freeing problems.
  if (bg == NULL)
  {
    std::cerr << "Loading Background failed. Exit (1)" << std::endl;
    SDL_FreeSurface(lama);
    SDL_FreeSurface(field_colours);
    SDL_Quit();
    return 1;
  }
  rcBGPos.x = 0;
  rcBGPos.y = 0;

  // ----

  bool window_open = true;
  int index = 0;
  int colour = 1;
  int offset = 4, left_aligned, starting_low;

  int anchor_x = 60;
  int anchor_y = 60;

  // inserting and field outer bounds (for stone insertion)
  int outer_indices, ltr, bound_top, bound_left, bound_right, i_;
  outer_indices = 2 * rows + cols;

  std::vector<int> gcolour;
  gcolour.push_back(rand() % FIELD_FRAMES + 1);

  left_aligned = (2) * (rcColourSrc.w + offset);
  starting_low = (2 + rows) * (rcColourSrc.h + offset);

  left_aligned = 60;;
  starting_low = 170;

  SDL_Event event;

  Game game(rows, cols);  // classical 4x4
  game.start();

  int lamas[2] = {5, 5}, score[2] = {0, 0}, tmp_score = 0;
  bool turn_player0 = true;  // indicates, it is player 0's turn

  bool confirm = false, is_removing_pattern = false;
  std::vector<FieldPattern> *pattern;

  /* Remove possible random starting patterns.*/
  pattern = game.search_patterns();
  while (pattern->size())
  {
    game.remove_patterns(pattern);
    pattern = game.search_patterns();
  }

  long last_update = time(NULL);

  /* Update-Loop: Game and frames, etc.*/
  while (window_open)
  {
    bool update = false;
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
              confirm = true;
              break;

            case SDLK_RIGHT:
              if (!confirm && index < outer_indices - 1)
                index += 1;
              if (DEBUG) std::cout << "Increase, now " << index << std::endl;
              update = true;
              break;

            case SDLK_LEFT:
              if (!confirm && 0 < index)
                index -= 1;
              if (DEBUG) std::cout << "Decrease, now " << index << std::endl;
              update = true;
              break;
          }
          break;
      }
    }

    /* ======= Draw. === */
    SDL_FillRect(screen, NULL, 0xffffff); // fill white.

    /* ===== Draw background. =============================================== */
    rcBGPos.x = anchor_x - (rcColourPos.w + offset);
    rcBGPos.y = anchor_y;

    /* ===== Draw the field and the insertion indicator.. =================== */
    // Update chosen index for display.
    set_index(index, rows, cols,
        &ltr, &bound_top, &bound_left, &bound_right, &i_);
    for (int r = 0; r > bound_top; r--)
    {
      rcColourPos.y = anchor_y + (rows+r) * (rcColourSrc.h + offset);

      for (int c = bound_left; c < bound_right; c++)
      {
        rcColourPos.x = (c) * (rcColourSrc.w + offset) + anchor_x;

        // draw field, if inside of [0,cols) and [0,rows)
        if (c >= 0 && c < cols && r <= 0 && r > -rows)
        {
          // field colour
          rcColourSrc.x = rcColourSrc.w * game.colour_at(-r, c);
          SDL_BlitSurface(field_colours, &rcColourSrc, screen, &rcColourPos);
        }
        // Indicate chosen position (index) for insertion.
        else if (!confirm &&
            ((ltr & 0b101) && -r == i_)  // left or right, and chosen row
            || (ltr == 2 && c == i_)  // top, and chosen column
            )
        {

          if (DEBUG) std::cout
            << "Insert into LTR:" << (ltr&4) << (ltr&2) << (ltr&1)
              << ", i'" << i_
              << ", colour:" << gcolour[0]
              << std::endl;

          // inserting colour
          rcColourSrc.x = rcColourSrc.w * gcolour[0];
          SDL_BlitSurface(field_colours, &rcColourSrc, screen, &rcColourPos);
        }
      }
    }

    /* ===== Update: Insert at position. ==================================== */
    if (confirm)
    {
      game.insert(index, gcolour[0]);

      /* Check, if something is won and add score and lamas.*/
      if (DEBUG) std::cout << "- check score and lamas." << std::endl;
      pattern = game.search_patterns();  // new vector<FieldPattern>()
      while (pattern->size())
      {
        for (FieldPattern p : *pattern)
        {
          tmp_score = score_for(p);

          score[turn_player0] += tmp_score;

          /* Visual/Pretty Show every pattern. */
          // TODO lamas move
          // TODO pattern highlight

          // if the other still has a lama, get that lama.
          lamas[(int) turn_player0] += lamas[(int) !turn_player0] ? 1 : 0;
          lamas[(int) !turn_player0] -=lamas[(int) !turn_player0] ? 1 : 0;
        }
        game.remove_patterns(pattern, true);  // delete vector<FieldPattern>()
        pattern = game.search_patterns();  // new vector<FieldPattern>()

        // TODO pretty gravity display?
      }

      /* End move.*/
      gcolour.push_back(rand() % FIELD_FRAMES + 1);  // new game colour.
      gcolour.erase(gcolour.begin());
      turn_player0 = !turn_player0;  // toggle player
      confirm = false;

      /*Display new points.*/
      // TODO

      if (DEBUG) std::cout
        << "Next/Now: Player " << (turn_player0 ? "0" : "1")
        << std::endl
        << " ... "
        << (score[0]) << " (" << lamas[0] << ")"
        << " | "
        << (score[1]) << " (" << lamas[1] << ")"
        << std::endl
        ;

      /* Define end of current game.*/
      if (!lamas[0])
      {
        std::cout << "Player 0 lost." << std::endl;
      }
      else if (!lamas[1])
      {
        std::cout << "Player 1 lost." << std::endl;
      }
    }

    /* ===== Draw points and lamas. ========================================= */
    rcLamaPos.x = 0;
    rcLamaPos.y = starting_low + rcLamaSrc.h;

    rcNumPos.y = 10;
    rcNumPos.x = 10 + 5*rcNumSrc.w;  // it will grow to the left.
    display_points(score[0], numbers, &rcNumSrc, screen, &rcNumPos);

    rcNumPos.x = SCREEN_WIDTH - 10;  // it will grow to the left.
    display_points(score[1], numbers, &rcNumSrc, screen, &rcNumPos);

    // Draw lively lamas.
    if (time(NULL) - last_update > 0)
    {
      rcLamaSrc.x = (rcLamaSrc.x + rcLamaSrc.w) % (LAMA_SIZE * LAMA_FRAMES);
      last_update = time(NULL);
    }
    SDL_BlitSurface(lama, &rcLamaSrc, screen, &rcLamaPos);

    SDL_BlitSurface(bg, NULL, screen, &rcBGPos);

    SDL_UpdateRect(screen, 0, 0, 0, 0); // update screen.
  }

  std::cout << "Free bg." << std::endl;
  SDL_FreeSurface(bg);
  std::cout << "Free lama." << std::endl;
  SDL_FreeSurface(lama);
  std::cout << "Free colours." << std::endl;
  SDL_FreeSurface(field_colours);
  std::cout << "Free numbers." << std::endl;
  SDL_FreeSurface(numbers);

  if (pattern) delete pattern;

  SDL_Quit();

  return 0;
}

#endif
