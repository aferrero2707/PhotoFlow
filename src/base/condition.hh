/* 
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#ifndef PF_CONDITION_H
#define PF_CONDITION_H

#include <glib.h>
#include <vips/vips.h>


namespace PF
{

class Condition
{
  GMutex* mutex;
  GCond* condition;
  bool signaled;

public:
  Condition(): signaled(false)
  {
    mutex = vips_g_mutex_new();
    condition = vips_g_cond_new();
  }

  ~Condition()
  {
    vips_g_mutex_free(mutex);
    vips_g_cond_free(condition);
  }

  /*void lock()
  {
    g_mutex_lock( mutex );
  }

  void unlock()
  {
    g_mutex_unlock( mutex );
  }*/

  void reset()
  {
    g_mutex_lock( mutex );
    signaled = false;
    g_mutex_unlock( mutex );
  }

  void signal()
  {
    g_mutex_lock( mutex );
    signaled = true;
    g_cond_signal( condition );
    g_mutex_unlock( mutex );
  }

  void wait()
  {
    g_mutex_lock( mutex );
    //signaled = false;
    while( !signaled )
      g_cond_wait( condition, mutex );
    signaled = false;
    //if(unlk)
    g_mutex_unlock( mutex );
  }
};

}

#endif
