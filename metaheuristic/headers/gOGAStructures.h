#ifndef GOGA_STRUCTURES_H
#define GOGA_STRUCTURES_H

#include <vector>

using namespace std;

typedef struct Group
{
    vector<int> orders;
    int distance;
} group;

typedef struct Individual
{
    vector<group> cromo;
    int fitness;
} individual;

typedef struct Evaluation
{
    vector<int> distances;
    int fitness;
} evaluation;

typedef struct Mutate
{
    individual mutated;
    bool occurred;
} mutate;

#endif