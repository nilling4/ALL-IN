// grid.hpp
#pragma once
#include <unordered_set>
// hash_specializations.hpp
#ifndef HASH_SPECIALIZATIONS_HPP
#define HASH_SPECIALIZATIONS_HPP
#include "vector"
#include <utility>
#include <functional>

#define NORTH 0
#define EAST 2
#define SOUTH 1
#define WEST 3
#endif // HASH_SPECIALIZATIONS_HPP
const int GRID_WIDTH = 160;
const int GRID_HEIGHT = 80;
struct sEdge{
    float sx, sy, ex, ey;
};
struct sCell{
    int edgeID[4];
    bool edge_exist[4];
    bool exist = false;
};  
extern std::vector<sEdge> edges; 
extern sCell* cells;

// Declare the grid
extern int grid[GRID_HEIGHT][GRID_WIDTH];
extern int flowField[GRID_HEIGHT][GRID_WIDTH];
extern std::vector<std::tuple<float,float,float>> triangleCorners;
void resetCorners();
void CalculateVisibleTriangles(float radius);