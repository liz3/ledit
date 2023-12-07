#ifndef SELECTION_H
#define SELECTION_H

class Selection {
 public:
  bool active = false;
  int xStart, xEnd;
  int yStart, yEnd;
  void activate(int x, int y) {
    xStart = x;
    xEnd = x;
    yStart =y;
    yEnd = y;
    active = true;
  }
  void stop() {
    active = false;
  }
  void diffX(int diff) {
    if(!active)
      return;
    xEnd = diff;
  }
  void diffY(int diff) {
    if(!active)
      return;
    yEnd = diff;
  }
  void diff(int x, int y) {
    if(!active)
      return;
    xEnd = x;
    yEnd = y;
  }
  bool isLineIncluded(int ySearch) {
    if(yEnd < yStart)
      return ySearch >= yEnd && ySearch <= yStart;
    if(yEnd > yStart)
      return ySearch >= yStart && ySearch <= yEnd;
    return ySearch == yStart;
  }
  int getYSmaller() {
    return yEnd < yStart ? yEnd : yStart;
  }
  int getYBigger() {
    return yStart > yEnd ? yStart : yEnd;
  }
  int getYStart() {
    return yStart;
  }
  int getYEnd() {
    return yEnd;
  }
  bool isSame(){
    return active && yStart == yEnd && xStart == xEnd;
  }
  int getXSmaller() {
    return xEnd < xStart ? xEnd : xStart;
  }
  int getXBigger() {
    return xStart > xEnd ? xStart : xEnd;
  }
  int getXStart() {
    return xStart;
  }
  int getXEnd() {
    return xEnd;
  }

 private:
};

#endif
