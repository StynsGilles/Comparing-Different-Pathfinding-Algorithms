#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class JPS
	{
	public:
		JPS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

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

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		void IdentifySuccessors(NodeRecord currentRecord, T_NodeType* pStartNode, T_NodeType* pEndNode);
		void PruneNeighbours(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pGoalNode);
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;


		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};
	template<class T_NodeType, class T_ConnectionType>
	inline JPS<T_NodeType, T_ConnectionType>::JPS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph{ pGraph }
		, m_HeuristicFunction{ hFunction }
	{
	}
	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//Here we will calculate our path using Jump Point Search
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
		}
		return finalPath;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::IdentifySuccessors(NodeRecord currentRecord, T_NodeType* pStartNode, T_NodeType* pEndNode)
	{
		std::vector<T_NodeType> successors;
		std::vector<NodeRecord> neighbours;

		for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
		{
			auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
			int differenceX{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(currentRecord.pNode).x, -1, 1) };
			int differenceY{ Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(currentRecord.pNode).y, -1, 1) };

			T_NodeType jumpNode{};
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::PruneNeighbours(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pGoalNode)
	{
		int parentIndex{ currentRecord.pConnection->GetFrom()};
		if (parentIndex < 0 || parentIndex >= m_pGraph->GetNrOfNodes())
		{
			for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
				prunedNeighbors.push_back(neighbor);
			}
			return;
		}
		T_NodeType parent = m_pGraph->GetNode(parentIndex);
		auto parentConnections = m_pGraph->GetNodeConnections(parent->GetIndex());
		Elite::Vector2 orientationVectorParent{ m_pGraph->GetNodePos(currentRecord.pNode).x - Clamp(m_pGraph->GetNodePos(parent).x  , -1, 1) ,
												m_pGraph->GetNodePos(currentRecord.pNode).y - Clamp(m_pGraph->GetNodePos(parent).y  , -1, 1) };
		std::vector<T_NodeType> neighborstoIgnore;
		neighborstoIgnore.push_back(parent);
		for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
		{
			auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
			//if the current evalued neighbor is the parent, ignore this neighbor
			if (neighbor == parent)
			{
				continue;
			}
			//if the current evalued neighbor is also a neighbor of the parent, ignore this neighbor
			auto it = std::find_if(parentConnections.begin(), parentConnections.end()[&neighbor](T_ConnectionType c)
				{
					return c->GetTo() == neighbor.GetIndex();
				});
			if (it != parentConnections.end())
			{
				neighborstoIgnore.push_back(neighbor);
				continue;
			}

			//if the current evalued neighbor is in the same direction as the record is from the parent, add it to the pruned neighbors
			Elite::Vector2 distanceVector{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(currentRecord.pNode).x, -1, 1) ,
										   Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(currentRecord.pNode).y, -1, 1) };
			if (distanceVector == orientationVectorParent)
			{
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				prunedNeighbors.push_back(neighBorRecord);
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pGoalNode);
				openList.push_back(newNode);
				continue;
			}

			float costToNeighbor{ currentRecord.pConnection->GetCost() + currentConnection.GetCost() };

			//horizontal movement
			if (orientationVectorParent.x != 0 && orientationVectorParent.y == 0 ||
				orientationVectorParent.x == 0 && orientationVectorParent.y != 0)
			{
			}

			//vertical movement
			if (orientationVectorParent.x != 0 && orientationVectorParent.y != 0)
			{

			}

			float cost = currentRecord.costSoFar + currentConnection->GetCost();
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline float JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}