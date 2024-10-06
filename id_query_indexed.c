#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
    int64_t osm_id;
    const struct record *record;
};

struct indexed_data {
    struct index_record *irs;
    int n;
};

struct indexed_data* mk_indexed(struct record* rs, int n) {
    //Initial indexed data, and malloc some memory
    struct indexed_data *data;
    data = malloc(sizeof(struct indexed_data));

    //Intial indexed record, and malloc the memory with N pieces of records
    struct index_record *data_record;
    data_record = malloc(n*sizeof(struct index_record));

    for (int i = 0; i < n; i++){
        data_record[i].osm_id = rs[i].osm_id;
        data_record[i].record = &rs[i];
    }
    
    data -> irs = data_record;
    data -> n = n;
    return data;
}


void free_indexed(struct indexed_data* data){
    free(data -> irs);     
    free(data);   
}

const struct record* lookup_indexed(struct indexed_data *data, int64_t needle){
    for (int i = 0; i < (data -> n); i++) {
        if((data -> irs)[i].osm_id == needle) {
            return (data -> irs)[i].record;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
    
}