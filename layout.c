#include "include/layout.h"
#include "include/canvas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EL_PAGE_SIZE 8

#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct ui_el_t UIEL;
struct ui_el_t {
  UIEL *parent;
  UIEL *next;
  UIEL *child_first;
  UIEL *child_last;
  ElSpec spec;
  Point pos;
  Point pos_accum;
  struct {
    int w, h;
  } size;
};

typedef struct el_page_t ElPage;
struct el_page_t {
  UIEL el[EL_PAGE_SIZE];
  ElPage *next;
  int size;
};

struct layout_t {
  Canvas *canvas;
  ElPage *page;
  ElPage *page_tail;
  UIEL *el;
  size_t width;
  size_t height;
};

static ElPage *page_create() {
  ElPage *page = malloc(sizeof(ElPage));
  if (page == NULL)
    return NULL;
  page->size = 0;
  page->next = NULL;
  return page;
}

static UIEL *page_alloc(Layout *layout) {
  if (layout == NULL)
    return NULL;

  if (layout->page == NULL) {
    layout->page = page_create();
    if (layout->page == NULL)
      return NULL;
    layout->page_tail = layout->page;
  }

  if (layout->page_tail->size == EL_PAGE_SIZE)
    layout->page_tail->next = page_create();

  UIEL *el = &layout->page_tail->el[layout->page_tail->size++];
  memset(el, 0, sizeof(UIEL));
  return el;
}

static void page_destroy(Layout *layout) {
  if (layout == NULL)
    return;

  ElPage *page = layout->page;
  while (page != NULL) {
    ElPage *next = page->next;
    page->next = NULL;
    free(page);
    page = next;
  }
}

typedef struct {
  ElPage *page;
  UIEL *el;
} PageIter;
static UIEL *iter_next(PageIter *iter) {
  if (iter->page == NULL || iter->page->size == 0)
    return NULL;
  if (iter->el == NULL)
    iter->el = iter->page->el;

  UIEL *el = iter->el++;
  if (iter->el == iter->page->el + iter->page->size) {
    iter->page = iter->page->next;
    iter->el = NULL;
  }

  return el;
}

#define page_iter(layout)                                                      \
  ((PageIter){.page = ((Layout *)layout)->page, .el = NULL})
#define page_foreach(layout)                                                   \
  PageIter __iter##__LINE__ = page_iter(layout);                               \
  for (UIEL *el = iter_next(&__iter##__LINE__); el != NULL;                    \
       el = iter_next(&__iter##__LINE__))

Layout *lay_create(size_t width, size_t height, Color bg) {
  Layout *lay = malloc(sizeof(struct layout_t));
  if (lay == NULL)
    return NULL;

  lay->canvas = canvas_init(width, height);
  if (lay->canvas == NULL)
    goto canvas_fail;

  lay->width = width;
  lay->height = height;

  lay->page = NULL;
  lay->page_tail = NULL;
  lay->el = NULL;

  return lay;
stack_fail:
  canvas_destroy(lay->canvas);
canvas_fail:
  free(lay);
  return NULL;
}

void lay_destroy(Layout *layout) {
  if (layout == NULL)
    return;

  canvas_destroy(layout->canvas);
  page_destroy(layout);
  free(layout);
}

void lay_el_open(Layout *layout, ElSpec spec) {
  if (layout == NULL)
    return;

  UIEL *el = page_alloc(layout);
  if (el == NULL)
    return;

  el->spec = spec;
  el->parent = layout->el;

  if (spec.padding > 0) {
    el->pos_accum.x += spec.padding;
    el->pos_accum.y += spec.padding;
  }

  if (el->parent != NULL) {
    if (el->parent->child_last == NULL)
      el->parent->child_first = el;
    else
      el->parent->child_last->next = el;
    el->parent->child_last = el;
  }

  layout->el = el;
}

static inline void fail(const char *msg) {
  printf("%s\n", msg);
  exit(EXIT_FAILURE);
}

static inline uint8_t count_children(UIEL *el) {
  uint8_t count = 0;
  for (UIEL *child = el->child_first; child != NULL; child = child->next)
    count++;
  return count;
}

void lay_el_close(Layout *layout) {
  if (layout == NULL || layout->el == NULL)
    return;

  UIEL *el = layout->el;
  UIEL *parent = el->parent;

  if (el->spec.size.width.type == ELSZ_FIXED)
    el->size.w = el->spec.size.width.value + el->spec.padding * 2;
  else if (el->spec.size.width.type == ELSZ_FIT)
    el->size.w += el->spec.padding * 2;
  else
    fail("invalid width type");

  if (el->spec.size.height.type == ELSZ_FIXED)
    el->size.h = el->spec.size.height.value + el->spec.padding * 2;
  else if (el->spec.size.height.type == ELSZ_FIT)
    el->size.h += el->spec.padding * 2;
  else
    fail("invalid height type");

  uint8_t child_count = count_children(el);
  uint8_t gap = child_count > 1 ? el->spec.gap * (child_count - 1) : 0;

  if (el->spec.layout == LAY_ROW) {
    el->size.w += gap;
  } else if (el->spec.layout == LAY_COL) {
    el->size.h += gap;
  } else
    fail("invalid layout");

  if (parent != NULL) {
    uint8_t padding = el->spec.padding * 2;

    if (parent->spec.layout == LAY_COL) {
      parent->size.h += el->size.h;
      parent->size.w = max(layout->el->size.w, parent->size.w);

      el->pos = parent->pos_accum;
      parent->pos_accum.y += el->size.h + parent->spec.gap;
    } else if (parent->spec.layout == LAY_ROW) {
      parent->size.w += el->size.w;
      parent->size.h = max(layout->el->size.h, parent->size.h);

      el->pos = parent->pos_accum;
      parent->pos_accum.x += el->size.w + parent->spec.gap;
    } else {
      fail("Unsupported layout");
    }
  }

  layout->el = layout->el->parent;
}

Canvas *lay_render(Layout *layout) {
  if (layout == NULL)
    return NULL;

  page_foreach(layout) {
    Point parent_pos = el->parent == NULL ? (Point){0, 0} : el->parent->pos;
    Point pos = point_move(parent_pos, el->pos);

    switch (el->spec.brush) {
    case BR_STROKE:
      if (el->spec.rounded > 0)
        canvas_rrect(layout->canvas, pos, el->size.w, el->size.h,
                     el->spec.rounded, el->spec.color);
      else
        canvas_rect(layout->canvas, pos, el->size.w, el->size.h,
                    el->spec.color);
      break;
    case BR_FILL:
      if (el->spec.rounded > 0)
        canvas_rrectf(layout->canvas, pos, el->size.w, el->size.h,
                      el->spec.rounded, el->spec.color);
      else
        canvas_rectf(layout->canvas, pos, el->size.w, el->size.h,
                     el->spec.color);
      break;
    }
  }

  return layout->canvas;
}
