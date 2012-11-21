#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <vector>

#include "Bitmap.hpp"
#include "Rect.hpp"

bool IsEmpty( const Rect& rect, Bitmap* bmp );
std::vector<Rect> RemoveEmpty( const std::vector<Rect>& rects, Bitmap* bmp );

Rect CropEmpty( const Rect& rect, Bitmap* bmp );
std::vector<Rect> CropEmpty( const std::vector<Rect>& rects, Bitmap* bmp );

std::vector<Rect> MergeHorizontal( const std::vector<Rect>& rects );
std::vector<Rect> MergeVertical( const std::vector<Rect>& rects );

std::vector<int> CalcBroadDuplicates( const std::vector<Rect>& rects, Bitmap* bmp );
bool AreExactDuplicates( const Rect& rect1, const Rect& rect2, Bitmap* bmp );

#endif
