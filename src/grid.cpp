// grid.cpp
#include "grid.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>
// Define the grid
int grid[GRID_HEIGHT][GRID_WIDTH] = {{0}}; // Initialize all cells to 0 (unoccupied)
int flowField[GRID_HEIGHT][GRID_WIDTH] = {{0}}; // Initialize all cells to 0 (unoccupied)
std::vector<sEdge> edges = {}; 
sCell* cells = new sCell[GRID_HEIGHT * GRID_WIDTH];
std::vector<std::tuple<float,float,float>> triangleCorners = {};
void resetCorners(){
    edges = {};
    for (int i = 0;i<GRID_HEIGHT;i++){
        for (int j = 0;j<GRID_WIDTH;j++){
            for (int k = 0;k<4;k++){
                cells[i*GRID_WIDTH + j].edge_exist[k] = false;
                cells[i*GRID_WIDTH + j].edgeID[k] = 0;
            }
        }
    }
    for (int x = 0; x < GRID_WIDTH ; x++)
        for (int y = 0; y < GRID_HEIGHT ; y++){
            // Create some convenient indices
            int i = (y ) * GRID_WIDTH + (x );			// This
            int n = (y  - 1) * GRID_WIDTH + (x );		// Northern Neighbour
            int s = (y  + 1) * GRID_WIDTH + (x );		// Southern Neighbour
            int w = (y ) * GRID_WIDTH + (x  - 1);	// Western Neighbour
            int e = (y ) * GRID_WIDTH + (x  + 1);	// Eastern Neighbour

            // If this cell exists, check if it needs edges
            if (cells[i].exist)
            {
                // If this cell has no western neighbour, it needs a western edge
                if (!cells[w].exist)
                {
                    // It can either extend it from its northern neighbour if they have
                    // one, or It can start a new one.
                    if (cells[n].edge_exist[WEST])
                    {
                        // Northern neighbour has a western edge, so grow it downwards
                        edges[cells[n].edgeID[WEST]].ey += 12;
                        cells[i].edgeID[WEST] = cells[n].edgeID[WEST];
                        cells[i].edge_exist[WEST] = true;
                    }
                    else
                    {
                        // Northern neighbour does not have one, so create one
                        sEdge edge;
                        edge.sx = (x) * 12.0f; edge.sy = (y) * 12.0f;
                        edge.ex = edge.sx; edge.ey = edge.sy + 12.0f;

                        // Add edge to Polygon Pool
                        int edgeID = edges.size();
                        edges.push_back(edge);

                        // Update tile information with edge information
                        cells[i].edgeID[WEST] = edgeID;
                        cells[i].edge_exist[WEST] = true;
                    }
                }

                // If this cell dont have an eastern neignbour, It needs a eastern edge
                if (!cells[e].exist)
                {
                    // It can either extend it from its northern neighbour if they have
                    // one, or It can start a new one.
                    if (cells[n].edge_exist[EAST])
                    {
                        // Northern neighbour has one, so grow it downwards
                        edges[cells[n].edgeID[EAST]].ey += 12.0f;
                        cells[i].edgeID[EAST] = cells[n].edgeID[EAST];
                        cells[i].edge_exist[EAST] = true;
                    }
                    else
                    {
                        // Northern neighbour does not have one, so create one
                        sEdge edge;
                        edge.sx = ( x + 1) * 12.0f; edge.sy = ( y) * 12.0f;
                        edge.ex = edge.sx; edge.ey = edge.sy + 12.0f;

                        // Add edge to Polygon Pool
                        int edgeID = edges.size();
                        edges.push_back(edge);

                        // Update tile information with edge information
                        cells[i].edgeID[EAST] = edgeID;
                        cells[i].edge_exist[EAST] = true;
                    }
                }

                // If this cell doesnt have a northern neignbour, It needs a northern edge
                if (!cells[n].exist)
                {
                    // It can either extend it from its western neighbour if they have
                    // one, or It can start a new one.
                    if (cells[w].edge_exist[NORTH])
                    {
                        // Western neighbour has one, so grow it eastwards
                        edges[cells[w].edgeID[NORTH]].ex += 12.0f;
                        cells[i].edgeID[NORTH] = cells[w].edgeID[NORTH];
                        cells[i].edge_exist[NORTH] = true;
                    }
                    else
                    {
                        // Western neighbour does not have one, so create one
                        sEdge edge;
                        edge.sx = (x) * 12.0f; edge.sy = (y) * 12.0f;
                        edge.ex = edge.sx + 12.0f; edge.ey = edge.sy;

                        // Add edge to Polygon Pool
                        int edgeID = edges.size();
                        edges.push_back(edge);

                        // Update tile information with edge information
                        cells[i].edgeID[NORTH] = edgeID;
                        cells[i].edge_exist[NORTH] = true;
                    }
                }

                // If this cell doesnt have a southern neignbour, It needs a southern edge
                if (!cells[s].exist)
                {
                    // It can either extend it from its western neighbour if they have
                    // one, or It can start a new one.
                    if (cells[w].edge_exist[SOUTH])
                    {
                        // Western neighbour has one, so grow it eastwards
                        edges[cells[w].edgeID[SOUTH]].ex += 12.0f;
                        cells[i].edgeID[SOUTH] = cells[w].edgeID[SOUTH];
                        cells[i].edge_exist[SOUTH] = true;
                    }
                    else
                    {
                        // Western neighbour does not have one, so I need to create one
                        sEdge edge;
                        edge.sx = (x) * 12.0f; edge.sy = (y + 1) * 12.0f;
                        edge.ex = edge.sx + 12.0f; edge.ey = edge.sy;

                        // Add edge to Polygon Pool
                        int edgeID = edges.size();
                        edges.push_back(edge);

                        // Update tile information with edge information
                        cells[i].edgeID[SOUTH] = edgeID;
                        cells[i].edge_exist[SOUTH] = true;
                    }
                }

            }

        }

}
void CalculateVisibleTriangles(float radius)
	{
		// Get rid of existing polygon

		triangleCorners = {};
        Entity player;
        for (Entity entity : registry.players.entities) {
            player = entity;
        }
		// For each edge in PolyMap
		for (auto &e1 : edges)
		{
			// Take the start point, then the end point (we could use a pool of
			// non-duplicated points here, it would be more optimal)
			for (int i = 0; i < 2; i++)
			{
				float rdx, rdy;
				rdx = (i == 0 ? e1.sx : e1.ex) - registry.motions.get(player).position.x;
				rdy = (i == 0 ? e1.sy : e1.ey) - registry.motions.get(player).position.y;

				float base_ang = atan2f(rdy, rdx);

				float ang = 0;
				// For each point, cast 3 rays, 1 directly at point
				// and 1 a little bit either side
				for (int j = 0; j < 3; j++)
				{
					if (j == 0)	ang = base_ang - 0.0001f;
					if (j == 1)	ang = base_ang;
					if (j == 2)	ang = base_ang + 0.0001f;

					// Create ray along angle for required distance
					rdx = radius * cosf(ang);
					rdy = radius * sinf(ang);

					float min_t1 = INFINITY;
					float min_px = 0, min_py = 0, min_ang = 0;
					bool bValid = false;

					// Check for ray intersection with all edges
					for (auto &e2 : edges)
					{
						// Create line segment vector
						float sdx = e2.ex - e2.sx;
						float sdy = e2.ey - e2.sy;

						if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
						{
							// t2 is normalised distance from line segment start to line segment end of intersect point
							float t2 = (rdx * (e2.sy - registry.motions.get(player).position.y) + (rdy * (registry.motions.get(player).position.x - e2.sx))) / (sdx * rdy - sdy * rdx);
							// t1 is normalised distance from source along ray to ray length of intersect point
							float t1 = (e2.sx + sdx * t2 - registry.motions.get(player).position.x) / rdx;

							// If intersect point exists along ray, and along line 
							// segment then intersect point is valid
							if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
							{
								// Check if this intersect point is closest to source. If
								// it is, then store this point and reject others
								if (t1 < min_t1)
								{
									min_t1 = t1;
									min_px = registry.motions.get(player).position.x + rdx * t1;
									min_py = registry.motions.get(player).position.y + rdy * t1;
									min_ang = atan2f(min_py - registry.motions.get(player).position.y, min_px - registry.motions.get(player).position.x);
									bValid = true;
								}
							}
						}
					}

					if(bValid)// Add intersection point to visibility polygon perimeter
						triangleCorners.push_back({ min_ang, min_px, min_py });
				}
			}
		}

		// Sort perimeter points by angle from source. This will allow
		// us to draw a triangle fan.
        sort(
            triangleCorners.begin(),
            triangleCorners.end(),
            [](const std::tuple<float, float, float> &t1, const std::tuple<float, float, float> &t2)
            {
                return std::get<0>(t1) < std::get<0>(t2);
            });

	}
