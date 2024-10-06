#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data *data; 
  data = malloc(sizeof( struct naive_data));
  data->n = n;
  data->rs = rs;
  return data;
}

void free_naive(struct naive_data* data) {
  free(data);
}

double calEuclidean(double x1, double x2, double y1, double y2){
  return sqrt(pow((x1 - x2),2) + pow((y1 - y2),2));
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  double closestDistance = calEuclidean(lon, data->rs[0].lon, lat, data->rs[0].lat);
  int index = 0;
  for (int i = 1; i < (data->n); i++){
    double currentDistance = calEuclidean(lon, data->rs[i].lon, lat, data->rs[i].lat);
    if (currentDistance < closestDistance){
      closestDistance = currentDistance;
      index = i;
    }
  }
  return &(data->rs)[index];
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
