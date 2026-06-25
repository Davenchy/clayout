#include "include/point.h"
#include <math.h>

Point point_delta(Point p1, Point p2) { return P(p2.x - p1.x, p2.y - p1.y); }

Point point_clamp(Point p, Point min, Point max) {
  return P(p.x < min.x   ? min.x
           : p.x > max.x ? max.x
                         : p.x,
           p.y < min.y   ? min.y
           : p.y > max.y ? max.y
                         : p.y

  );
}

Point point_lerp(Point from, Point to, float t) {
  return P((int)roundf(from.x + (to.x - from.x) * t),
           (int)roundf(from.y + (to.y - from.y) * t));
}

float point_dist(Point p0, Point p1) {
  return sqrtf((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
}

void point_swap(Point *p0, Point *p1) {
  Point tmp = *p0;
  *p0 = *p1;
  *p1 = tmp;
}

Point point_swap_axis(Point p0) { return P(p0.y, p0.x); }

float point_slop(Point p0, Point p1) {
  return point_delta_slop(point_delta(p0, p1));
}

float point_delta_slop(Point delta) {
  if (delta.x == 0)
    return 1.f;
  return (float)delta.y / (float)delta.x;
}
