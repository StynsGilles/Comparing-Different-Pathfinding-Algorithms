#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

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

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth{ width }
	, m_SpaceHeight{ height }
	, m_NrOfRows{ rows }
	, m_NrOfCols{ cols }
	, m_Neighbors{ maxEntities }
	, m_NrOfNeighbors{ 0 }
	, m_CellWidth{ m_SpaceWidth * 2.f / m_NrOfCols }
	, m_CellHeight{ m_SpaceHeight * 2.f / m_NrOfRows }
{
	CreateCells();
	m_Neighbors.reserve(maxEntities);
	m_Neighbors.resize(maxEntities);
}

void CellSpace::CreateCells()
{
	float cellLeft{ -m_SpaceWidth }, cellBottom{ -m_SpaceHeight };
	for (int col = 0; col < m_NrOfCols; col++)
	{
		for (int row = 0; row < m_NrOfRows; row++)
		{
			m_Cells.push_back(Cell(cellLeft + m_CellWidth * col, cellBottom + m_CellHeight * row, m_CellWidth, m_CellHeight));
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	if (PositionToIndex(agent->GetPosition()) != PositionToIndex(oldPos))
	{
		m_Cells[PositionToIndex(oldPos)].agents.remove(agent);
		m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius)
{
	m_NrOfNeighbors = 0;

	Elite::Vector2 bottomLeftPoint{ pos.x - queryRadius, pos.y - queryRadius };
	Elite::Vector2 topRightPoint{ pos.x + queryRadius, pos.y + queryRadius };

	int indexBottomLeftCell{ PositionToIndex(bottomLeftPoint) }, indexTopRightCell{ PositionToIndex(topRightPoint) };

	int bottomLeftCellCol{ indexBottomLeftCell % m_NrOfCols }, bottomLeftCellRow{ indexBottomLeftCell / m_NrOfCols };
	int topRightCellCol{ indexTopRightCell % m_NrOfCols }, topRightCellRow{ indexTopRightCell / m_NrOfCols };
	
	Elite::Vector2 distanceToAgent{};
	for (int row = bottomLeftCellRow; row <= topRightCellRow; row++)
	{
		for (int col = bottomLeftCellCol; col <= topRightCellCol; col++)
		{
			int cellIndex{ row * m_NrOfCols + col };
			for (SteeringAgent* potentialNeighbor : m_Cells[cellIndex].agents)
			{
				distanceToAgent = potentialNeighbor->GetPosition() - pos;
				if (pos != potentialNeighbor->GetPosition())
				{
					if (distanceToAgent.MagnitudeSquared() < pow(queryRadius, 2))
					{
						m_Neighbors[m_NrOfNeighbors] = potentialNeighbor;
						m_NrOfNeighbors++;
					}
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	for (Cell cell : m_Cells)
	{
		DEBUGRENDERER2D->DrawPolygon(&Elite::Polygon(cell.GetRectPoints()), Elite::Color(1.f, 0.f, 0.f, 1.f));
		DEBUGRENDERER2D->DrawString(cell.GetRectPoints()[1], std::to_string(cell.agents.size()).c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2& pos) const
{
	int col{ int(pos.x + m_SpaceWidth) / int(m_CellWidth) };
	int row{ int(pos.y + m_SpaceHeight) / int(m_CellHeight) };

	GetAxis(col, m_NrOfCols);
	GetAxis(row, m_NrOfRows);

	//col = Elite::Clamp(col, 0, m_NrOfCols - 1);
	//row = Elite::Clamp(row, 0, m_NrOfRows - 1);
	return col * m_NrOfRows + row;
}

void CellSpace::GetAxis(int& axis, const int nrOfAxis) const
{
	if (axis >= nrOfAxis)
	{
		axis = 0;
	}
	if (axis < 0)
	{
		axis = nrOfAxis - 1;
	}
}
