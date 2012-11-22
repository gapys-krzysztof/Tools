#ifndef __VIDEO_HPP__
#define __VIDEO_HPP__

#include <vector>

#include "Bitmap.hpp"
#include "Rect.hpp"

void ShowBitmap( Bitmap* bmp, const std::vector<Rect>& rects, const std::vector<DupRect>& dupes );

#endif
