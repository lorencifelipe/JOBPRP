# The joint order batching and picking routing problem: algorithms and new formulation

## Abstract

Efficiently managing large deposits and warehouses is not an easy task. The amount of variables and processes involved from the moment a consumer purchases a single product until its receipt is quite considerable. There are two major problems involving warehouses processes: the order picking problem (OPP) and the order batching problem (OBP). The OPP aims to minimize the distance traveled by a picker while collecting a set of products (orders). The OBP seeks to assign orders to batches with a capacity limit to minimize the sum of distances traveled during the retrieving of products from all batches. When these two problems are approached together, they become the Joint Order Batching and Picking Routing Problem (JOBPRP). 

This work proposes a novel formulation for JOBPRP and develops a dynamic programming based heuristic, and a grouping genetic algorithm with controlled gene transmission to JOBPRP. To assess our proposals, we executed computational experiments over literature datasets. The mathematical model was used within a mixed-integer programming solver (Gurobi) and tested on the smaller instances to evaluate the quality of the solutions of our metaheuristic approach. Our computational results evidence high stability for all tested instances and much lower objective value than the previously reported in the literature while maintaining a reasonable computational time.

## Guide of directories/files

- /heuristic: implementation of the heuristic solution
  - /heuristic_results: output of heuristic experiments, organized by warehouses
  - /reduced_instances: input files, organized by warehouses
  - heuristic.cpp: main heuristic file
  - run_heuristic_test.py: experiments controller
- /metaheuristic: implementation of the metaheuristic solution
  - /headers: all .h files
  - /reduced_instances: input files, organized by warehouses
  - /results: output of metaheuristic experiments, organized by warehouses
  - /sources: .cpp files, jobprp_ga_v1.cpp is the main
  - run_ga_experiment.py: experiments controller
- /solver: integration with Gurobi
  - /ga_results: 
