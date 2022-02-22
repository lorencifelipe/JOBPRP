#ifndef COMMON_FUNCTIONS_H
#define COMMON_FUNCTIONS_H

#include <stdlib.h>
#include "problemStructures.h"
#include "../headers/dPFunctions.h"

using namespace std;

warehouse addressing(int numAisles, int numPickingPositions);

void shuffle(vector<int> &mu, vector<int> &indexes, int seed);

int ordersSimilarityDegree(vector<int> orders, int order, vector<vector<vector<int>>> items, int pickingLocations, int horizontalTransition);

int batchWeight(vector<int> mu, vector<int> batch);

#endif