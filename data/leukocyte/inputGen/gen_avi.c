#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avilib.h"

#define WIDTH 640
#define HEIGHT 480
#define FPS 30.0
#define FRAMES 600
#define TOP 110
#define BOTTOM 328
#define CELL_COUNT 20
#define MIN_RADIUS 6.0f
#define MAX_RADIUS 12.0f

typedef struct {
  float x;
  float y;
  float vx;
  float vy;
  float r;
  uint8_t intensity;
} Cell;

static uint32_t rng_state = 1;

static void rng_seed(uint32_t seed) {
  rng_state = seed ? seed : 1;
}

static uint32_t rng_next(void) {
  rng_state = rng_state * 1664525u + 1013904223u;
  return rng_state;
}

static float randf(float min, float max) {
  float t = (float)rng_next() / (float)UINT32_MAX;
  return min + t * (max - min);
}

static int clamp_int(int value, int lo, int hi) {
  if (value < lo) return lo;
  if (value > hi) return hi;
  return value;
}

static void init_cells(Cell *cells, int count) {
  for (int i = 0; i < count; ++i) {
    cells[i].x = randf(40.0f, (float)WIDTH - 40.0f);
    cells[i].y = randf((float)TOP + 20.0f, (float)BOTTOM - 20.0f);
    cells[i].vx = randf(-1.3f, 1.3f);
    if (cells[i].vx == 0.0f) cells[i].vx = 0.7f;
    cells[i].vy = randf(-0.9f, 0.9f);
    if (cells[i].vy == 0.0f) cells[i].vy = -0.5f;
    cells[i].r = randf(MIN_RADIUS, MAX_RADIUS);
    cells[i].intensity = (uint8_t)(180 + (i * 7) % 60);
  }
}

static void update_cells(Cell *cells, int count) {
  for (int i = 0; i < count; ++i) {
    cells[i].x += cells[i].vx;
    cells[i].y += cells[i].vy;

    float min_x = cells[i].r + 1.0f;
    float max_x = (float)WIDTH - 2.0f - cells[i].r;
    if (cells[i].x < min_x || cells[i].x > max_x) {
      cells[i].vx = -cells[i].vx;
      cells[i].x = cells[i].x < min_x ? min_x : max_x;
    }

    float min_y = (float)TOP + cells[i].r + 1.0f;
    float max_y = (float)BOTTOM - 1.0f - cells[i].r;
    if (cells[i].y < min_y || cells[i].y > max_y) {
      cells[i].vy = -cells[i].vy;
      cells[i].y = cells[i].y < min_y ? min_y : max_y;
    }
  }
}

static void fill_background(uint8_t *frame, int frame_idx) {
  for (int y = 0; y < HEIGHT; ++y) {
    int row = y * WIDTH;
    for (int x = 0; x < WIDTH; ++x) {
      int base = 16 + ((x + y + frame_idx) & 0x0f);
      int noise = (int)((rng_next() >> 24) & 0x0f);
      int band = abs(x - ((frame_idx * 3) % WIDTH));
      int band_boost = band < 18 ? (18 - band) : 0;
      int value = clamp_int(base + noise + band_boost, 0, 255);
      frame[row + x] = (uint8_t)value;
    }
  }
}

static void draw_cells(uint8_t *frame, const Cell *cells, int count) {
  for (int i = 0; i < count; ++i) {
    int cx = (int)(cells[i].x + 0.5f);
    int cy = (int)(cells[i].y + 0.5f);
    int r = (int)(cells[i].r + 0.5f);
    int r2 = r * r;
    int x0 = cx - r;
    int x1 = cx + r;
    int y0 = cy - r;
    int y1 = cy + r;

    for (int y = y0; y <= y1; ++y) {
      if (y < 0 || y >= HEIGHT) continue;
      int dy = y - cy;
      int row = y * WIDTH;
      for (int x = x0; x <= x1; ++x) {
        if (x < 0 || x >= WIDTH) continue;
        int dx = x - cx;
        if (dx * dx + dy * dy <= r2) {
          int idx = row + x;
          if (frame[idx] < cells[i].intensity) {
            frame[idx] = cells[i].intensity;
          }
        }
      }
    }
  }
}

static void usage(const char *prog) {
  fprintf(stderr, "usage: %s <output.avi> [--seed N]\n", prog);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  const char *output = argv[1];
  uint32_t seed = 1;

  for (int i = 2; i < argc; ++i) {
    if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
      seed = (uint32_t)strtoul(argv[i + 1], NULL, 10);
      ++i;
    } else {
      usage(argv[0]);
      return 1;
    }
  }

  rng_seed(seed);

  avi_t *avi = AVI_open_output_file((char *)output);
  if (!avi) {
    AVI_print_error("Error with AVI_open_output_file");
    return 1;
  }

  AVI_set_video(avi, WIDTH, HEIGHT, FPS, "DIB ");

  uint8_t *frame = (uint8_t *)malloc(WIDTH * HEIGHT);
  if (!frame) {
    fprintf(stderr, "Out of memory for frame buffer.\n");
    AVI_close(avi);
    return 1;
  }

  Cell cells[CELL_COUNT];
  init_cells(cells, CELL_COUNT);

  for (int f = 0; f < FRAMES; ++f) {
    fill_background(frame, f);
    draw_cells(frame, cells, CELL_COUNT);
    if (AVI_write_frame(avi, (char *)frame, WIDTH * HEIGHT, 1) != 0) {
      AVI_print_error("Error with AVI_write_frame");
      free(frame);
      AVI_close(avi);
      return 1;
    }
    update_cells(cells, CELL_COUNT);
  }

  free(frame);
  AVI_close(avi);
  return 0;
}
