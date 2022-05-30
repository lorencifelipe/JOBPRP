#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <climits>
#include <deque>
#include <chrono>

using namespace std;

/*CONSTANTS*/

const int MAX_ORDERS = 100;

const int MAX_ADRESSES = 500;

// First operations
const int START[7] = {1, 2, 3, 5, 4, 6};

// Operations to obtain each equivalence class
// To obtain Eq. Class [index], add config(0) to class(1) - VERTICAL
const int REVERSE_TABLE_1[8][10][2] = {
    {},                                                                               // discart
    {{2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}}, //{a,b} = add config a to b class, to obtain 1 = (U,U,1C)
    {{2, 2}, {6, 2}, {2, 6}},
    {{3, 3}, {6, 3}, {3, 6}},
    {{1, 1}, {5, 2}, {5, 3}, {2, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 4}, {5, 5}, {5, 6}},
    {{3, 2}, {4, 2}, {2, 3}, {4, 3}, {2, 5}, {3, 5}, {4, 5}, {6, 5}, {4, 6}},
    {{6, 6}},
    {{6, 7}},
};

// Operations to obtain each equivalence class
// To obtain Eq. Class [index], add config[0] to class[1] - HORIZONTAL
const int REVERSE_TABLE_2[8][4][2] = {
    {}, // discart
    {{1, 1}},
    {{2, 2}, {2, 4}},
    {{3, 3}, {3, 4}},
    {{4, 4}},
    {{4, 2}, {4, 3}, {4, 5}},
    {{5, 6}},
    {{5, 2}, {5, 3}, {5, 4}, {5, 7}},
};

/*STRUCTS*/

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
typedef struct VERT
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

typedef struct Solution
{
    double x[MAX_ORDERS][MAX_ORDERS];
    double z[MAX_ADRESSES][MAX_ADRESSES][MAX_ORDERS];
    double u[MAX_ADRESSES][MAX_ORDERS];
    double objVal{0};
} solution;

// Batch id is the index
// The set of batches is a vector<batch>
typedef struct Batch
{
    int weight;
    vector<int> orders;            // Orders that are inside the batch
    vector<vector<int>> locations; // Locations for DP
    double distance;               // Total distance traveled at route
} batch;

vector<vector<double>> delta(500, vector<double>(500, 0));
solution s;

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

// ShortestDistance function
// Returns a matrix with the shortest distance between a pair of locations
void shortestDistance(warehouse w, int numPickingPos, double pickingPosLength, double pickingPosWidth, double aislewidth, double distanceDispatch)
{
    double distanceDF; // Shortest distance from depot to front aisle
    for (int i = 0; i < w.addresses.size(); i++)
    {
        for (int j = 0; j < w.addresses.size(); j++)
        {
            if (delta[i][j] == 0)
            {
                if (i == j)
                {
                    continue;
                }
                else if (w.coordinates[i].aisle == w.coordinates[j].aisle)
                {
                    delta[i][j] = abs((w.coordinates[i].location - w.coordinates[j].location) * pickingPosLength);
                    delta[j][i] = delta[i][j];
                }
                else
                {
                    if (i == 0) // If dispatch area
                    {
                        distanceDF = sqrt(pow(distanceDispatch, 2) + pow(w.coordinates[j].aisle * 2 * pickingPosWidth, 2));
                        delta[i][j] = distanceDF + w.coordinates[j].location * pickingPosLength;
                        delta[j][i] = delta[i][j];
                    }
                    else
                    {
                        delta[i][j] = min(
                            (numPickingPos - w.coordinates[i].location + 1) * pickingPosLength + abs(2 * pickingPosWidth * (w.coordinates[j].aisle - w.coordinates[i].aisle)) + (numPickingPos - w.coordinates[j].location + 1) * pickingPosLength,
                            w.coordinates[i].location * pickingPosLength + abs(2 * pickingPosWidth * (w.coordinates[j].aisle - w.coordinates[i].aisle)) + w.coordinates[j].location * pickingPosLength); // HERE
                        delta[j][i] = delta[i][j];
                    }
                }
            }
        }
    }
}

