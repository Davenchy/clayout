# layout — Pure C layout engine (inspired by Clay)

## Commands

| Command      | Action                             |
|--------------|------------------------------------|
| `make`       | Build `app` binary                 |
| `make run`   | Build + run                        |
| `make view`  | Build + run + open PPM with `feh`  |
| `make clean` | Remove binary, PPM, and `.o` files |

Build flags: `gcc -ggdb -lm` (debug symbols, math lib).  
Output image: `layout.ppm` (PPM P6 binary format). Viewer defaults to `feh`.  
No tests, no linter, no formatter.

## Architecture

- **Source files**: `main.c` (entrypoint), `layout.c` (layout engine), `canvas.c` (pixel canvas), `point.c` (geometry helpers). Headers in `include/`.
- **Flow**: `lay_create(w, h, bg)` → `El(lay, {spec}) { /* children */ }` DSL → `lay_render(layout)` → `canvas_write(canvas, "layout.ppm")`.
- `El()` macro: for-loop calling `lay_el_open` / `lay_el_close` — accepts a compound literal `ElSpec`.
- Element sizes: `Fixed(n)` or `Fit` (enum `ELSZ_FIXED` / `ELSZ_FIT`).
- Layout modes: `LAY_ROW` (horizontal), `LAY_COL` (vertical).
- Brush modes: `BR_FILL` (filled), `BR_STROKE` (outline).
- Canvas is a single malloc (header + pixel data). No dynamic resizing.
- Element storage: linked list of fixed-size pages (`EL_PAGE_SIZE = 8` els per page).

## Conventions

- All `#include` paths relative to project root (e.g. `#include "include/canvas.h"`).
- No dynamic memory for individual elements — pool-allocated in pages.
- No external dependencies beyond the C standard library and `libm` (`-lm`).
