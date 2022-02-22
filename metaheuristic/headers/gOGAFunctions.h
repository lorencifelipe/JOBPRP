#ifndef GOGA_FUNCTIONS_H
#define GOGA_FUNCTIONS_H

#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include "../headers/commonFunctions.h"
#include "../headers/gOGAStructures.h"
//#include "../headers/gOGAConstants.h"

using namespace std;

vector<group> generateIndividual(vector<int> mu, int capacityLimit, vector<vector<vector<int>>> items, int pickingLocations, int horizontalTransition, mt19937 &gen);

evaluation evaluateIndividual(vector<group> individual, vector<vector<vector<int>>> items, int pickingLocations, int horizontalTransition);

vector<individual> selectParents(vector<individual> population,  const int PARENTS_SIZE, mt19937 &gen);

vector<individual> crossover(vector<individual> parents, int numOrders, vector<int> mu, int capacityLimit, mt19937 &gen);

mutate mutation(individual indi, vector<int> mu, int capacityLimit, mt19937 &gen);

#endif