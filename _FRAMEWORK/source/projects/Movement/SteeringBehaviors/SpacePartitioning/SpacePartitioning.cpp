#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

using namespace Elite;

Index2D::Index2D(int column, int row)
{
	this->column = column;
	this->row = row;
}

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

Elite::Vector2 Cell::GetRectCentre() const
{
	Vector2 centre{ boundingBox.bottomLeft };
	centre.x += boundingBox.width / 2;
	centre.y += boundingBox.height / 2;
	return centre;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int columns, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(columns)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
	, m_CellWidth{ width / columns }
	, m_CellHeight{ height / rows }
{
	InitialiseGrid(rows, columns);
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int index{ PositionToIndex(agent->GetPosition()) };
	m_Cells[index].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	Vector2& currentPos{ agent->GetPosition() };
	int oldIndex{ PositionToIndex(oldPos) };
	int currentIndex{ PositionToIndex(agent->GetPosition()) };
	if (oldIndex != currentIndex)
	{
		m_Cells[oldIndex].agents.remove(agent);
		m_Cells[currentIndex].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	const Vector2& position{ agent->GetPosition() };
	const Vector2 offsetVector{ queryRadius, queryRadius };
	const Index2D startIndex{ PositionTo2DIndex(Vector2{ position - offsetVector }) };
	const Index2D endIndex{ PositionTo2DIndex(Vector2{ position + offsetVector }) };

	m_NrOfNeighbors = 0;
	m_CheckedCells.clear();
	Index2D cellIndex{ PositionTo2DIndex(Vector2{ position - offsetVector }) };
	for (; cellIndex.row < endIndex.row + 1; ++cellIndex.row)
	{
		for (; cellIndex.column < endIndex.column + 1; ++cellIndex.column)
		{
			int index{ Index2DToIndex(cellIndex) };
			m_CheckedCells.push_back(index);
			Cell& cell{ m_Cells[index] };
			for (SteeringAgent* cellAgent : cell.agents)
			{
				if (cellAgent != agent)
				{
					m_Neighbors[m_NrOfNeighbors] = cellAgent;
					++m_NrOfNeighbors;
				}
			}
		}
		cellIndex.column = startIndex.column;
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	size_t nrOfCells{ m_Cells.size() };
	for (size_t i{ 0 }; i < nrOfCells; ++i)
	{
		const Cell& cell{ m_Cells[i] };
		Elite::Polygon polygon{ cell.GetRectPoints() };
		DEBUGRENDERER2D->DrawPolygon(&polygon, Color{ 1.0f, 0, 0 }, 1.0f);
		Vector2 cellCentre{ cell.GetRectCentre() };
		DEBUGRENDERER2D->DrawString(cellCentre, std::to_string(cell.agents.size()).c_str());
	}
}

void CellSpace::RenderNeighbourhoodCells() const
{
	size_t nrOfCheckedCells{ m_CheckedCells.size() };
	for (size_t i{ 0 }; i < nrOfCheckedCells; ++i)
	{
		const Cell& cell{ m_Cells[m_CheckedCells[i]] };
		Elite::Polygon polygon{ cell.GetRectPoints() };
		DEBUGRENDERER2D->DrawPolygon(&polygon, Color{ 0, 0, 1.0f }, 0.9f);
	}
}

void CellSpace::InitialiseGrid(int rows, int columns)
{
	m_Cells.resize(size_t(rows * columns));
	float left, bottom{};
	for (int row{ 0 }; row < rows; ++row)
	{
		for (int column{ 0 }; column < columns; ++column)
		{
			left = m_CellWidth * column;
			bottom = m_CellHeight * row;
			m_Cells[row * columns + column] = { left, bottom, m_CellWidth, m_CellHeight };
		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	Index2D index{ PositionTo2DIndex(pos) };
	return index.row * m_NrOfCols + index.column;
}

Index2D CellSpace::PositionTo2DIndex(const Elite::Vector2 pos) const
{
	int column{ Clamp(int(pos.x / m_CellWidth), 0, m_NrOfCols - 1) };
	int row{ Clamp(int(pos.y / m_CellHeight),0, m_NrOfRows - 1) };
	return { column, row };
}

inline int CellSpace::Index2DToIndex(const Index2D& index2D) const
{
	return index2D.row * m_NrOfCols + index2D.column;
}