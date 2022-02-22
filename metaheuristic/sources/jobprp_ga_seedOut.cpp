#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <limits>
#include <chrono>
#include "../headers/problemStructures.h"
#include "../headers/dPConstants.h"
#include "../headers/dPFunctions.h"
#include "../headers/commonFunctions.h"
#include "../headers/gOGAFunctions.h"
#include "../headers/gOGAStructures.h"

using namespace std;

// Main function -> receives .lyt, .ord, SEED, POP_SIZE, OFFSPRING_SIZE, MUTATION_PROB, ITERATION_LIMIT
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
    if (argc != 8)
    {
        cout << "Error: you need to specify exactly 7 instance files. Sorry." << endl;
        exit(0);
    }
    else
    {
        // cout << "Reading..." << endl;
        lytFileName = argv[1];
        ordFileName = argv[2];
        const int SEED = stoi(argv[3]);
        const int POP_SIZE = stoi(argv[4]);
        const int OFFSPRING_SIZE = stoi(argv[5]); // POP_SIZE / 4;
        const double MUTATION_PROB = stod(argv[6]);
        const int ITERATION_LIMIT = stoi(argv[7]);
        const int PARENTS_SIZE = OFFSPRING_SIZE * 2;

        mt19937 gen(SEED);

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

        /*METAHEURISTIC*/
        chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
        cout << "[";
        int t = 0; // generation
        // Start initial population and evaluate fitness
        vector<individual> population;
        // individual best;
        // best.fitness = INT_MAX;
        for (int i = 0; i < POP_SIZE; i++)
        {
            individual indi;
            //  cout << "Generating individual " << i << "." << endl;
            indi.cromo = generateIndividual(mu, capacityLimit, items, numPickingPos, 5, gen);
            // cout << "Calculating fitness of individual " << i << "." << endl;
            evaluation indiEvaluation = evaluateIndividual(indi.cromo, items, numPickingPos, 5);
            indi.fitness = indiEvaluation.fitness;
            for (int j = 0; j < indiEvaluation.distances.size(); j++)
            {
                indi.cromo[j].distance = indiEvaluation.distances[j];
            }
            population.push_back(indi);
        }

        // for (int i = 0; i < population.size(); i++)
        // {
        //     if (population[i].fitness < best.fitness)
        //     {
        //         best = population[i];
        //     }
        // }
        // cout << "Sorting... " << endl;
        sort(population.begin(), population.end(), [](individual &x, individual &y)
             { return x.fitness < y.fitness; });

        individual best = population[0];
        vector<int> bests;
        vector<double> times;

        //   cout << "*** STARTING *** " << endl;
        // cout << "Generation 0 >> Best distance: " << best.fitness << endl;

        while (t < ITERATION_LIMIT) // Or best is stable for x iterations
        {
            vector<individual> parents = selectParents(population, PARENTS_SIZE, gen);    // Select Parents
            vector<individual> offspring = crossover(parents, n, mu, capacityLimit, gen); // Do crossover
            for (int i = 0; i < offspring.size(); i++)
            {
                evaluation offspringEvaluation = evaluateIndividual(offspring[i].cromo, items, numPickingPos, 5);
                offspring[i].fitness = offspringEvaluation.fitness;
                for (int j = 0; j < offspringEvaluation.distances.size(); j++)
                {
                    offspring[i].cromo[j].distance = offspringEvaluation.distances[j];
                }
            }

            // for (int i = 0; i < offspring.size(); i++)
            // {
            //     if (offspring[i].fitness < best.fitness)
            //     {
            //         best = offspring[i];
            //     }
            // }

            sort(offspring.begin(), offspring.end(), [](individual &x, individual &y)
                 { return x.fitness < y.fitness; });

            if (offspring[0].fitness < best.fitness)
            {
                best = offspring[0];
            }

            // Prob mutation
            vector<double> weights{1 - MUTATION_PROB, MUTATION_PROB};
            discrete_distribution<int> mut(begin(weights), end(weights));
            mt19937 mutProb;
            mutProb.seed(SEED);
            for (int i = 1; i < offspring.size(); i++)
            {
                if (mut(mutProb))
                {
                    mutate m = mutation(offspring[i], mu, capacityLimit, gen);
                    if (m.occurred)
                    {
                        offspring[i].cromo = m.mutated.cromo;
                        evaluation e = evaluateIndividual(offspring[i].cromo, items, numPickingPos, 5);
                        offspring[i].fitness = e.fitness;
                        for (int j = 0; j < e.distances.size(); j++)
                        {
                            offspring[i].cromo[j].distance = e.distances[j];
                        }
                        if (offspring[i].fitness < best.fitness)
                        {
                            best = offspring[i];
                        }
                    }
                }
            }
            population.insert(population.end(), offspring.begin(), offspring.end());
            sort(population.begin(), population.end(), [](individual &x, individual &y)
                 { return x.fitness < y.fitness; });
            population.resize(POP_SIZE);
            best = population[0];
            t++; // Next gen
            bests.push_back(best.fitness);
            chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
            chrono::duration<double> timeSpan = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
            times.push_back(timeSpan.count());
            // cout << "Generation " << t << " >> Best distance: " << best.fitness;
        }
        cout << "[";
        for(int i : bests){
            cout << i << ",";
        }
        cout << "]";
        cout << endl;
        cout << "[";        
        for(double d : times){
            cout << d << ",";
        }
        cout << "]";
    }
}
