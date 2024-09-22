#include "selection.h"

void Selection::activate(int x, int y) {
  xStart = x;
  xEnd = x;
  yStart = y;
  yEnd = y;
  active = true;
}
void Selection::stop() { active = false; }
void Selection::diffX(int diff) {
  if (!active)
    return;
  xEnd = diff;
}
void Selection::diffY(int diff) {
  if (!active)
    return;
  yEnd = diff;
}
void Selection::diff(int x, int y) {
  if (!active)
    return;
  xEnd = x;
  yEnd = y;
}
bool Selection::isLineIncluded(int ySearch) {
  if (yEnd < yStart)
    return ySearch >= yEnd && ySearch <= yStart;
  if (yEnd > yStart)
    return ySearch >= yStart && ySearch <= yEnd;
  return ySearch == yStart;
}
int Selection::getYSmaller() { return yEnd < yStart ? yEnd : yStart; }
int Selection::getYBigger() { return yStart > yEnd ? yStart : yEnd; }
int Selection::getYStart() { return yStart; }
int Selection::getYEnd() { return yEnd; }
bool Selection::isSame() { return active && yStart == yEnd && xStart == xEnd; }
int Selection::getXSmaller() { return xEnd < xStart ? xEnd : xStart; }
int Selection::getXBigger() { return xStart > xEnd ? xStart : xEnd; }
int Selection::getXStart() { return xStart; }
int Selection::getXEnd() { return xEnd; }