#ifndef POINT_H
#define POINT_H

#define P(_x, _y) ((Point){.x = _x, .y = _y})
typedef struct point_t {
  int x, y;
} Point;

#define point_isvert(p) (abs((p).x) < abs((p).y))
#define point_move(p, d) (P((p).x + (d).x, (p).y + (d).y))
#define point_move_xy(p, _x, _y) (P((p).x + (_x), (p).y + (_y)))

Point point_delta(Point p1, Point p2);
Point point_clamp(Point p, Point min, Point max);
Point point_lerp(Point from, Point to, float t);
float point_dist(Point p0, Point p1);
void point_swap(Point *p0, Point *p1);
Point point_swap_axis(Point p0);
float point_slop(Point p0, Point p1);
float point_delta_slop(Point delta);

#endif
