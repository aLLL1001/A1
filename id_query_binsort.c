#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "id_query.h"

struct index_record {
    int64_t osm_id;
    const struct record *record;
};

struct binary_data {
    struct index_record *irs;
    int n;
};

int comparator(const void *a, const void *b){
    int64_t a_id = ((struct index_record *)a) -> osm_id;
    int64_t b_id = ((struct index_record *)b) -> osm_id;
    if (a_id > b_id ){
        return 1;
    }
    else if (a_id == b_id){
        return 0;
    }
    else{
        return -1;
    }
}

struct binary_data* mk_binary(struct record* rs, int n){
    struct binary_data *data = malloc(sizeof(struct binary_data));
    struct index_record *data_record = malloc(n*sizeof(struct index_record));
    for (int i = 0; i < n; i++ ){
        data_record[i].osm_id = rs[i].osm_id;
        data_record[i].record = &rs[i];
    }
    qsort(data_record, n, sizeof(struct index_record), comparator);
    data -> irs = data_record;
    data -> n = n;
    return data; 
}

void free_binary(struct binary_data* data){
    free(data -> irs);
    free(data);
}

const struct record* lookup_binary(struct binary_data *data, int64_t needle){
    int low = 0;
    int high = (data->n) - 1;
    int mid;
    while (low <= high){
        mid = floor (low + (high-low)/2);
        if ((data->irs[mid]).osm_id < needle){
            low = mid + 1;  
        }
        else if ((data->irs[mid]).osm_id > needle){
            high = mid -1;
        }
        else {
            return data->irs[mid].record;
        }
    }
    return 0;
}



int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binary,
                    (free_index_fn)free_binary,
                    (lookup_fn)lookup_binary);
    
}