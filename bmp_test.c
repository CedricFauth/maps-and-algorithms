#include "bmp_writer.h"

void to_pixel(int val, struct pixel* p, void* tmp) {
  switch (val) {
    case -2:
      p->r = 0;
      p->g = 0;
      p->b = 0;
      break;
    case -1:
      p->r = 50;
      p->g = 50;
      p->b = 50;
      break;
    case 0:
      p->r = 0;
      p->g = 255;
      p->b = 0;
      break;
    default:
      p->r = val;
      p->g = 0;
      p->b = 255-val;
      break;
  }
}


int main(void) {

  int arr0[] = {-2,-1,0};
  int arr1[] = {1, 100, 200};
  int *arr[] = {arr0, arr1};

  bmp_from_array2d(arr, 2, 3, 10, to_pixel, NULL);
  return 0;
}
