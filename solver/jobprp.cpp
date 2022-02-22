#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <climits>
#include <deque>
#include "../gurobi903/linux64/include/gurobi_c++.h"

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

typedef struct SolverSolution
{
    int cols;
    double runTime;
    double obj;
    double vars[12530000];
    string varsNames[12530000];
} solverSolution;

GRBVar x[100][100];
GRBVar z[500][500][100];
GRBVar u[500][100];
vector<vector<double>> delta(500, vector<double>(500, 0));
solverSolution solverS;

vector<int> prepareAdresses(vector<vector<int>> phi)
{
    vector<int> e;
   // e.push_back(0);
    for (int i = 0; i < phi.size(); i++)
    {
        for (int j = 0; j < phi[i].size(); j++)
        {
            if (phi[i][j])
            {
                e.push_back(i);
                break;
            }
        }
    }
    return e;
}

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

void callSolver(int n, int capacityLimit, vector<int> e, vector<int> mu, vector<vector<int>> phi, string name)
{
    // cout << "Wait :: Calling Gurobi..." << endl;
    const double one = 1.0;
    try
    {
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "jobprp.log");
        // env.set("LogToConsole", "0");
        env.set("TimeLimit", "3600");
        //env.set("IterationLimit", "100000");
        env.start();
        // Create an empty model
        GRBModel jobprp = GRBModel(env);

        // Add x vars
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j <= i; j++)
            {
                string name = "x_[" + to_string(i) + "][" + to_string(j) + "]";
                x[i][j] = jobprp.addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
            }
        }

        // Add z vars
        for (int i : e)
        {
            for (int j : e)
            {
                if (i != j)
                {
                    for (int k = 0; k < n; k++)
                    {
                        string name = "z_[" + to_string(i) + "][" + to_string(j) + "][" + to_string(k) + "]";
                        z[i][j][k] = jobprp.addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                    }
                }
            }
        }
        // Add u vars
        for (int i : e)
        {
            for (int j = 0; j < n; j++)
            {
                string name = "u_[" + to_string(i) + "][" + to_string(j) + "]";
                u[i][j] = jobprp.addVar(1.0, e.size(), 0.0, GRB_INTEGER, name);
            }
        }
        // Linear expression to obj. func.
        GRBLinExpr obj;
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                for (int j : e)
                {
                    if (i != j)
                    {
                        obj += delta[i][j] * z[i][j][b];
                    }
                }
            }
        }
        jobprp.setObjective(obj, GRB_MINIMIZE);
        obj.clear();
        // Linear expression to constraints
        GRBLinExpr sum1, sum2;
        // Add constraints
        // R1
        for (int o = 0; o < n; o++)
        {
            for (int b = 0; b <= o; b++)
            {
                sum1 += x[o][b];
            }
            jobprp.addConstr(sum1 == 1);
            sum1.clear();
        }
        // R2
        for (int b = 0; b < n; b++)
        {
            for (int o = b; o < n; o++)
            {
                sum1 += mu[o] * x[o][b];
            }
            jobprp.addConstr(sum1 <= capacityLimit);
            sum1.clear();
        }
        // R3
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                for (int j : e)
                {
                    if (j != i)
                    {
                        sum1 += z[j][i][b];
                    }
                }
                for (int o = b; o < n; o++)
                {
                    sum2 += phi[i][o] * x[o][b];
                }
                jobprp.addConstr(sum1 <= sum2);
                sum1.clear();
                sum2.clear();
            }
        }
        // R4
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                for (int j : e)
                {
                    if (j != i)
                    {
                        sum1 += z[i][j][b];
                    }
                }
                for (int o = b; o < n; o++)
                {
                    sum2 += phi[i][o] * x[o][b];
                }
                jobprp.addConstr(sum1 <= sum2);
                sum1.clear();
                sum2.clear();
            }
        }

        // R4 =< 1
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                for (int j : e)
                {
                    if (j != i)
                    {
                        sum1 += z[j][i][b];
                    }
                }
                jobprp.addConstr(sum1 <= 1);
                sum1.clear();
            }
        }

        // R4 =< 1
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                for (int j : e)
                {
                    if (j != i)
                    {
                        sum1 += z[i][j][b];
                    }
                }
                jobprp.addConstr(sum1 <= 1);
                sum1.clear();
            }
        }

        // Nova 1
        for (int b = 0; b < n; b++)
        {
            for (int o = b; o < n; o++)
            {
                for (int i : e)
                {
                    for (int j : e)
                    {
                        if (j != i)
                        {
                            sum1 += z[j][i][b];
                        }
                    }
                    jobprp.addConstr(sum1 >= phi[i][o] * x[o][b]);
                    sum1.clear();
                }
            }
        }

        // Nova 2
        for (int b = 0; b < n; b++)
        {
            for (int o = b; o < n; o++)
            {
                for (int i : e)
                {
                    for (int j : e)
                    {
                        if (j != i)
                        {
                            sum1 += z[i][j][b];
                        }
                    }
                    jobprp.addConstr(sum1 >= phi[i][o] * x[o][b]);
                    sum1.clear();
                }
            }
        }

        // R5
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                for (int j : e)
                {
                    if (j != i && j != 0)
                    {
                        jobprp.addConstr(u[j][b] >= u[i][b] + 1 - e.size() * (1 - z[i][j][b]));
                    }
                }
            }
        }

       // R9 -> Best time without
        for (int b = 0; b < n; b++)
        {
            for (int i : e)
            {
                if (i != 0)
                {
                    sum1 += z[0][i][b] * i;
                }
            }
            for (int j : e)
            {
                if (j != 0)
                {
                    sum2 += z[j][0][b] * j;
                }
            }
            jobprp.addConstr(sum1 <= sum2);
            sum1.clear();
            sum2.clear();
        }

        // //    // R10 - TEST (Empty batches)
        // for (int b = 0; b < n - 1; b++)
        // {
        //     for (int o = b; o < n; o++)
        //     {
        //         sum1 += x[o][b];
        //     }
        //     for (int o = b + 1; o < n; o++)
        //     {
        //         sum2 += x[o][b + 1];
        //     }
        //     jobprp.addConstr(sum1 >= sum2);
        //     sum1.clear();
        //     sum2.clear();
        // }

        // R10 new test - EMPTY BATCHES
        for (int b = 0; b < n - 1; b++)
        {
            for (int o = b; o < n; o++)
            {
                sum1 += x[o][b];
            }
            for (int o = b + 1; o < n; o++)
            {
                sum2 += x[o][b + 1];
            }
            jobprp.addConstr(sum1 * (n - 1) >= sum2);
            sum1.clear();
            sum2.clear();
        }

        // Optimize model
        jobprp.optimize();
       // GRBVar *vars = jobprp.getVars();
        //solverS.cols = jobprp.get(GRB_IntAttr_NumVars);
        //solverS.runTime = jobprp.get(GRB_DoubleAttr_Runtime);
        //solverS.obj = jobprp.get(GRB_DoubleAttr_ObjVal);
        // cout << "Solver result: " << solverS.obj << endl;
        ofstream problemVars;
        problemVars.open(name+".out");
        problemVars << name << endl;
        problemVars << n << endl;
        problemVars << e.size() << endl;
        problemVars << capacityLimit << endl;
       // problemVars << jobprp.get(GRB_DoubleAttr_LB);
       // problemVars << jobprp.get(GRB_DoubleAttr_UB);
        problemVars << jobprp.get(GRB_IntAttr_NumConstrs) << endl;
        problemVars << jobprp.get(GRB_IntAttr_NumVars) << endl;
        problemVars << jobprp.get(GRB_DoubleAttr_ObjVal) << endl;
        problemVars << jobprp.get(GRB_DoubleAttr_ObjBound) << endl; //Best bound
        problemVars << jobprp.get(GRB_DoubleAttr_MIPGap) << endl;
        problemVars << jobprp.get(GRB_DoubleAttr_Runtime) << endl;
        
        problemVars.close();

        // for (int i = 0; i < solverS.cols; i++)
        // {
        //     //     problemVars << vars[i].get(GRB_StringAttr_VarName) << ": " << vars[i].get(GRB_DoubleAttr_X) << endl;
        //     solverS.vars[i] = vars[i].get(GRB_DoubleAttr_X);
        //     solverS.varsNames[i] = vars[i].get(GRB_StringAttr_VarName);
        // }
        // problemVars.close();
        cout << endl;
    }
    catch (GRBException error)
    {
        cout << "Error code = " << error.getErrorCode() << endl;
        cout << error.getMessage() << endl;
    }
    catch (...)
    {
        cout << "We have a (big) little problem here... Sorry." << endl;
    }
}

