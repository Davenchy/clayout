#include "include/canvas.h"
#include "include/point.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define pixel_idx(x, y) (((y) * canvas->width + (x)) * 3)
#define blend(old, new, alpha) ((old) * (1 - (alpha)) + (new) * (alpha))
#define lerp(old, new, t) ((old) + (t) * ((new) - (old)))
#define clamp(x, min, max)                                                     \
  (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

#define is_invalid_point(canvas, p)                                            \
  ((p).x < 0 || (p).x >= (canvas)->width || (p).y < 0 ||                       \
   (p).y >= (canvas)->height)
#define canvas_point_clamp(canvas, point)                                      \
  point_clamp(point, (Point){0, 0}, (Point){(canvas)->width, (canvas)->height})
#define arc_point(c, r, a)                                                     \
  (P((c).x + (int)roundf(cos((a)) * (r)), (c).y + (int)roundf(sin((a)) * (r))))

struct canvas_t {
  size_t width;
  size_t height;
  uint8_t *data;
};

Canvas *canvas_init(size_t width, size_t height) {
  Canvas *canvas =
      malloc(width * height * 3 * sizeof(uint8_t) + sizeof(Canvas));
  if (canvas == NULL)
    return NULL;

  canvas->width = width;
  canvas->height = height;
  canvas->data = (uint8_t *)((Canvas *)canvas + 1);

  return canvas;
}

void canvas_destroy(Canvas *canvas) {
  if (canvas == NULL)
    return;

  canvas->width = 0;
  canvas->height = 0;
  canvas->data = NULL;

  free(canvas);
}

bool canvas_write(Canvas *canvas, char *filename) {
  if (canvas == NULL || canvas->data == NULL || filename == NULL)
    return false;

  char header[256] = {0};
  int header_len = snprintf(header, sizeof(header), "P6\n%zu %zu\n255\n",
                            canvas->width, canvas->height);

  FILE *f = fopen(filename, "wb");
  if (f == NULL)
    return false;

  fwrite(header, header_len, 1, f);
  fwrite(canvas->data, canvas->width * canvas->height * 3, 1, f);
  fclose(f);

  return true;
}

void canvas_pixel(Canvas *canvas, Point pixel, Color color) {
  if (canvas == NULL || is_invalid_point(canvas, pixel))
    return;
  int idx = pixel_idx(pixel.x, pixel.y);
  canvas->data[idx + 0] = color.r;
  canvas->data[idx + 1] = color.g;
  canvas->data[idx + 2] = color.b;
}

void canvas_line(Canvas *canvas, Point p0, Point p1, Color color) {
  if (canvas == NULL)
    return;

  p0 = canvas_point_clamp(canvas, p0);
  p1 = canvas_point_clamp(canvas, p1);

  Point delta = P(p1.x - p0.x, p1.y - p0.y);

  int is_vert = point_isvert(delta);
  if (is_vert ? delta.y < 0 : delta.x < 0) {
    point_swap(&p0, &p1);
    delta = point_delta(p0, p1);
  }

  float m = point_delta_slop(is_vert ? point_swap_axis(delta) : delta);
  int steps = is_vert ? delta.y : delta.x;
  for (int i = 0; i <= steps; i++) {
    Point move = is_vert ? P(i * m, i) : P(i, i * m);
    Point p = point_move(p0, move);
    canvas_pixel(canvas, p, color);
  }
}

void canvas_rect(Canvas *canvas, Point p, int w, int h, Color color) {
  canvas_line(canvas, p, P(p.x + w, p.y), color);
  canvas_line(canvas, P(p.x + w, p.y), P(p.x + w, p.y + h), color);
  canvas_line(canvas, P(p.x + w, p.y + h), P(p.x, p.y + h), color);
  canvas_line(canvas, P(p.x, p.y + h), P(p.x, p.y), color);
}

void canvas_rectf(Canvas *canvas, Point p, int w, int h, Color color) {
  if (canvas == NULL || is_invalid_point(canvas, p) ||
      is_invalid_point(canvas, point_move_xy(p, w, h)))
    return;

  Point end = point_move_xy(p, w, h);
  for (int y = p.y; y < end.y; y++)
    for (int x = p.x; x < end.x; x++)
      canvas_pixel(canvas, P(x, y), color);
}

void canvas_arc_segs(Canvas *canvas, Point center, float radius, float angle,
                     float rotate, unsigned int segs, Color color) {
  if (canvas == NULL || is_invalid_point(canvas, center) ||
      is_invalid_point(canvas, point_move_xy(center, radius, radius)))
    return;

  float step = angle / segs;
  for (float a = rotate; a < angle + rotate; a += step) {
    Point p0 = arc_point(center, radius, a);
    Point p1 = arc_point(center, radius, a + step);
    canvas_line(canvas, p0, p1, color);
  }
}

void canvas_arcf(Canvas *canvas, Point center, float radius, float angle,
                 float rotate, Color color) {
  if (canvas == NULL || is_invalid_point(canvas, center) ||
      is_invalid_point(canvas, point_move_xy(center, radius, radius)))
    return;

  float step = angle / arc_segs(radius);
  Point p_init = arc_point(center, radius, rotate);

  for (float a = rotate + step; a < angle + rotate; a += step) {
    Point p0 = arc_point(center, radius, a);
    Point p1 = arc_point(center, radius, a + step);
    canvas_trigf(canvas, p_init, p0, p1, color);
  }

  if (angle < M_PI) {
    Point p = arc_point(center, radius, angle + rotate);
    canvas_trigf(canvas, p_init, center, p, color);
  }
}

void canvas_trig(Canvas *canvas, Point p0, Point p1, Point p2, Color color) {
  canvas_line(canvas, p0, p1, color);
  canvas_line(canvas, p1, p2, color);
  canvas_line(canvas, p2, p0, color);
}

// static Point lerp_point(Point p0, Point p1, float t) {
//   return (Point){
//       .x = (int)roundf(lerp(p0.x, p1.x, t)),
//       .y = (int)roundf(lerp(p0.y, p1.y, t)),
//   };
// }
static int point_max_comp(int count, ...) {
  va_list args;
  int max = 0;
  va_start(args, count);
  for (int i = 0; i < count; i++) {
    Point p = va_arg(args, Point);
    int x = abs(p.x);
    int y = abs(p.y);
    if (x > max)
      max = x;
    if (y > max)
      max = y;
  }
  va_end(args);
  return max;
}

void canvas_trigf(Canvas *canvas, Point p0, Point p1, Point p2, Color color) {
  if (canvas == NULL)
    return;

  int steps = point_max_comp(3, p0, p1, p2);
  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    Point pl0 = point_lerp(p2, p0, t);
    Point pl1 = point_lerp(p2, p1, t);
    canvas_line(canvas, pl0, pl1, color);
  }
}

