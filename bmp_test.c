#include "bmp_writer.h"

void to_pixel(void* v, struct pixel* p, void* tmp) {
  char c = *(char*)v;
  p->r = c;
  p->g = 0;
  p->b = 0;
}


int main(void) {

  char arr0[] = {0, 20, 100};
  char arr1[] = {20, 50, 120};
  char *arr[] = {arr0, arr1};

  bmp_from_array2d(arr, sizeof(**arr), 2, 3, 1, to_pixel, NULL);
  return 0;
}