// Knapsack function
// Returns the indexes (from mu) of orders that must be allocated to batches
vector<int> knapsack(vector<int> mu, int capacityLimit)
{
    vector<int> x;
    vector<vector<int>> m(mu.size() + 1, vector<int>(capacityLimit + 1, 0));
    // Table
    for (int i = 1; i < mu.size() + 1; i++)
    {
        for (int j = 1; j < capacityLimit + 1; j++)
        {
            if (mu[i - 1] > j)
            {
                m[i][j] = m[i - 1][j];
            }
            else
            {
                m[i][j] = max(m[i - 1][j], mu[i - 1] + m[i - 1][j - mu[i - 1]]);
            }
        }
    }
    // Backtrack solution
    int p = mu.size();
    int q = capacityLimit;
    while (p > 0 && q > 0)
    {
        if (m[p][q] == m[p - 1][q])
        {
            p--;
        }
        else
        {
            x.push_back(p - 1);
            p--;
            q -= mu[p];
        }
    }
    return x;
}

// generateInitialBatches function
// Return a vector with batches - an initial sol. for x[o][b]
vector<batch> generateInitialBatches(vector<int> mu, int capacityLimit)
{
    batch b;
    vector<int> indexes;
    vector<batch> batches;
    // Initialize all batches as empty
    b.distance = 0;
    b.orders = {};
    batches.resize(mu.size(), b);
    // Define indexes
    for (int i = 0; i < mu.size(); i++)
    {
        indexes.push_back(i);
    }
    // While are not allocated orders
    int i = 0;
    while (indexes.size() > 0)
    {
        vector<int> x = knapsack(mu, capacityLimit);
        for (int j : x)
        {
            batches[i].orders.push_back(indexes[j]);
        }
        for (int j : x)
        {
            mu.erase(mu.begin() + j);
            indexes.erase(indexes.begin() + j);
        }
        i++;
    }
    return batches;
}

// vertical_route_sum function
// Returns a vector with vertical length and in case IV, location of max GAP
vector<int> vertical_route_sum(int picking_locations, vector<int> items, int operation)
{
    if (items.size() == 0 && operation != 6)
    {
        return {-1, 0};
    }
    else if (operation == 1)
    {
        return {picking_locations, 0}; //+1?
    }
    else if (operation == 2)
    {
        return {2 * (picking_locations - items[0]), 0};
    }
    else if (operation == 3)
    {
        return {2 * (items.back()), 0};
    }
    else if (operation == 4 && items.size() >= 2)
    {
        int gap = 0;
        int pos = 0;
        for (int i = 1; i < items.size(); i++) // items.size() - 1
        {
            int temp = items[i] - items[i - 1];
            if (temp > gap)
            {
                gap = temp;
                pos = i;
            }
        }
        return {2 * (picking_locations - gap), pos}; // pos is the position of max gap
    }
    else if (operation == 5)
    {
        return {2 * picking_locations, 0};
    }
    else if (operation == 6)
    {
        return {0, 0};
    }
    else
    {
        return {-1, 0};
    }
}

// horizontal_route_sum function
// Returns an int, representing the length of k[0] transition (horizontal)
int horizontal_route_sum(int horizontal_transition, int operation)
{
    if (operation == 1)
    {
        return 2 * horizontal_transition;
    }
    else if (operation == 2)
    {
        return 2 * horizontal_transition;
    }
    else if (operation == 3)
    {
        return 2 * horizontal_transition;
    }
    else if (operation == 4)
    {
        return 4 * horizontal_transition;
    }
    else
    {
        return 0;
    }
}

