//  Natron
//
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Rect.h"

#include <algorithm> // min, max

#define MINAREA64 4096  // = 4096 pixels (=64*64)
#define MINAREA128 16384
#define MINAREA256 65536
#define MINAREA MINAREA128 // minimum rectangle area

/// if splitCount is zero, this function returns a set of less than area()/MINAREA rects which are no smaller than MINAREA
std::vector<RectI> RectI::splitIntoSmallerRects(int splitsCount) const
{
    std::vector<RectI> ret;

    if ( isNull() ) {
        return ret;
    }
#ifdef NATRON_SPLITRECT_SCANLINE
    int averagePixelsPerSplit = std::ceil(double( area() ) / (double)splitsCount);
    /*if the splits happen to have less pixels than 1 scan-line contains, just do scan-line rendering*/
    if ( averagePixelsPerSplit < width() ) {
        for (int i = bottom(); i < top(); ++i) {
            ret.push_back( RectI(left(), i, right(), i + 1) );
        }
    } else {
        //we round to the ceil
        int scanLinesCount = std::ceil( (double)averagePixelsPerSplit / (double)width() );
        int startBox = bottom();
        while (startBox < top() - scanLinesCount) {
            ret.push_back( RectI(left(), startBox, right(), startBox + scanLinesCount) );
            startBox += scanLinesCount;
        }
        if ( startBox < top() ) {
            ret.push_back( RectI( left(), startBox, right(), top() ) );
        }
    }
#else
    // make sure there are at least MINAREA pixels (=128*128) per rect, and the rects are as square as possible.
    // This minimizes the overlapping areas between rendered regions
    if (area() <= MINAREA) {
        ret.push_back(*this);
    } else {
        // autocompute splitsCount
        if (!splitsCount) {
            splitsCount = area() / MINAREA;
        }
        //printf("splitsCount=%d\n", splitsCount);
        // the average rect area
        double avgArea = area() / (double)splitsCount;
        //printf("avgArea=%g,sqrt=%g\n", avgArea, sqrt(avgArea));
        bool landscape = width() > height();
        int dim1 = landscape ? width() : height();
        int dim2 = landscape ? height() : width();
        //printf("dim1=%d\n", dim1);
        //printf("dim2=%d\n", dim2);

        int num1 = (int)(std::ceil(dim1 / std::sqrt(avgArea)));
        assert(num1 > 0);
        //printf("num1=%d\n", num1);
        int num2 = std::max(1, std::min(splitsCount / num1, dim2/(MINAREA/(dim1/num1)))); // integer division
        assert(num1 >= num2);
        //printf("num2=%d\n", num2);
        num1 = std::max(1, std::min(splitsCount / num2, dim1/(1+(MINAREA-1)/(dim2/num2))));
        //printf("num1=%d\n", num1);
        assert(splitsCount >= num1 * num2);
        assert((dim1/num1)*(dim2/num2) >= MINAREA);
        int numRows = landscape ? num2 : num1;
        int numCols = landscape ? num1 : num2;
        for (int i = numRows - 1; i >= 0; --i) {
            int y1_ = bottom() + i     * height() / numRows;
            int y2_ = bottom() + (i+1) * height() / numRows;
            for (int j = 0; j < numCols; ++j) {
                int x1_ = left() + j     * width() / numCols;
                int x2_ = left() + (j+1) * width() / numCols;
                //printf("x1_=%d,x2_=%d,y1_=%d,y2_=%d\n",x1_,x2_,y1_,y2_);
                assert((x2_-x1_)*(y2_-y1_) >= MINAREA);
                //printf("area is %d\n", (x2_-x1_)*(y2_-y1_));
                ret.push_back( RectI(x1_, y1_, x2_, y2_) );
            }
        }
    }
#endif

    return ret;
}
