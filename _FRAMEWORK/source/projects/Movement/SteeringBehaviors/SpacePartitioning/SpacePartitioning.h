/*=============================================================================*/
// Copyright 2019-2020
// Authors: Yosha Vandaele
/*=============================================================================*/
// SpacePartitioning.h: Contains Cell and Cellspace which are used to partition a space in segments.
// Cells contain pointers to all the agents within.
// These are used to avoid unnecessary distance comparisons to agents that are far away.

// Heavily based on chapter 3 of "Programming Game AI by Example" - Mat Buckland
/*=============================================================================*/

#pragma once
#include <list>
#include <vector>
#include <iterator>
#include "framework\EliteMath\EVector2.h"
#include "framework\EliteGeometry\EGeometry2DTypes.h"

class SteeringAgent;

struct Index2D
{
	Index2D(int column = 0, int row = 0);
	int column;
	int row;
};

// --- Cell ---
// ------------
struct Cell
{
	Cell(float left = 0, float bottom = 0, float width = 0, float height = 0);

	std::vector<Elite::Vector2> GetRectPoints() const;
	Elite::Vector2 GetRectCentre() const;
	
	// all the agents currently in this cell
	std::list<SteeringAgent*> agents;
	Elite::Rect boundingBox;
};

// --- Partitioned Space ---
// -------------------------
class CellSpace
{
public:
	CellSpace(float width, float height, int rows, int columns, int maxEntities);

	void AddAgent(SteeringAgent* agent);
	void UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos);

	void RegisterNeighbors(SteeringAgent* agent, float queryRadius);
	const std::vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }

	//empties the cells of entities
	void EmptyCells();

	void RenderCells()const;
	void RenderNeighbourhoodCells() const;

private:
	// Cells and properties
	std::vector<Cell> m_Cells;

	float m_SpaceWidth;
	float m_SpaceHeight;

	int m_NrOfRows;
	int m_NrOfCols;

	float m_CellWidth;
	float m_CellHeight;

	// Members to avoid memory allocation on every frame
	std::vector<SteeringAgent*> m_Neighbors;
	std::vector<int> m_CheckedCells;
	int m_NrOfNeighbors;

	// Helper functions
	inline void InitialiseGrid(int rows, int columns);

	int PositionToIndex(const Elite::Vector2 pos) const;
	Index2D PositionTo2DIndex(const Elite::Vector2 pos) const;
	inline int CellSpace::Index2DToIndex(const Index2D& index2D) const;
	
};