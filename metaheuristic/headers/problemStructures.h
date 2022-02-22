/*STRUCTS*/

#ifndef PROBLEM_STRUCTURES_H
#define PROBLEM_STRUCTURES_H

#include <vector>
#include <deque>

using namespace std;

typedef struct Coord
{
    int aisle;
    int location;
} coord;

typedef struct Warehouse
{
    vector<int> addresses;
    vector<coord> coordinates;
} warehouse;

// Vertice - used at DP
typedef struct Vert
{
    int arcos;
    int aisle;
    int location;
} vertice;

// Order id is the index
// The set of orders is a vector<order>
// The weight of an order is the number of articles
typedef struct Order
{
    int weight;
    vector<coord> position;
} order;


// Batch id is the index
// The set of batches is a vector<batch>
typedef struct Batch
{
    int weight;
    vector<int> orders;            // Orders that are inside the batch
    vector<vector<int>> locations; // Locations for DP
    double distance;               // Total distance traveled at route
} batch;

#endif