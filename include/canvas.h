#ifndef CANVAS_H
#define CANVAS_H

#include "point.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#define col(r, g, b) ((Color){r, g, b})
typedef struct pixel_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

typedef struct canvas_t Canvas;

Canvas *canvas_init(size_t width, size_t height);
void canvas_destroy(Canvas *canvas);
bool canvas_write(Canvas *canvas, char *filename);

void canvas_pixel(Canvas *canvas, Point pixel, Color color);
void canvas_get_pixel(Canvas *canvas, Point pixel, Color color);

void canvas_trig(Canvas *canvas, Point p0, Point p1, Point p2, Color color);
void canvas_trigf(Canvas *canvas, Point p0, Point p1, Point p2, Color color);

void canvas_line(Canvas *canvas, Point p0, Point p1, Color color);

void canvas_rect(Canvas *canvas, Point p, int w, int h, Color color);
void canvas_rectf(Canvas *canvas, Point p, int w, int h, Color color);

#define canvas_rectc(canvas, p, w, h, c)                                       \
  (canvas_rect(canvas, point_move_xy((x), -(w) / 2, -(h) / 2), (w), (h), (c)))
#define canvas_rectfc(canvas, p, w, h, c)                                      \
  (canvas_rectf(canvas, point_move_xy((x), -(w) / 2, -(h) / 2), (w), (h), (c)))

#define arc_segs(r) (M_PI * (r))
#define canvas_arc(canvas, center, radius, angle, rotate, color)               \
  canvas_arc_segs(canvas, center, radius, angle, rotate, arc_segs(radius),     \
                  color)
void canvas_arcf(Canvas *canvas, Point center, float radius, float angle,
                 float rotate, Color color);
void canvas_arc_segs(Canvas *canvas, Point center, float radius, float angle,
                     float rotate, unsigned int segs, Color color);

void canvas_rrect(Canvas *canvas, Point p, int w, int h, float r, Color color);
void canvas_rrectf(Canvas *canvas, Point p, int w, int h, float r, Color color);

#endif
