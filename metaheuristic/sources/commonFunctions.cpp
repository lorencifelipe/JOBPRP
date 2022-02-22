#include "../headers/commonFunctions.h"

// Addresing function
// Returns the addresses set and a picking location/aisle for each address
warehouse addressing(int numAisles, int numPickingPositions)
{
    warehouse w;
    vector<int> addresses;
    vector<coord> coordinates;
    w.addresses.push_back(0); // Dispatch area
    coord coordinate;
    coordinate.aisle = 0;
    coordinate.location = 0;
    w.coordinates.push_back(coordinate); // Dispatch area
    int address = 1;                     // First address
    for (int i = 0; i < numAisles; i++)
    {
        for (int j = 1; j <= numPickingPositions; j++)
        {
            coordinate.aisle = i;
            coordinate.location = j;
            w.addresses.push_back(address);
            w.coordinates.push_back(coordinate);
            address++;
        }
    }
    return w;
}

// Shuffles mu and its respective indexes vectors
void shuffle(vector<int> &mu, vector<int> &indexes, int seed)
{
    srand(seed);
    int temp = 0;
    int newIndex = 0;
    for (int i = 0; i < mu.size(); i++)
    {
        newIndex = rand() % mu.size();
        temp = mu[i];
        mu[i] = mu[newIndex];
        mu[newIndex] = temp;
        temp = indexes[i];
        indexes[i] = indexes[newIndex];
        indexes[newIndex] = temp;
    }
}

int ordersSimilarityDegree(vector<int> orders, int order, vector<vector<vector<int>>> items, int pickingLocations, int horizontalTransition)
{
    orders.push_back(order);
    int numAisles = items[0].size();
    vector<vector<vector<int>>> solution;
    deque<vector<int>> opt;
    vector<vector<int>> locations;
    locations.resize(numAisles);

    for (int o : orders)
    {
        for (int i = 0; i < numAisles; i++)
        {
            for (int j : items[o][i])
            {
                int jIn = 0;
                for (int k : locations[i])
                {
                    if (k == j)
                    {
                        jIn = 1;
                        break;
                    }
                }
                if (!jIn)
                {
                    locations[i].push_back(j);
                }
            }
        }
    }

    solution = traceRoute(pickingLocations, horizontalTransition, numAisles, locations);
    opt = optTour(solution);
    return (opt.back().front());
}

int batchWeight(vector<int> mu, vector<int> batch)
{
    int weight = 0;
    for (int o : batch)
    {
        weight += mu[o];
    }
    return weight;
}