int main(int argc, char *argv[])
{
    // Statements
    warehouse w;
    string lytFileName, ordFileName;
    int numAisles, numPickingPos, item, aisle, pickingPos, n, capacityLimit, totalA, numOrder, numArticles;
    double pickingPosLength, pickingPosWidth, aisleWidth, distanceDispatch;
    vector<vector<vector<int>>> items; // Endereço completo para cada order, usado na PD
    vector<int> mu;
    vector<vector<int>> phi;
    vector<vector<vector<int>>> solution;
    deque<vector<int>> opt;
    vector<int> initialAdresses;
    vector<vector<int>> sample;
    vector<coord> optOrder;
    coord depot;
    depot.aisle = 0;
    depot.location = 0;
    int k = 1; // Tamanho da sample

    // Check args
    if (argc != 3)
    {
        cout << "Error: you need to specify exactly 2 instance files. Sorry." << endl;
        exit(0);
    }
    else
    {
        cout << "Reading..." << endl;
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
        
        //CHANGE PICKINGPOSWIDTH!!!!!!!1 

        // Assign addresses
        // w.addresses is the vector with all addresses (e)
        w = addressing(numAisles, numPickingPos);
        // Defines shortest distance between each pair of addresses
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
                phi[0][numOrder] = 1; //Gambis
                items.push_back(orderItems); // For each order, a bidimensional vector [aisle][picking position] of items
                orderItems.clear();
                ordersInstance >> numOrder >> numArticles;
            }
            ordersInstance.close();
        }
        vector<int> addresses = prepareAdresses(phi);
        callSolver(n,capacityLimit,addresses,mu,phi,ordFileName);
    }
}
