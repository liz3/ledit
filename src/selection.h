#ifndef SELECTION_H
#define SELECTION_H

class Selection {
 public:
  bool active = false;
  int xStart, xEnd;
  int yStart, yEnd;
  void activate(int x, int y);
  void stop();
  void diffX(int diff);
  void diffY(int diff);
  void diff(int x, int y);
  bool isLineIncluded(int ySearch);
  int getYSmaller();
  int getYBigger();
  int getYStart();
  int getYEnd();
  bool isSame();
  int getXSmaller();
  int getXBigger();
  int getXStart();
  int getXEnd();
};

#endif
