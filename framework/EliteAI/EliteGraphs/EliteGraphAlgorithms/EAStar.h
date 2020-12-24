#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar: public GraphAlgorithm<T_NodeType, T_ConnectionType>
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode) override;

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//Here we will calculate our path using AStar
		bool foundPath{ false };
		vector<T_NodeType*> finalPath;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord{};
		NodeRecord startRecord{};
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(startRecord);

		while (!openList.empty())
		{
			std::sort(openList.begin(), openList.end());
			currentRecord = openList.front();

			if (currentRecord.pNode == pGoalNode)
			{
				foundPath = true;
				break;
			}

			for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				float costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
				bool cheaperFound{ false };
				for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (neighbor == recordIterator.pNode)
					{
						if (recordIterator.costSoFar > costSoFar)
						{
							std::iter_swap(it, closedList.end() - 1);
							closedList.pop_back();
						}
						else
						{
							cheaperFound = true;
							++it;
						}
						break;
					}
					else
						++it;
				}
				if (cheaperFound)
				{
					continue;
				}
				cheaperFound = false;
				for (std::vector<NodeRecord>::iterator it{ openList.begin() }; it != openList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (neighbor == recordIterator.pNode)
					{
						if (recordIterator.costSoFar > costSoFar)
						{
							std::iter_swap(it, openList.end() - 1);
							openList.pop_back();
						}
						else
						{
							++it;
							cheaperFound = true;
						}
						break;
					}
					else
						++it;
				}
				if (cheaperFound)
				{
					continue;
				}
				NodeRecord newNode{};
				newNode.pNode = m_pGraph->GetNode(currentConnection->GetTo());
				newNode.pConnection = currentConnection;
				newNode.costSoFar = costSoFar;
				newNode.estimatedTotalCost = newNode.costSoFar + GetHeuristicCost(newNode.pNode, pGoalNode);
				openList.push_back(newNode);
			}

			closedList.push_back(currentRecord);
			openList.erase(openList.begin());
		}

		if (!foundPath)
		{
			float lowestCost{ FLT_MAX };
			NodeRecord nearestNodeToEnd{};
			for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
			{
				NodeRecord& recordIterator = *it;
				float costNodeToGoal = GetHeuristicCost(recordIterator.pNode, pGoalNode);
				if (costNodeToGoal < lowestCost && recordIterator.pNode != pGoalNode)
				{
					lowestCost = costNodeToGoal;
					nearestNodeToEnd = recordIterator;
				}
				++it;
			}
			currentRecord = nearestNodeToEnd;
		}

		while (currentRecord.pNode != pStartNode)
		{
			finalPath.push_back(currentRecord.pNode);
			for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
			{
				NodeRecord& recordIterator = *it;
				if (recordIterator.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom()))
				{
					currentRecord = recordIterator;
					break;
				}
				else
					++it;
			}
		}
		finalPath.push_back(pStartNode);
		std::reverse(finalPath.begin(), finalPath.end());

		return finalPath;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}