/*CONSTANTS*/

#ifndef DP_CONSTANTS_H
#define DP_CONSTANTS_H

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

#endif