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

//Creating a data structure that can save longitude and latitude 
struct coord_record {
    double lon;
    double lat;
    const struct record *record;
};

//Data structure for each node
struct kdTree_node {
    struct coord_record* coordrecord; 
    struct kdTree_node* left;
    struct kdTree_node* right;
    int axis;
};

//Data structure for all the nodes and the size of all the records 
struct kdTree_data{
    struct kdTree_node* nodes;
    int n;
};


//Comparing which will be used in qsort to compare the longitudes 
int comparator_lon(const void* coordA, const void* coordB) {
    struct coord_record* a = (struct coord_record*)coordA;
    struct coord_record* b = (struct coord_record*)coordB;  
    if ((a->lon) < (b->lon)){
        return -1;
    } 
    else if ((a->lon) > (b->lon)) {
        return 1;
    }
    return 0;
}

//Comparing which will be used in qsort to compare the latitudes
int comparator_lat(const void* coordA, const void* coordB) {
    struct coord_record* a = (struct coord_record*)coordA;
    struct coord_record* b = (struct coord_record*)coordB;  
    if ((a->lat) < (b->lat)) {
        return -1;
    }
    else if ((a->lat) > (b->lat)) {
        return 1;
    }
    return 0;
}


struct kdTree_node* buildKdTreeRec(struct coord_record* points, int n, int depth) {
    if (n == 0){
        return NULL;
    }    
    //Computing the remainder, which can denotes either x axis or y axis
    int axis = depth % 2;
    //axis == 0 is lon, which is x axis; 
    //axis == 1 is lat, which is y axis
    if (axis == 0){
        qsort(points, n, sizeof(struct coord_record), comparator_lon);
    }
    else{
        qsort(points, n, sizeof(struct coord_record), comparator_lat);
    }
    // Find the median point based on axis
    int median = n / 2;
    struct kdTree_node* node = malloc(sizeof(struct kdTree_node));
    node->axis = axis;
    node->coordrecord = &points[median];
    node->left = buildKdTreeRec(&points[0], median, depth + 1);
    node->right = buildKdTreeRec(&points[median + 1], n - median - 1, depth + 1);
    return node;
}
    
struct kdTree_data* mk_kdTree(struct record* rs, int n) {
    struct kdTree_data *kdData = malloc(sizeof( struct kdTree_data));
    struct coord_record *coord = malloc(n*sizeof(struct coord_record));;     
    // To create the coord_record structure, and put every record 
    // into this data structure.
    for (int i =0 ; i < n; i++){
        coord[i].lat = rs[i].lat;
        coord[i].lon = rs[i].lon;
        coord[i].record = &rs[i];
    }

    kdData -> n = n;

    kdData -> nodes = buildKdTreeRec(coord, n, 0);

    return kdData;
}


// Function to free memory for the kd-Tree nodes
void freeKdTree(struct kdTree_node* node) {
    freeKdTree(node->left);
    freeKdTree(node->right);
    free(node);
}


void free_kdTree(struct kdTree_data* kdData) {
    freeKdTree(kdData->nodes);
    free(kdData);
}


//Calculate teh euclidean distance between two points
double calEuclidean(double x1, double x2, double y1, double y2){
  return sqrt(pow((x1 - x2),2) + pow((y1 - y2),2));
}

// Algorithm from the assignment description:
// if node is NULL then
// return
// else if node.point is closer to query than closest then
// replace closest with node.point;
// diff ← node.point[node.axis] - query [node.axis];
// radius ← the distance between query and closest
// if diff ≥ 0 ∨ radius > |diff| then
//  lookup (closest, query, node.left)
// if diff ≤ 0 ∨ radius > |diff| then
//  lookup (closest, query, node.right)

struct coord_record* kdTreeSearchRec
                    (struct coord_record* closest,
                     const double* query,
                     struct kdTree_node* node
                     ) {
    if (node == NULL) {
        return closest;
    }
    double distClosest = calEuclidean(closest->lon, query[0], closest->lat, query[1]); 
    double distanceNode = calEuclidean(node->coordrecord->lon, query[0], node->coordrecord->lat, query[1]);
    if (distanceNode < distClosest) {
        closest = node->coordrecord;
    }

    double diff;

    if (node->axis == 0){
        diff = (node->coordrecord->lon ) - (query[node->axis]);
    }
    else{
        diff = (node->coordrecord->lat ) - (query[node->axis]);
    }

    double radius = calEuclidean(query[0], closest->lon, query[1], closest->lat);
    // if diff ≥ 0 ∨ radius > |diff| then
    // look into the left subtree
    if ((diff >= 0) || (radius >= fabs(diff))){
        closest = kdTreeSearchRec(closest, query, node->left);
    }

    // if diff <= 0 ∨ radius > |diff| then
    // look into the right subtree
    if ((diff <= 0) || (radius >= fabs(diff))){
        closest = kdTreeSearchRec(closest, query, node->right);
    }

    return closest;
}


const struct record* lookup_kdTree(struct kdTree_data *data, double lon, double lat) {
    double query[2] = {lon, lat};
    return  kdTreeSearchRec(data->nodes->coordrecord, query, data->nodes)->record;
}


int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdTree,
                          (free_index_fn)free_kdTree,
                          (lookup_fn)lookup_kdTree);
}