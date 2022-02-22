#include "../headers/gOGAFunctions.h"

evaluation evaluateIndividual(vector<group> individual, vector<vector<vector<int>>> items, int pickingLocations, int horizontalTransition)
{
    evaluation e;
    e.fitness = 0;
    int numAisles = items[0].size();

    int distance = 0;
    for (int b = 0; b < individual.size(); b++)
    {
        vector<vector<vector<int>>> solution;
        deque<vector<int>> opt;
        vector<vector<int>> locations;
        locations.resize(numAisles);

        for (int o : individual[b].orders)
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
        for (int i = 0; i < locations.size(); i++)
        {
            sort(locations[i].begin(), locations[i].end());
        }
        solution = traceRoute(pickingLocations, horizontalTransition, numAisles, locations);
        opt = optTour(solution);
        e.distances.push_back(opt.back().front());
        e.fitness += opt.back().front();
    }
    return (e);
}

// Adicionar cada grupo individualmente ao invés de usar resize (na linha 54)
vector<group> generateIndividual(vector<int> mu, int capacityLimit, vector<vector<vector<int>>> items, int pickingLocations, int horizontalTransition, mt19937 &gen)
{
    group g;
    vector<group> individual;
    // vector<vector<int>> individual;
    // individual.resize(mu.size());
    individual.push_back(g);
    vector<int> indexes;
    for (int i = 0; i < mu.size(); i++)
    {
        indexes.push_back(i);
    }
    int batch = 0;
    while (indexes.size() > 0) // Equanto há pedidos pendentes (não atribuidos a lotes)
    {
        int randomOrder = gen() % indexes.size();                 // Escolhe um pedido aleatoriamente
        individual[batch].orders.push_back(indexes[randomOrder]); // Adiciona o pedido ao lote atual
        indexes.erase(indexes.begin() + randomOrder);             // Remove o pedido da lista de pendentes
        int weight = batchWeight(mu, individual[batch].orders);   // Calcula o peso atual do lote
        int fits = 1;
        while (fits && weight < capacityLimit)
        {
            vector<int> currentOrders(individual[batch].orders);
            int index = 0;
            int selected = -1;
            int minDistance = INT_MAX;
            for (int o : indexes)
            {
                if (weight + mu[o] <= capacityLimit)
                {
                    int distance = ordersSimilarityDegree(currentOrders, o, items, pickingLocations, horizontalTransition);
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        selected = index;
                    }
                }
                index++;
            }
            if (selected != -1)
            {
                individual[batch].orders.push_back(indexes[selected]);
                weight += mu[indexes[selected]];
                indexes.erase(indexes.begin() + selected);
            }
            else
            {
                fits = 0;
            }
        }
        // Aqui adiciona novo lote
        if (indexes.size() > 0)
        {
            individual.push_back(g);
        }
        batch++;
    }
    return (individual);
}

vector<individual> selectParents(vector<individual> population, const int PARENTS_SIZE, mt19937 &gen)
{
    vector<individual> parents;
    vector<int> weights;
    int weight = population.size();
    for (int i = 0; i < population.size(); i++)
    {
        weights.push_back(weight);
        weight--;
    }
    // for (int i = 0; i < weights.size(); i++)
    // {
    //     totalWeight += weights[i];
    // }

    while (parents.size() < PARENTS_SIZE)
    {
        discrete_distribution<int> dist(begin(weights), end(weights));

        int i = dist(gen);
        //  cout << "Choosed parent " << i << endl;
        parents.push_back(population[i]);
        weights[i] = 0;
    }

    return parents;
}

