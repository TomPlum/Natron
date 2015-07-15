//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */

#ifndef NATRON_ENGINE_TEXTURERECT_H_
#define NATRON_ENGINE_TEXTURERECT_H_

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include <algorithm> // min, max

#include "Engine/Rect.h"

/** @class This class describes the rectangle (or portion) of an image that is contained
 * into a texture. x1,y1,x2,y2 are respectivly the image coordinates of the left,bottom,right,top
 * edges of the texture. w,h are the width and height of the texture. Note that x2 - x1 != w
 * and likewise y2 - y1 != h , this is because a texture might not contain all the lines/columns
 * of the image in the portion defined.
 **/
struct TextureRect
{
    TextureRect()
        : x1(0)
          , y1(0)
          , x2(0)
          , y2(0)
          , w(0)
          , h(0)
          , closestPo2(1)
          , par(1.)
    {
    }

    TextureRect(int x1_,
                int y1_,
                int x2_,
                int y2_,
                int w_,
                int h_,
                int closestPo2_,
                double par_)
        : x1(x1_)
          , y1(y1_)
          , x2(x2_)
          , y2(y2_)
          , w(w_)
          , h(h_)
          , closestPo2(closestPo2_)
          , par(par_)
    {
    }

    void set(int x1_,
             int y1_,
             int x2_,
             int y2_,
             int w_,
             int h_,
             int closestPo2_,
             double par_)
    {
        x1 = x1_;
        y1 = y1_;
        x2 = x2_;
        y2 = y2_;
        w = w_;
        h = h_;
        closestPo2 = closestPo2_;
        par = par_;
    }

    void reset()
    {
        set(0, 0, 0, 0, 0, 0, 1, 1.);
    }

    int x1,y1,x2,y2; // the edges of the texture. These are coordinates in the full size image
    int w,h; // the width and height of the texture. This has nothing to do with x,y,r,t
    int closestPo2; //< the closest power of 2 of the original region of interest of the image
    double par; // the par of the associated image
    
    bool isNull() const
    {
        return (x2 <= x1) || (y2 <= y1);
    }

    bool intersect(const RectI & r,
                   RectI* intersection) const
    {
        if ( isNull() || r.isNull() ) {
            return false;
        }

        if ( (x1 > r.x2) || (r.x1 > x2) || (y1 > r.y2) || (r.y1 > y2) ) {
            return false;
        }

        intersection->x1 = std::max(x1,r.x1);
        intersection->x2 = std::min(x2,r.x2);
        intersection->y1 = std::max(y1,r.y1);
        intersection->y2 = std::min(y2,r.y2);

        return true;
    }
    
    bool contains(const TextureRect& other) const
    {
        return other.x1 >= x1 &&
        other.y1 >= y1 &&
        other.x2 <= x2 &&
        other.y2 <= y2;
    }
};

inline bool
operator==(const TextureRect & first,
           const TextureRect & second)
{
    return first.x1 == second.x1 &&
           first.y1 == second.y1 &&
           first.x2 == second.x2 &&
           first.y2 == second.y2 &&
           first.w == second.w &&
           first.h == second.h &&
           first.closestPo2 == second.closestPo2 &&
           first.par == second.par;
}

inline bool
operator!=(const TextureRect & first,
           const TextureRect & second)
{
    return !(first == second);
}

#endif // NATRON_ENGINE_TEXTURERECT_H_
