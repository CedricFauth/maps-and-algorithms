#ifndef BMP_WRITER_H
#define BMP_WRITER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct bmp_header {
  uint8_t magic[2];
  uint32_t filesize;
  uint16_t res0;
  uint16_t res1;
  uint32_t bmp_offset;
}__attribute__((packed));

struct dib_header {
  uint32_t infoheader_size;
  int32_t width;
  int32_t height;
  uint16_t n_planes;
  uint16_t n_bpp;
  uint32_t compress_type;
  uint32_t data_size;
  uint32_t h_res;
  uint32_t v_res;
  uint32_t n_colors;
  uint32_t n_impcolors;
}__attribute__((packed));

struct pixel {
  uint8_t b, g, r;
}__attribute__((packed));

void bmp_from_array2d(void* array2d, int v_size, int height, int width, int scale, 
                        void (*to_pixel)(void* v, struct pixel* p, void* tmp), void* tmp) {
  uint16_t n_bpp = 24;
  uint32_t row_size = (n_bpp*width*scale+31)/32 * 4;
  uint32_t data_size = height*scale * row_size*scale;
  //printf("rowsize %d\n", row_size);

  struct bmp_header bmp_head = {
    .magic = {0x42, 0x4d},
    .filesize = 54 + data_size,
    .res0 = 0,
    .res1 = 0,
    .bmp_offset = 54
  };

  struct dib_header dib_head = {
    .infoheader_size = 40,
    .width = width*scale,
    .height = -height*scale,
    .n_planes = 1,
    .n_bpp = n_bpp,
    .compress_type = 0,
    .data_size = data_size,
    .h_res = 2835,
    .v_res = 2835,
    .n_colors = 0,
    .n_impcolors = 0,
  };

  FILE* f = fopen("out.bmp", "w");

  fwrite(&bmp_head, 1, 14, f);
  fwrite(&dib_head, 1, 40, f);

  uint8_t** bytes_array2d = (uint8_t**) array2d;
  uint8_t* row = malloc(sizeof(*row)*row_size);
  struct pixel* pixel_row = (struct pixel*)row;

  for (int i=0; i<height; ++i) {
    memset(row, 0, row_size);
    for (int j=0; j<width; ++j) {
        for (int s=0; s<scale; ++s)
          to_pixel(*(bytes_array2d + i) + j*v_size, pixel_row + j*scale + s, tmp);
    }
    for (int s=0; s<scale; ++s)
      fwrite(row, sizeof(*row), row_size, f);
  }
  free(row);

  fclose(f);

}

#endif//BMP_WRITER_H
