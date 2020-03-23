#ifndef _LAMA_H
#define _LAMA_H

class Lama
{
  private:
    bool player0 = true;  // is part of player 0 team
    int pos = 0;  // is positioned at that x coordinate
    int goal_pos = -1;  // if goal_pos is not -1, the lama will walk there.

  public:

    /* Create a new lama. */
    Lama(bool p0 = true, int x = 0)
      : player0(p0), pos(x), goal_pos(-1)
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
        if (this->goal_pos < this->pos != to_the_left)
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

    /* Return, where the lama is currently standing. */
    int where()
    {
      return this->pos;
    }

    /* Check, if the lama is walking.*/
    bool is_walking()
    {
      return this->goal_pos >= 0;
    }

    /* Set the goal, the lama should walk to. If goal is -1, it will stop. */
    void set_goal(int goal_pos)
    {
      this->goal_pos = goal_pos < 0 ? -1 : goal_pos;
    }


    /* Return, if the player is currently part of player 0's teaml. */
    int from_player0()
    {
      return this->player0;
    }

    void set_player0(bool p0 = true)
    {
      this->player0 = p0;
    }

    void reset(bool player0 = true, int pos = 0)
    {
      this->player0 = player0;
      this->pos = pos;
      this->goal_pos = -1;
    }
};

#endif