// traceRoute function
// Used to trace graph
// Input: picking_locations => number of picking positions in each aisle
//        horizontal_transition => length (units of measure) between parallel aisles
//        num_aisles => total number of parallel aisles (vertical)
//        items => picking positision of items in each aisle
vector<vector<vector<int>>> traceRoute(int picking_locations, int horizontal_transition, int num_aisles, vector<vector<int>> items)
{
    // Control of itens in each aisle
    int total_items = 0;
    int picked = 0;
    for (int i = 0; i < items.size(); i++)
    {
        total_items += items[i].size();
    }

    // Generate solution table
    vector<int> base = {-1, -1, -1, -1}; //-> initialize a line as dash (-)
    vector<vector<int>> l;               // Layers (column)
    for (int i = 0; i <= 7; i++)
    {
        l.push_back(base); // Generate a column from lines
    }
    vector<vector<vector<int>>> solution;
    for (int i = 0; i <= 2 * num_aisles - 1; i++)
    {
        solution.push_back(l); // Generate a solution matrix from columns
    }

    // Start routing
    int min_length = INT_MAX;
    int i = 1;
    for (auto &k : START)
    {
        vector<int> route = vertical_route_sum(picking_locations, items[0], k); // AQUI O BUG
        int temp = route[0];
        if (temp > 0 || (items[0].size() == 1 && items[0][0] == 0)) // OU AQUI
        {
            min_length = temp;
            solution[1][i][0] = temp;
            solution[1][i][1] = -1;
            solution[1][i][2] = k;
            solution[1][i][3] = route[1]; // case 4
        }
        i++;
    }
    picked += items[0].size();

    // Fill the table
    for (int i = 2; i <= 2 * num_aisles - 1; i++)
    {
        if (i % 2) // If odd -> vertical transition
        {
            picked += items[(i - 1) / 2].size();
            for (int j = 1; j <= 7; j++)
            {
                if ((j == 6 && picked > 0) || (j == 7 && picked - total_items > 0)) // Obs (b), (c) from tb1.
                {
                    continue;
                }
                int min_length = INT_MAX;
                for (auto &k : REVERSE_TABLE_1[j])
                {
                    if (solution[i - 1][k[1]][0] > 0 && !(k[0] == 6 && items[(i - 1) / 2].size() > 0)) // Basic and obs(a) from tb1.

                    {
                        vector<int> route = vertical_route_sum(picking_locations, items[(i - 1) / 2], k[0]);
                        int temp = solution[i - 1][k[1]][0] + route[0];
                        if (temp <= min_length && route[0] >= 0) // Important: temp <= min_length
                        {
                            min_length = temp;
                            solution[i][j][0] = temp;
                            solution[i][j][1] = k[1];
                            solution[i][j][2] = k[0];
                            solution[i][j][3] = route[1];
                        }
                    }
                }
            }
        }
        else // If Even -> Horizontal transition
        {
            for (int j = 1; j <= 7; j++)
            {
                int min_length = INT_MAX;
                for (auto &k : REVERSE_TABLE_2[j])
                {
                    if (solution[i - 1][k[1]][0] > -1)
                    {
                        int route = horizontal_route_sum(horizontal_transition, k[0]);
                        int temp = solution[i - 1][k[1]][0] + route;
                        if (temp < min_length)
                        {
                            min_length = temp;
                            solution[i][j][0] = temp;
                            solution[i][j][1] = k[1];
                            solution[i][j][2] = k[0];
                            solution[i][j][3] = 0;
                        }
                    }
                }
            }
        }
    }
    return solution;
}

// optTour function
// Returns the shortest of the partial tour subgraphs (PTS's)
deque<vector<int>> optTour(vector<vector<vector<int>>> solution)
{
    deque<vector<int>> opt;
    int min_length = INT_MAX;
    int index, next;
    int n = solution.size() - 1;
    for (int j = 2; j <= 7; j++) // j = 2 to cut (U,U,1C)
    {
        if (j == 5 or j == 6)
        { // Cut (E,E,2C) and (0,0,0C)
            continue;
        }
        if (solution[n][j][0] < min_length && solution[n][j][0] > 0)
        {
            min_length = solution[n][j][0];
            index = j;
            next = solution[n][j][1];
        }
    }
    opt.push_front({solution[n][index][0], solution[n][index][1], solution[n][index][2], solution[n][index][3]});
    for (int i = n - 1; i >= 1; i--)
    {
        opt.push_front({solution[i][next][0], solution[i][next][1], solution[i][next][2], solution[i][next][3]});
        next = solution[i][next][1];
    }
    return opt;
}

// add_vertice function
// Function to put vertices at graph
void add_vertice(vector<vertice> (&graph)[50][50], vertice v, vertice u) // Last mod: (&graph)[50][50] in place of graph[50][50]
{
    graph[u.aisle][u.location].push_back(v);
    graph[v.aisle][v.location].push_back(u);
}