void canvas_rrect(Canvas *canvas, Point p, int w, int h, float r, Color color) {
  int w2 = w - (int)roundf(r * 2);
  int h2 = h - (int)roundf(r * 2);

  canvas_line(canvas, point_move_xy(p, r, 0), point_move_xy(p, w - r, 0),
              color);
  canvas_line(canvas, point_move_xy(p, r, h), point_move_xy(p, w - r, h),
              color);
  canvas_line(canvas, point_move_xy(p, w, r), point_move_xy(p, w, h - r),
              color);
  canvas_line(canvas, point_move_xy(p, 0, r), point_move_xy(p, 0, h - r),
              color);

  canvas_arc(canvas, point_move_xy(p, r, r), r, M_PI_2, M_PI, color);
  canvas_arc(canvas, point_move_xy(p, w - r, r), r, M_PI_2, M_PI_2 * 3, color);
  canvas_arc(canvas, point_move_xy(p, w - r, h - r), r, M_PI_2, 0, color);
  canvas_arc(canvas, point_move_xy(p, r, h - r), r, M_PI_2, M_PI_2, color);
}

void canvas_rrectf(Canvas *canvas, Point p, int w, int h, float r,
                   Color color) {
  int w2 = w - (int)roundf(r * 2);
  int h2 = h - (int)roundf(r * 2);

  canvas_rectf(canvas, point_move_xy(p, r, 0), w2, h, color);
  canvas_rectf(canvas, point_move_xy(p, 0, r), r, h2, color);
  canvas_rectf(canvas, point_move_xy(p, w - r, r), r, h2, color);

  canvas_arcf(canvas, point_move_xy(p, r, r), r, M_PI_2, M_PI, color);
  canvas_arcf(canvas, point_move_xy(p, w - r, r), r, M_PI_2, M_PI_2 * 3, color);
  canvas_arcf(canvas, point_move_xy(p, w - r, h - r), r, M_PI_2, 0, color);
  canvas_arcf(canvas, point_move_xy(p, r, h - r), r, M_PI_2, M_PI_2, color);
}
