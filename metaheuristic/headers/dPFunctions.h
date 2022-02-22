/* DYNAMIC PROGRAMMING FUNCTIONS */

#ifndef DP_FUNCTIONS_H
#define DP_FUNCTIONS_H

#include <climits>
#include <algorithm>
#include "problemStructures.h"
#include "../headers/dPConstants.h"
#include "../headers/problemStructures.h"

using namespace std;

vector<int> vertical_route_sum(int picking_locations, vector<int> items, int operation);

int horizontal_route_sum(int horizontal_transition, int operation);

vector<vector<vector<int>>> traceRoute(int picking_locations, int horizontal_transition, int num_aisles, vector<vector<int>> items);

deque<vector<int>> optTour(vector<vector<vector<int>>> solution);

void add_vertice(vector<vertice> (&graph)[50][50], vertice v, vertice u);

vector<coord> traceTour(vector<vector<int>> items, deque<vector<int>> opt, coord depot, int num_aisles, int picking_locations);

vector<vector<int>> returnLocations(vector<int> orders, vector<vector<vector<int>>> items);

#endif