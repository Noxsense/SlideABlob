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

const bool debug = false;

int start_window(int rows, int cols, int time_per_turn = 15)
{
  SDL_Surface *screen, *tmp, *lama, *bg, *field_colour;
  SDL_Rect rcLamaPos, rcLamaSrc, rcColourPos, rcColourSrc, rcBG;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption("Slide a Lama (Clone)", "Slide a Lama by Nox");

  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_WIDTH, 0, 0);
  SDL_EnableKeyRepeat(70, 70); // set keyboard repeat.

  // ----

  int ck; // colour key for alpha

  /** Load Lama. */
  if ((tmp = SDL_LoadBMP("res/llama_eat_0.bmp")) == NULL)
  {
    std::cerr << "Resource (Lama) not loaded." << std::endl;
    SDL_Quit();
    return -1;
  }
  lama = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  ck = SDL_MapRGB(screen->format, 0xff,0x0,0xff);  // pink
  SDL_SetColorKey(lama, SDL_SRCCOLORKEY | SDL_RLEACCEL, ck);

  /* Define frame and sprite from texture.*/
  rcLamaSrc.w = LAMA_SIZE;
  rcLamaSrc.h = LAMA_SIZE;
  rcLamaSrc.x = 0 * rcLamaSrc.w;
  rcLamaSrc.y = 3 * rcLamaSrc.h;

  /** Load Field colour. */
  if ((tmp = SDL_LoadBMP("res/field_colours.bmp")) == NULL)
  {
    std::cerr << "Resource (Field Colour) not loaded." << std::endl;
    SDL_Quit();
    return -1;
  }
  field_colour = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  /* Define frame and sprite from texture.*/
  rcColourSrc.w = FIELD_SIZE;
  rcColourSrc.h = FIELD_SIZE;
  rcColourSrc.x = 0 * rcColourSrc.w;
  rcColourSrc.y = 0 * rcColourSrc.w;

  if ((tmp = SDL_LoadBMP("res/bg_tile.bmp")) == NULL)
  {
    std::cerr << "Resource (Background) not loaded." << std::endl;
    SDL_Quit();
    return -1;
  }
  bg = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  rcBG.x = 0;
  rcBG.y = 0;

  // ----

  bool window_open = true, confirm = false;
  bool turn_player0 = true;  // indicates, it is player 0's turn
  int index = 0;
  int colour = 1;
  int offset = 4, left_aligned, starting_low, gcolour;

  int outer_indices;  // small offset
  outer_indices = 2 * rows + cols;
  gcolour = rand() % FIELD_FRAMES + 1;

  left_aligned = (2) * (rcColourSrc.w + offset);
  starting_low = (2 + rows) * (rcColourSrc.h + offset);

  SDL_Event event;

  Game game(rows, cols);  // classical 4x4
  game.start();

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
              if (debug) std::cout << "Confirm (" << index << ")" << std::endl;
              confirm = true;
              break;

            case SDLK_RIGHT:
              if (!confirm && index < outer_indices - 1)
                index += 1;
              if (debug) std::cout << "Increase, now " << index << std::endl;
              update = true;
              break;

            case SDLK_LEFT:
              if (!confirm && 0 < index)
                index -= 1;
              if (debug) std::cout << "Decrease, now " << index << std::endl;
              update = true;
              break;
          }
          break;
      }
    }

    /* ======= Draw. === */

    SDL_FillRect(screen, NULL, 0xffffff); // fill white.

    rcColourPos.x = left_aligned;
    rcColourPos.y = starting_low;

    /* Indicate chosen position.*/
    if (!confirm)
    {
      int left = index < rows ? index : -1;
      int top =  index >= rows && index < rows + cols ? index - rows : -1;
      int right = index >= rows + cols && index < 2*rows + cols
        ? rows - index + cols + rows - 1 : -1;

      rcColourPos.x  // --
        = (right >= 0 ? cols : top >= 0 ? top : -1)
        * (rcColourSrc.w + offset) + left_aligned;

      rcColourPos.y  // |
        = -(top < 0 ? (left < 0 ? right : left) : rows)
        * (rcColourSrc.h + offset)
        + starting_low;

      rcColourSrc.x = rcColourSrc.w * gcolour;
      SDL_BlitSurface(field_colour, &rcColourSrc, screen, &rcColourPos);
    }
    /* Update: Insert at position. */
    else
    {
      game.insert(index, gcolour);

      /* Check, if something is won and add score and lamas.*/
      if (debug) std::cout << "- check score and lamas." << std::endl;

      /* End move.*/
      gcolour = rand() % FIELD_FRAMES + 1;  // new game colour.
      turn_player0 = !turn_player0;  // toggle player
      confirm = false;  // TODO later.
    }

    rcColourPos.x = left_aligned;
    rcColourPos.y = starting_low;

    for (int i = 0; i < game.get_size(); i ++)
    {
      rcColourSrc.x = rcColourSrc.w * game.colour_at(i);
      SDL_BlitSurface(field_colour, &rcColourSrc, screen, &rcColourPos);

      // next position.
      if (i > 0 && (i % game.get_cols()) == game.get_cols() - 1)
      {
        rcColourPos.x = left_aligned;  // reset x
        rcColourPos.y -= rcColourSrc.h + offset;  // grow up
      }
      else
      {
        rcColourPos.x += rcColourSrc.w + offset;  // left to right
      }
    }

    rcLamaPos.x = 0;
    rcLamaPos.y = starting_low + rcLamaSrc.h;

    if (time(NULL) - last_update > 0)
    {
      rcLamaSrc.x = (rcLamaSrc.x + rcLamaSrc.w) % (LAMA_SIZE * LAMA_FRAMES);
      last_update = time(NULL);
    }

    SDL_BlitSurface(lama, &rcLamaSrc, screen, &rcLamaPos);

    SDL_UpdateRect(screen, 0, 0, 0, 0); // update screen.
  }

  SDL_FreeSurface(lama);
  SDL_FreeSurface(bg);
  SDL_FreeSurface(field_colour);

  SDL_Quit();

  return 0;
}

#endif
