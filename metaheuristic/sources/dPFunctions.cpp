#include "../headers/dPFunctions.h"

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

vector<vector<int>> returnLocations(vector<int> orders, vector<vector<vector<int>>> items)
{
    int numAisles = items[0].size();
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
            sort(locations[i].begin(),locations[i].end());
        }
    }
    return locations;
}