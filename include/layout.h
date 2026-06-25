#ifndef LAYOUT_H
#define LAYOUT_H

#include "canvas.h"
#include "point.h"

typedef struct layout_t Layout;

typedef enum draw_cmd_type_t {
  CMD_LINE = 0,
  CMD_TRIG,
  CMD_ARC,
  CMD_CIRCLE,
  CMD_CIRCLE_SEGS,
  CMD_RECT,
  CMD_RRECT,
  DRAW_CMD_TYPE_LEN,
} DrawCMDType;

typedef struct draw_cmd_t {
  DrawCMDType type;
  Color color;
  union {
    struct {
      Point a;
      Point b;
    } line;
    struct {
      Point a;
      Point b;
      Point c;
    } trig;
    struct {
      Point center;
      float radius;
    } circle;
    struct {
      Point center;
      float radius;
      unsigned int segs;
    } circle_segs;
    struct {
      Point p;
      int w;
      int h;
    } rect;
    struct {
      Point p;
      int w;
      int h;
      float r;
    } rrect;
    struct {
      Point center;
      float radius;
      float angle;
      float rotate;
    } arc;
  } params;
} DrawCMD;

enum el_size_t {
  ELSZ_FIT = 0,
  ELSZ_FIXED,
  ELSZ_LEN,
};
typedef struct el_spec_size_t {
  enum el_size_t type;
  size_t value;
} ElSpecSize;
#define Fit ((ElSpecSize){.type = ELSZ_FIT})
#define Fixed(_val) ((ElSpecSize){.type = ELSZ_FIXED, .value = _val})

typedef struct el_spec_t {
  enum {
    LAY_COL,
    LAY_ROW,
    LAY_SIZE,
  } layout;
  enum { BR_FILL, BR_STROKE } brush;
  struct {
    ElSpecSize width;
    ElSpecSize height;
  } size;
  float rounded;
  uint8_t gap;
  uint8_t padding;
  Color color;
} ElSpec;

Layout *lay_create(size_t width, size_t height, Color bg);
void lay_destroy(Layout *layout);
DrawCMD *lay_draw_cmds(Layout *layout, size_t *size);
Canvas *lay_render(Layout *layout);

void lay_el_open(Layout *layout, ElSpec spec);
void lay_el_close(Layout *layout);

#define El(layout, ...)                                                        \
  for (int __once = (lay_el_open((layout), ((ElSpec)__VA_ARGS__)), 1); __once; \
       (lay_el_close(layout), __once--))

#endif