// traceTour function
// Construct optimal order picking tour subgraph
vector<coord> traceTour(vector<vector<int>> items, deque<vector<int>> opt, coord depot, int num_aisles, int picking_locations)
{
    vector<coord> order; // Ordered visit list
    vector<vertice> graph[50][50];

    // Iterate over opt
    for (int i = 0; i < opt.size(); i++)
    {
        vertice u, v;
        int stat = opt[i][2];
        if (i % 2)
        { // if odd number -> Horizontal
            v.aisle = ((i - 1) / 2) + 1;
            u.aisle = ((i - 1) / 2);
            if (stat == 1)
            {
                // Bottom
                v.arcos = 1;
                v.location = 0;
                u.arcos = 1;
                u.location = 0;
                add_vertice(graph, v, u);
                // Top
                v.location = picking_locations;
                u.location = picking_locations;
                add_vertice(graph, v, u);
            }
            else if (stat == 2)
            {
                v.arcos = 2;
                v.location = picking_locations;
                u.arcos = 2;
                u.location = picking_locations;
                add_vertice(graph, v, u);
            }
            else if (stat == 3)
            {
                v.arcos = 2;
                v.location = 0;
                u.arcos = 2;
                u.location = 0;
                add_vertice(graph, v, u);
            }
            else if (stat == 4)
            {
                // Bottom
                v.arcos = 2;
                v.location = 0;
                u.arcos = 2;
                u.location = 0;
                add_vertice(graph, v, u);
                // Top
                v.location = picking_locations;
                u.location = picking_locations;
                add_vertice(graph, v, u);
            }
        }
        else
        { // if even -> Vertical
            v.aisle = i / 2;
            u.aisle = i / 2;
            if (stat == 1)
            {
                if (items[i / 2].size() == 0)
                { // If no products
                    v.arcos = 1;
                    v.location = 0;
                    u.arcos = 1;
                    u.location = picking_locations;
                    add_vertice(graph, v, u);
                }
                else
                {
                    // Connect bottom corner to first product
                    v.arcos = 1;
                    v.location = 0;
                    u.arcos = 1;
                    u.location = items[i / 2].front();
                    add_vertice(graph, v, u);
                    // Connect last product to top corner
                    v.location = items[i / 2].back();
                    u.location = picking_locations;
                    add_vertice(graph, v, u);
                    // Connnect intermediate vertices
                    for (int j = 1; j < items[i / 2].size(); j++)
                    {
                        v.location = items[i / 2][j - 1];
                        u.location = items[i / 2][j];
                        add_vertice(graph, v, u);
                    }
                }
            }
            else if (stat == 2)
            {
                // Top
                v.arcos = 2;
                v.location = picking_locations;
                u.arcos = 2;
                u.location = items[i / 2].back();
                add_vertice(graph, v, u);
                // Tail => MUST BE WALKED OTHERWISE
                for (int j = 1; j < items[i / 2].size(); j++)
                {
                    v.location = items[i / 2][j - 1];
                    u.location = items[i / 2][j];
                    add_vertice(graph, v, u);
                }
            }
            else if (stat == 3)
            {
                // Bottom
                v.arcos = 2;
                v.location = 0;
                u.arcos = 2;
                u.location = items[i / 2].front();
                add_vertice(graph, v, u);
                // Tail
                for (int j = 1; j < items[i / 2].size(); j++)
                {
                    v.location = items[i / 2][j - 1];
                    u.location = items[i / 2][j];
                    add_vertice(graph, v, u);
                }
            }
            else if (stat == 4)
            {
                // Bottom
                v.arcos = 2;
                v.location = 0;
                u.arcos = 2;
                u.location = items[i / 2].front();
                add_vertice(graph, v, u);
                // Top
                v.location = items[i / 2].back();
                u.location = picking_locations;
                add_vertice(graph, v, u);
                // Middle
                for (int j = 1; j < items[i / 2].size(); j++)
                {
                    if (j == opt[i][3])
                    {
                        continue;
                    }
                    v.location = items[i / 2][j - 1];
                    u.location = items[i / 2][j];
                    add_vertice(graph, v, u);
                }
            }
            else if (stat == 5)
            {
                // Bottom
                v.arcos = 2;
                v.location = 0;
                u.arcos = 2;
                u.location = items[i / 2].front();
                add_vertice(graph, v, u);
                // Top
                v.location = items[i / 2].back();
                u.location = picking_locations;
                add_vertice(graph, v, u);
                // Middle
                for (int j = 1; j < items[i / 2].size(); j++)
                {
                    v.location = items[i / 2][j - 1];
                    u.location = items[i / 2][j];
                    add_vertice(graph, v, u);
                }
            }
        }
    }

    // Explore graph from depot
    coord current;
    current.location = depot.location;
    current.aisle = depot.aisle;
    while (true)
    {
        order.push_back(current);
        int arcs = -2;
        int index_next = -1;
        int aisle_next = -1;
        int location_next = -1;
        int index_current = -1;
        for (int i = 0; i < graph[current.aisle][current.location].size(); i++) // analyze neighboring vertices
        {
            if (graph[current.aisle][current.location][i].arcos > arcs)
            {
                index_next = i; // index to next vertice to be explored
                aisle_next = graph[current.aisle][current.location][i].aisle;
                location_next = graph[current.aisle][current.location][i].location;
                arcs = graph[current.aisle][current.location][i].arcos;
            }
        }
        // Find index from current vertice at next vertice's neighbors
        for (int i = 0; i < graph[aisle_next][location_next].size(); i++)
        {
            if (graph[aisle_next][location_next][i].aisle == current.aisle && graph[aisle_next][location_next][i].location == current.location)
            {
                index_current = i;
                break;
            }
        }
        if (arcs == 2)
        {
            // Update current arcc
            graph[current.aisle][current.location][index_next].arcos = -1;
            // Update arcs from next
            graph[aisle_next][location_next][index_current].arcos = -1;
            // Update current aisle/location
            current.aisle = aisle_next;
            current.location = location_next;
        }
        else if (arcs == 1 || arcs == -1)
        {
            graph[current.aisle][current.location][index_next].arcos = -2;
            graph[aisle_next][location_next][index_current].arcos = -2;
            current.aisle = aisle_next;
            current.location = location_next;
        }
        else
        {
            break;
        }
    }
    return order;
}