vector<individual> crossover(vector<individual> parents, int numOrders, vector<int> mu, int capacityLimit, mt19937 &gen)
{
    //  cout << "Start crossover " << endl;
    vector<individual> offspring;

    while (parents.size() > 0)
    {
        int maxLength;
        vector<int> weights;
        vector<int> freeList(numOrders, 1);
        individual tape, child;
        int randIdx = gen() % parents.size(); // Ou usar função shuffle
        individual p1 = parents[randIdx];
        parents.erase(parents.begin() + randIdx);
        randIdx = gen() % parents.size();
        individual p2 = parents[randIdx];
        parents.erase(parents.begin() + randIdx);
        sort(p1.cromo.begin(), p1.cromo.end(), [](group &g1, group &g2)
             { return g1.orders.size() > g2.orders.size(); });
        sort(p2.cromo.begin(), p2.cromo.end(), [](group &g1, group &g2)
             { return g1.orders.size() > g2.orders.size(); });
        if (p1.cromo.size() >= p2.cromo.size())
        {
            maxLength = p2.cromo.size();
            // diff
        }
        else
        {
            maxLength = p1.cromo.size();
            // diff
        }
        // Compara batch p1, batch p2
        for (int i = 0; i < maxLength; i++)
        {
            if (p1.cromo[i].distance <= p2.cromo[i].distance)
            {
                tape.cromo.push_back(p1.cromo[i]);
                tape.cromo.push_back(p2.cromo[i]);
            }
            else
            {
                tape.cromo.push_back(p2.cromo[i]);
                tape.cromo.push_back(p1.cromo[i]);
            }
        }
        if (p1.cromo.size() > p2.cromo.size())
        {
            for (int i = maxLength; i < p1.cromo.size(); i++)
            {
                tape.cromo.push_back(p1.cromo[i]);
            }
        }
        if (p2.cromo.size() > p1.cromo.size())
        {
            for (int i = maxLength; i < p2.cromo.size(); i++)
            {
                tape.cromo.push_back(p2.cromo[i]);
            }
        }

        // filter child
        for (group g : tape.cromo) // Para cara grupo no cromossomo
        {
            int numOrdersInG = g.orders.size(); // Determina quantos pedidos estão no grupo
            vector<int> notRepeated;
            for (int o : g.orders) // Para cada pedido incluso no grupo
            {
                if (freeList[o]) // Se o pedido não está repetido
                {
                    notRepeated.push_back(o);
                    numOrdersInG--; // Decrementa
                }
            }
            if (numOrdersInG == 0) // Se não há pedidos repetidos
            {
                for (int o : g.orders) // Para cada pedido incluso no grupo
                {
                    freeList[o] = 0; // Aponta como ocupado
                }
                child.cromo.push_back(g); // Adiciona o grupo na lista de cromossomos do filho
            }
            else
            { // Caso haja pedidos repetidos
                // adicionar não repetidos na freelist
                for (int o : notRepeated)
                {
                    freeList[o] = 1; // Adiciona à freeList
                }
            }
        }
        // Update weights
        for (group g : child.cromo)
        {
            int w = 0;
            for (int o : g.orders)
            {
                w += mu[o];
            }
            weights.push_back(w);
        }
        // First-fit
        for (int o = 0; o < freeList.size(); o++)
        {
            if (freeList[o])
            {
                int g = 0;
                while (g < weights.size())
                {
                    if (weights[g] + mu[o] <= capacityLimit)
                    {
                        freeList[o] = 0;
                        weights[g] += mu[o];
                        child.cromo[g].orders.push_back(o);
                        break;
                    }
                    g++;
                }
                if (g == weights.size())
                {
                    freeList[o] = 0;
                    group newG;
                    newG.orders.push_back(o);
                    child.cromo.push_back(newG);
                    weights.push_back(mu[o]);
                }
            }
        }
        offspring.push_back(child);
    }
    //   cout << endl;
    return offspring;
}

// Retornar também booleano para informar se foi efetuada a mutação (se não for feita mutação, não precisa reavaliar fitness)
mutate mutation(individual indi, vector<int> mu, int capacityLimit, mt19937 &gen)
{
    mutate m;
    int w1 = 0;
    int w2 = 0;
    int g1 = gen() % indi.cromo.size();
    int g2;
    do
    {
        g2 = gen() % indi.cromo.size();
    } while (g1 == g2);
    int o1 = gen() % indi.cromo[g1].orders.size();
    int o2 = gen() % indi.cromo[g2].orders.size();
    int p1 = indi.cromo[g1].orders[o1];
    int p2 = indi.cromo[g2].orders[o2];
    for (int o : indi.cromo[g1].orders)
    {
        w1 += mu[o];
    }
    for (int o : indi.cromo[g2].orders)
    {
        w2 += mu[o];
    }

    if ((w1 - mu[p1] + mu[p2] <= capacityLimit) && (w2 - mu[p2] + mu[p1] <= capacityLimit))
    {
        indi.cromo[g1].orders[o1] = p2;
        indi.cromo[g2].orders[o2] = p1;
        m.occurred = true;
        //  cout << "Mutation: order " << p1 << " changed with order " << p2 << " from batch " << g1 << " to batch " << g2 << "." << endl;
    }
    else
    {
        m.occurred = false;
        //  cout << "No mutation. " << endl;
    }
    m.mutated = indi;
    return m;
}