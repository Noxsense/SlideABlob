#ifndef _BLOB_GUI_HANDLER_H_
#define _BLOA_GUI_HANDLER_H_

#include <vector>
#include "SDL.h"

#include "blob.h"

#define SET_IDLE 0
#define SET_WALK_RIGHT 1
#define SET_WALK_LEFT 2

int random(int,int);

class BlobGuiHandler
{
//-----------------------------------------------------------------------------
  private:
    std::vector<Blob> blobs;
    std::vector<int> blobs_frame_set;
    std::vector<int> blobs_frame;

    int camp[2];  // position, where the blobs are divided.
    int velocity;  // pixel, the blob moves per step.

    int max_frames_y;
    int max_frames_x;

    SDL_Rect recBlobSrc, recBlobPos;
    SDL_Surface *surfBlob;
//-----------------------------------------------------------------------------
  public:

    /* Create a new set of blobs, counting n.*/
    BlobGuiHandler(int camp0, int camp1, int n = 10)
    {
      blobs.clear();
      blobs_frame.clear();
      blobs_frame_set.clear();

      this->velocity = 1;
      this->camp[0] = camp0;
      this->camp[1] = camp1;

      bool p0;
      int start_position;  // around their corrisponding camp.
      int offset = n/4 * 32;

      for (int i = 0; i < n; i++)
      {
        p0 = i % 2;

        start_position = camp[p0] + random(-offset, offset);

        blobs.push_back(Blob(p0, start_position));
        blobs_frame.push_back(0);
        blobs_frame_set.push_back(0);
      }
    }

    /* Reset all blobs.*/
    void reset_all()
    {
      bool p0;
      int n = max_blobs();
      int offset = n/2 * (recBlobSrc.w < 1 ? 32 : recBlobSrc.w);

      for (int i = 0; i < blobs.size(); i++)
      {
        p0 = i % 2;
        blobs[i].reset(p0, camp[p0] + random(-offset, offset));
        blobs_frame[i] = 0;
        blobs_frame_set[i] = 0;
      }
    }  // end of reset_all()

    /* Set texture and maximal frames for a blob.*/
    void set_texture(SDL_Surface *surf,
        int width, int height = 0,
        int max_frames_x = 1, int max_frames_y = 1)
    {
      surfBlob = surf;

      recBlobSrc.x = 0;
      recBlobSrc.y = 0;
      recBlobSrc.w = width;
      recBlobSrc.h = height > 0 ? height : width;  // square.

      recBlobPos.x = 0;
      recBlobPos.y = 0;

      this->max_frames_x = max_frames_x;
      this->max_frames_y = max_frames_y;
    }  // end set_texture(SDL_Surface, int, int, int, int)


    /* Set blob for player0; return it's final blob count.*/
    int new_blob_for_player(bool p0)
    {
      int blob_count = 0;
      int mover = -1;
      int n = blobs.size();
      int offset = recBlobSrc.w < 1 ? 10 : recBlobSrc.w / 3;

      for (int i = 0; i < blobs.size(); i++)
      {
        if (blobs[i].from_player0() == p0)  // is already from p0
        {
          blob_count += 1;
          continue;
        }
        else if (mover < 0)  // not from p0, not yet updated.
        {
          mover = i;
        }
      }

      if (mover >= 0)
      {
        int goal
          = camp[p0] + (p0 ? blob_count - 1 : -blob_count)*offset
          + random(-offset, offset);

        blobs[mover].set_player0(p0);
        blobs[mover].set_goal(goal);

        blob_count += 1;
      }

      return blob_count;
    } // int new_blob_for_player(bool player0)

    /* Get the number of all blobs handled by the handler.*/
    int max_blobs()
    {
      return this->blobs.size();
    }

    /* Count blobs for a player.*/
    int count_blobs(bool p0)
    {
      int n = 0;
      for (Blob l : blobs)
      {
        n += (l.from_player0() == p0);
      }
      return n;
    }  // end cound_blobs(bool)

    void set_velocity(int velocity)
    {
      this->velocity = velocity < 1 ? 1 : velocity;
    }

    /* Select next frame and if needed, updated position.*/
    void update_blob(int blob, bool random = false)
    {
      if (blob < 0 || blob >= blobs.size())
        return;  // invalid setting.

      int set = blobs_frame_set[blob];

      // if walking, start another set, 
      if (blobs[blob].is_walking())
      {
        blobs[blob].walk(this->velocity);

        // select circular next frame.
        blobs_frame[blob] = (blobs_frame[blob] + 1) % max_frames_x;

        if (blobs[blob].to_the_left())
        {
          set = SET_WALK_LEFT;
        }
        else
        {
          set = SET_WALK_RIGHT;
        }
      }
      else
      {
        set = SET_IDLE;

        // select random or circular next frame.
        blobs_frame[blob]
          = (random ? rand() : (blobs_frame[blob] + 1)) % max_frames_x;
      }

      int mid = (camp[0] + camp[1]) / 2;

      /* Colour depends on side of the blob. */

      blobs_frame_set[blob]
        = blobs[blob].where() < mid
        ? set % 3
        : (set % 3) + 3;;

    }  // end of update_blob(int);

    /* Update all blobs: Select frame and maybe update position.*/
    void update_all_blobs(bool random = false)
    {
      for (int i = 0; i < blobs.size(); i++)
      {
        update_blob(i, random);
      }
    }  // end update_all_blobs(bool)

    /* Draw a blob in it's current moment. */
    void draw_blob(int blob, SDL_Surface *screen, int pos_y = 0)
    {
      if (blob < 0 || blob >= blobs.size())
        return;

      if (!screen)
        return;

      int frame = blobs_frame[blob] % max_frames_x;

      recBlobSrc.x = blobs_frame[blob] * recBlobSrc.w;
      recBlobSrc.y = blobs_frame_set[blob] * recBlobSrc.h;

      recBlobPos.x = blobs[blob].where();
      // recBlobPos.y = pos_y - (pos_y < recBlobSrc.h ? 0 : recBlobSrc.h);
      recBlobPos.y = pos_y;

      SDL_BlitSurface(surfBlob, &recBlobSrc, screen, &recBlobPos);
    }  // draw_blob(int, SDL_Surface*)

    /* Draw all blobs in it's current moment.*/
    void draw_all_blobs(SDL_Surface *screen, int pos_y = 0)
    {
      for (int i = 0; i < blobs.size(); i++)
      {
        draw_blob(i, screen, pos_y);
      }
    }  // end draw_all_blobs(SDL_Surface*)
//-----------------------------------------------------------------------------
};  // end of class BlobGuiHandler

/* Supporting function.*/
int random(int start = 0, int end = 1000)
{
  return end < start ? random(end, start) : start + rand() % (end - start);
}

#endif  // _Blob_GUI_HANDLER_H_