int main(int argc, char *argv[])
{
    warehouse w;
    string lytFileName, ordFileName;
    int numAisles, numPickingPos, item, aisle, pickingPos, n, capacityLimit, totalA, numOrder, numArticles;
    double pickingPosLength, pickingPosWidth, aisleWidth, distanceDispatch;
    vector<vector<vector<int>>> items; // Endereço completo para cada order, usado na PD
    vector<int> mu;
    vector<vector<int>> phi;
    vector<batch> batches;
    vector<vector<vector<int>>> solution;
    deque<vector<int>> opt;
    vector<int> initialAdresses;
    vector<vector<int>> sample;
    vector<coord> optOrder;
    coord depot;
    depot.aisle = 0;
    depot.location = 0;
    // Check args
    if (argc != 3)
    {
        cout << "Error: you need to specify exactly 7 instance files. Sorry." << endl;
        exit(0);
    }
    else
    {
        // cout << "Reading..." << endl;
        lytFileName = argv[1];
        ordFileName = argv[2];

        // Open layout file
        ifstream layoutInstance(lytFileName);
        if (layoutInstance.is_open())
        {
            layoutInstance >> numAisles >> numPickingPos >> pickingPosLength >> pickingPosWidth >> aisleWidth >> distanceDispatch >> capacityLimit >> n;
            layoutInstance.close();
        }
        distanceDispatch = 0;
        // Assign addresses
        // w.addresses is the vector with all addresses (e)
        w = addressing(numAisles, numPickingPos);
        shortestDistance(w, numPickingPos, pickingPosLength, pickingPosWidth, aisleWidth, distanceDispatch);
        // Number of addresses
        totalA = w.addresses.size();

        // Open orders instance
        ifstream ordersInstance(ordFileName);
        if (ordersInstance.is_open())
        {
            vector<vector<int>> orderItems;
            phi.resize(totalA, vector<int>(n, 0));
            ordersInstance >> numOrder >> numArticles;
            while (numArticles != 0)
            {
                orderItems.resize(numAisles);
                orderItems[0].push_back(0); // Depot
                mu.push_back(numArticles);
                for (int i = 0; i < numArticles; i++)
                {
                    ordersInstance >> item >> aisle >> pickingPos;
                    orderItems[aisle / 2].push_back(pickingPos + 1); // Vector to DP
                    int addr = aisle / 2 * numPickingPos + pickingPos + 1;
                    phi[addr][numOrder] = 1;
                }
                items.push_back(orderItems); // For each order, a bidimensional vector [aisle][picking position] of items
                orderItems.clear();
                ordersInstance >> numOrder >> numArticles;
            }
            ordersInstance.close();
        }

        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

        // Generate initial batch configuration
        batches = generateInitialBatches(mu, capacityLimit);
        // Para cada lote, inclui as localizações para uso de DP
        for (int b = 0; b < batches.size(); b++)
        {
            batches[b].locations.resize(numAisles);
            for (int o : batches[b].orders)
            {
                for (int aisle = 0; aisle < items[o].size(); aisle++)
                {
                    for (int loc = 0; loc < items[o][aisle].size(); loc++)
                    {
                        int locIn = 0;
                        for (int x : batches[b].locations[aisle])
                        {
                            if (x == items[o][aisle][loc])
                            {
                                locIn = 1;
                                break;
                            }
                        }
                        if (!locIn)
                        {
                            batches[b].locations[aisle].push_back(items[o][aisle][loc]);
                        }
                    }
                }
            }
            for (int i = 0; i < batches[b].locations.size(); i++)
            {
                sort(batches[b].locations[i].begin(), batches[b].locations[i].end());
            }
        }
        // Para cada lote, calcular a rota mínima e a distância mínima usando PD
        for (int b = 0; b < batches.size(); b++)
        {
            if (batches[b].orders.size() > 0)
            {
                solution = traceRoute(numPickingPos + 1, 5, numAisles, batches[b].locations); // +1? //BUG AQUI
                opt = optTour(solution); // OU BUG AQUI
                batches[b].distance = opt.back().front();
                optOrder = traceTour(batches[b].locations, opt, depot, numAisles, numPickingPos + 1); //+1?
                for (int i = 2; i < optOrder.size(); i++)
                {
                    if (!((optOrder[i].location == 0 && optOrder[i].aisle != 0) || optOrder[i].location == numPickingPos + 1)) // +1?
                    {
                        int toAdd = optOrder[i].aisle * numPickingPos + optOrder[i].location;
                        // Converte para endereços cuidando se não está no vetor
                        if (find(initialAdresses.begin(), initialAdresses.end(), toAdd) == initialAdresses.end())
                        {
                            initialAdresses.push_back(toAdd);
                        }
                        // else if (optOrder[i].location == 0) // Add return to depot
                        // {
                        //     initialAdresses.push_back(toAdd);
                        // }
                        // cout << "Aisle: " << optOrder[i].aisle << " Location: " << optOrder[i].location << endl;
                    }
                }
                // Update z vars from solution
                for (int i = 0; i < initialAdresses.size() - 1; i++)
                {
                    s.z[initialAdresses[i]][initialAdresses[i + 1]][b] = 1;
                }
                s.z[initialAdresses.back()][0][b] = 1;
                initialAdresses.clear();
                //  cout << "Everything ok with batch " << b << "." << endl;
            }
        }

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        // chrono::duration<double> timeSpan = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
        chrono::duration<double, milli> timeSpan = t2 - t1; // Out in ms

        // Update x vars from solution
        for (int b = 0; b < batches.size(); b++)
        {
            for (int o : batches[b].orders)
            {
                s.x[o][b] = 1;
            }
        }
        // Update optimal from solution
        for (int b = 0; b < batches.size(); b++)
        {
            s.objVal += batches[b].distance;
        }

        cout << s.objVal << endl;
        cout << timeSpan.count() << endl; // ms
        cout << capacityLimit << endl;
        cout << n << endl;
    }
}