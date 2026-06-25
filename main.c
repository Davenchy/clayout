#include "include/canvas.h"
#include "include/layout.h"
#include <stdlib.h>

int main() {
  Layout *lay = lay_create(500, 500, (Color){0, 0, 0});
  if (lay == NULL)
    exit(EXIT_FAILURE);

  El(lay, {.padding = 20, .gap = 10, .color = {255, 0, 0}, .layout = LAY_ROW}) {
    El(lay, {
                .size = {.width = Fixed(150), .height = Fixed(100)},
                .color = {0, 255, 0},
                .rounded = 50,
            });

    El(lay, { .gap = 10 }) {
      El(lay, {
                  .size = {.width = Fixed(100), .height = Fixed(100)},
                  .color = {0, 255, 255},
              });
      El(lay, {
                  .size = {.width = Fixed(100), .height = Fixed(100)},
                  .color = {255, 255, 0},
              });
    }
  }

  Canvas *canvas = lay_render(lay);
  if (canvas == NULL)
    goto render_fail;

  canvas_write(canvas, "layout.ppm");
  lay_destroy(lay);

  return EXIT_SUCCESS;
render_fail:
  lay_destroy(lay);
  return EXIT_FAILURE;
}
