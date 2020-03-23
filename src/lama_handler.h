#ifndef _LAMA_GUI_HANDLER_H
#define _LAMA_GUI_HANDLER_H

#include <vector>
#include "SDL.h"

#include "lama.h"

class LamaGuiHandler
{
//-----------------------------------------------------------------------------
  private:
    std::vector<Lama> lamas;
    std::vector<int> lamas_frame;
    int seperator;  // position, where the lamas are divided.

    int max_frames_y;
    int max_frames_x;

    SDL_Rect recLamaSrc, recLamaPos;
    SDL_Surface *surfLama;
//-----------------------------------------------------------------------------
  public:

    /* Create a new set of lamas, counting n.*/
    LamaGuiHandler(int seperator, int n = 10)
      : seperator(seperator)
    {
      lamas.clear();
      lamas_frame.clear();

      bool p0;

      for (int i = 0; i < n; i++)
      {
        p0 = i % 2;
        lamas.push_back(Lama(p0, p0 * seperator + (rand() % seperator)));
        lamas_frame.push_back(0);
      }
    }

    /* Reset all lamas.*/
    void reset_all()
    {
      bool p0;

      for (int i = 0; i < lamas.size(); i++)
      {
        p0 = i % 2;
        lamas[i].reset(p0, p0 * seperator + (rand() % seperator));
        lamas_frame[i] = 0;
      }
    }  // end of reset_all()

    /* Set texture and maximal frames for a lama.*/
    void set_texture(SDL_Surface *surf,
        int width, int height = 0,
        int max_frames_x = 1, int max_frames_y = 1)
    {
      surfLama = surf;

      recLamaSrc.x = 0;
      recLamaSrc.y = 0;
      recLamaSrc.w = width;
      recLamaSrc.h = height > 0 ? height : width;  // square.

      recLamaPos.x = 0;
      recLamaPos.y = 0;

      this->max_frames_x = max_frames_x;
      this->max_frames_y = max_frames_y;
    }  // end set_texture(SDL_Surface, int, int, int, int)


    /* Set lama for player0; return it's final lama count.*/
    int new_lama_for_player(bool player0)
    {
      int lama_count = 0;
      bool updated = false;

      for (int i = 0; i < lamas.size(); i++)
      {
        if (lamas[i].from_player0() == player0)  // is already from p0
        {
          lama_count += 1;
          continue;
        }
        else if (!updated)  // not from p0, not yet updated.
        {
          lama_count += 1;
          lamas[i].set_player0(player0);
          lamas[i].set_goal(
              this->seperator*player0
              + rand() % (this->seperator / 2));
          updated = true;
        }
      }

      return lama_count;
    } // int new_lama_for_player(bool player0)

    /* Count lamas for a player.*/
    int count_lamas(bool player0)
    {
      int n = 0;
      for (Lama l : lamas)
      {
        n += (l.from_player0() == player0);
      }
      return n;
    }  // end cound_lamas(bool)

    /* Select next frame and if needed, updated position.*/
    void update_lama(int lama, bool random = false)
    {
      if (lama < 0 || lama >= lamas.size())
        return;  // invalid setting.

      int frame = lamas_frame[lama] % max_frames_x;
      int set = lamas_frame[lama] / max_frames_x;

      // select random or circular next frame.
      frame = (random ? rand() : (frame + 1)) % max_frames_x;

      // if walking, start another set, 
      if (lamas[lama].is_walking())
      {
        lamas[lama].walk(1);
        if (set != 1)
        {
          set = 1;
          frame = random ? frame : 0;
        }
      }

      lamas_frame[lama] = set*max_frames_x + frame;
    }  // end of update_lama(int);

    /* Update all lamas: Select frame and maybe update position.*/
    void update_all_lamas(bool random = false)
    {
      for (int i = 0; i < lamas.size(); i++)
      {
        update_lama(i, random);
      }
    }  // end update_all_lamas(bool)

    /* Draw a lama in it's current moment. */
    void draw_lama(int lama, SDL_Surface *screen, int pos_y = 0)
    {
      if (lama < 0 || lama >= lamas.size())
        return;

      if (!screen)
        return;

      int set = lamas_frame[lama] / max_frames_x;
      int frame = lamas_frame[lama] % max_frames_x;

      recLamaSrc.x = frame * recLamaSrc.w;
      recLamaSrc.y = set * recLamaSrc.h;

      recLamaPos.x = lamas[lama].where();
      recLamaPos.y = pos_y - recLamaSrc.h;

      SDL_BlitSurface(surfLama, &recLamaSrc, screen, &recLamaPos);
    }  // draw_lama(int, SDL_Surface*)

    /* Draw all lamas in it's current moment.*/
    void draw_all_lamas(SDL_Surface *screen, int pos_y = 0)
    {
      for (int i = 0; i < lamas.size(); i++)
      {
        draw_lama(i, screen, pos_y);
      }
    }  // end draw_all_lamas(SDL_Surface*)
//-----------------------------------------------------------------------------
};  // end of class LamaGuiHandler

#endif
