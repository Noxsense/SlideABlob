#ifndef _BLOB_GUI_HANDLER_H_
#define _BLOA_GUI_HANDLER_H_

#include <vector>
#include "SDL.h"

#define SET_IDLE 0
#define SET_WALK_RIGHT 1
#define SET_WALK_LEFT 2

int random(int,int);

class Blob
{
  private:
    bool player1 = false;  // is part of player 0 team
    int pos = 0;  // is positioned at that x coordinate
    int goal_pos = -1;  // if goal_pos is not -1, the blob will walk there.

  public:

    /* Create a new blob. */
    Blob(bool p1 = false, int x = 0)
      : player1(p1), pos(x), goal_pos(-1)
    {
      this->goal_pos = -1;
    }

    /* Walk towards the goal position. */
    void walk(int velocity = 1)
    {
      if (velocity < 0) return walk(-velocity);  // use positive number

      if (goal_pos >= 0)
      {
        bool to_the_left = this->goal_pos < this->pos;
        this->pos += goal_pos < pos ? -velocity : velocity;

        // goal and current relation changed; goal is reached.
        if ((this->goal_pos < this->pos) != to_the_left)
        {
          this->goal_pos = -1;
        }
      }
    }

    /* Stop walking, set goal to -1.*/
    void stop()
    {
      return set_goal(-1);
    }

    /* Return, where the blob is currently standing. */
    int where()
    {
      return this->pos;
    }

    /* Check, if the blob is walking.*/
    bool is_walking()
    {
      return this->goal_pos >= 0 && this->goal_pos != this->pos;
    }

    /* If it is waling, check if it's walking to the left.*/
    bool to_the_left()
    {
      return is_walking() && this->goal_pos < this->pos;
    }

    /* If it is waling, check if it's walking to the right.*/
    bool to_the_right()
    {
      return is_walking() && this->goal_pos > this->pos;
    }

    /* Set the goal, the blob should walk to. If goal is -1, it will stop. */
    void set_goal(int goal_pos)
    {
      this->goal_pos = goal_pos < 0 ? -1 : goal_pos;
    }

    /* Return 0/false, if player 0 is oner of blob, else 1 / true. */
    bool get_player()
    {
      return this->player1;
    }

    void set_player(bool p1)
    {
      this->player1 = p1;
    }

    void reset(bool player1 = false, int pos = 0)
    {
      this->player1 = player1;
      this->pos = pos < 0 ? 0 : pos;
      this->goal_pos = -1;
    }
};

class BlobGuiHandler
{
//-----------------------------------------------------------------------------
  private:
    std::vector<Blob> blobs;
    std::vector<int> blobs_frame_set;
    std::vector<int> blobs_frame;

    int mid, bounds;  // position, where the blobs are divided.
    int velocity;  // pixel, the blob moves per step.

    int max_frames_y;
    int max_frames_x;

    SDL_Rect recBlobSrc, recBlobPos;
    SDL_Surface *surfBlob;
//-----------------------------------------------------------------------------
  public:

    /* Create a new set of blobs, counting n.*/
    BlobGuiHandler(int mid, int bounds = 32, int n = 10)
    {
      blobs.clear();
      blobs_frame.clear();
      blobs_frame_set.clear();

      this->velocity = 1;
      this->mid = mid;
      this->bounds = bounds;

      bool p1;
      int start_position;  // around their corrisponding camp.

      for (int i = 0; i < n; i++)
      {
        p1 = i % 2;

        start_position
          = random(bounds + (p1 ? mid : 0), (p1 ? mid * 2: mid) - bounds);

        blobs.push_back(Blob(p1, start_position));
        blobs_frame.push_back(0);
        blobs_frame_set.push_back(0);
      }
    }

    /* Reset all blobs.*/
    void reset_all()
    {
      bool p1;

      int start_position;  // around their corrisponding camp.

      for (long unsigned int i = 0; i < blobs.size(); i++)
      {
        p1 = i % 2;

        start_position
        = random(bounds + (p1 ? mid : 0), (p1 ? mid * 2: mid) - bounds);

        blobs[i].reset(p1, start_position);
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

      this->bounds = width;

      this->max_frames_x = max_frames_x;
      this->max_frames_y = max_frames_y;

      this->update_all_blobs(true);  // start positiona and colour.

    }  // end set_texture(SDL_Surface, int, int, int, int)

    /* Set blob for player1; return it's final blob count.*/
    int new_blob_for_player(bool p1)
    {
      int blob_count = 0;
      int mover = -1;

      for (long unsigned int i = 0; i < blobs.size(); i++)
      {
        if (blobs[i].get_player() == p1)  // is already from p1
        {
          blob_count += 1;
          continue;
        }
        else if (mover < 0)  // not from p1, not yet updated.
        {
          mover = i;  // update.
        }
      }

      if (mover >= 0)
      {
        int goal = random(bounds + (p1 ? mid : 0), (p1 ? mid * 2: mid) - bounds);

        blobs[mover].set_player(p1);
        blobs[mover].set_goal(goal);

        blob_count += 1;
      }

      return blob_count;
    } // int new_blob_for_player(bool player1)

    /* Get the number of all blobs handled by the handler.*/
    int max_blobs()
    {
      return this->blobs.size();
    }

    /* Count blobs for a player.*/
    int count_blobs(bool p1)
    {
      int n = 0;
      for (Blob l : blobs)
      {
        n += (l.get_player() == p1);
      }
      return n;
    }  // end cound_blobs(bool)

    void set_velocity(int velocity)
    {
      this->velocity = velocity < 1 ? 1 : velocity;
    }

    /* Select next frame and if needed, updated position.*/
    void update_blob(long unsigned int blob, bool random = false)
    {
      if (blob >= blobs.size())
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

      /* Colour depends on side of the blob. */
      blobs_frame_set[blob]
        = blobs[blob].where() < mid
        ? set % 3
        : (set % 3) + 3;;

    }  // end of update_blob(int);

    /* Update all blobs: Select frame and maybe update position.*/
    void update_all_blobs(bool random = false)
    {
      for (long unsigned int i = 0; i < blobs.size(); i++)
      {
        update_blob(i, random);
      }
    }  // end update_all_blobs(bool)

    /* Draw a blob in it's current moment. */
    void draw_blob(long unsigned int blob, SDL_Surface *screen, int pos_y = 0)
    {
      if (blob >= blobs.size())
        return;

      if (!screen)
        return;

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
      for (long unsigned int i = 0; i < blobs.size(); i++)
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
