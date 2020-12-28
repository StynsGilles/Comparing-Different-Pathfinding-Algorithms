#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class JPS
	{
	public:
		JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

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
		void IdentifySuccessors(NodeRecord currentRecord, T_NodeType* pStartNode, T_NodeType* pEndNode, std::vector<NodeRecord>& successors);
		void PruneNeighbors(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pGoalNode);
		float GetCostNoCurrentRecord(std::list<T_ConnectionType*> connections, T_NodeType* neighbor, T_NodeType* parent) const;
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;
		T_NodeType* Jump(NodeRecord currentRecord, Elite::Vector2 direction, T_NodeType* pStartNode, T_NodeType* pEndNode, float& costSoFar);

		GridGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline JPS<T_NodeType, T_ConnectionType>::JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
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

			std::vector<NodeRecord> successors;
			IdentifySuccessors(currentRecord, pStartNode, pGoalNode, successors);

			for (auto successor : successors)
			{
				float costSoFar = successor.costSoFar;
				bool cheaperFound{ false };
				for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (successor.pNode == recordIterator.pNode)
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
					if (successor.pNode == recordIterator.pNode)
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
				newNode.pNode = successor.pNode;
				//newNode.pConnection = currentConnection;
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

	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::IdentifySuccessors(NodeRecord currentRecord, T_NodeType* pStartNode, T_NodeType* pEndNode, std::vector<NodeRecord>& successors)
	{
		std::vector<NodeRecord> neighbors;
		PruneNeighbors(currentRecord, neighbors, pEndNode);

		for (auto neighbor : neighbors)
		{
			float costSoFar = currentRecord.costSoFar;
			float directionX{ Elite::Clamp(m_pGraph->GetNodePos(neighbor.pNode).x - m_pGraph->GetNodePos(currentRecord.pNode).x, -1.f, 1.f) };
			float directionY{ Elite::Clamp(m_pGraph->GetNodePos(neighbor.pNode).y - m_pGraph->GetNodePos(currentRecord.pNode).y, -1.f, 1.f) };
			Elite::Vector2 directionVector{ directionX, directionY };
		
			T_NodeType* jumpNode = Jump(currentRecord, directionVector, pStartNode, pEndNode, costSoFar);
			if (jumpNode)
			{
				NodeRecord successor;
				successor.pNode = jumpNode;
				successor.costSoFar = costSoFar;
				successors.push_back(successor);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::PruneNeighbors(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pGoalNode)
	{
		int parentIndex{ currentRecord.pConnection->GetFrom()};
		if (parentIndex < 0 || parentIndex >= m_pGraph->GetNrOfNodes())
		{
			for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pGoalNode);
				prunedNeighbors.push_back(neighBorRecord);
			}
			return;
		}
		auto parent = m_pGraph->GetNode(parentIndex);
		auto parentConnections = m_pGraph->GetNodeConnections(parent->GetIndex());
		Elite::Vector2 orientationVectorParent{ m_pGraph->GetNodePos(currentRecord.pNode).x - Clamp(m_pGraph->GetNodePos(parent).x  , -1.f, 1.f) ,
												m_pGraph->GetNodePos(currentRecord.pNode).y - Clamp(m_pGraph->GetNodePos(parent).y  , -1.f, 1.f) };
		for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
		{
			auto neighbor = m_pGraph->GetNode(currentConnection->GetTo());
			//if the current evalued neighbor is the parent, ignore this neighbor
			if (neighbor == parent)
			{
				continue;
			}
			//if the current evalued neighbor is also a neighbor of the parent, ignore this neighbor
			auto it = std::find_if(parentConnections.begin(), parentConnections.end(), [&neighbor](T_ConnectionType* c)
				{
					return c->GetTo() == neighbor->GetIndex();
				});
			if (it != parentConnections.end())
			{
				continue;
			}

			//if the current evalued neighbor is in the same direction as the record is from the parent, add it to the pruned neighbors
			Elite::Vector2 orientationVectorNeighbor{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(currentRecord.pNode).x, -1.f, 1.f) ,
													  Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(currentRecord.pNode).y, -1.f, 1.f) };
			if (orientationVectorNeighbor == orientationVectorParent)
			{
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pGoalNode);
				prunedNeighbors.push_back(neighBorRecord);
				continue;
			}

			float costToNeighbor{ currentRecord.pConnection->GetCost() + currentConnection->GetCost() };

			//vertical movement
			if (orientationVectorParent.x != 0 && orientationVectorParent.y != 0)
			{
				if (GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()), neighbor, parent) < costToNeighbor)
				{
					continue;
				}
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pGoalNode);
				prunedNeighbors.push_back(neighBorRecord);
				continue;
			}
			//horizontal movement
			else
			{
				if (GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()), neighbor, parent) <= costToNeighbor)
				{
					continue;
				}
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pGoalNode);
				prunedNeighbors.push_back(neighBorRecord);
				continue;
			}

			//forced neighbors
			NodeRecord neighBorRecord;
			neighBorRecord.pNode = neighbor;
			neighBorRecord.pConnection = currentConnection;
			neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
			neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pGoalNode);
			prunedNeighbors.push_back(neighBorRecord);
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline float JPS<T_NodeType, T_ConnectionType>::GetCostNoCurrentRecord(std::list<T_ConnectionType*> connections, T_NodeType* neighbor, T_NodeType* parent) const
	{
		for (auto currentConnectionParent : m_pGraph->GetNodeConnections(parent->GetIndex()))
		{
			float costToNeighbor{ 0.f };
			//neighbor of the parent at the end of this connection
			auto neighborParent{ m_pGraph->GetNode(currentConnectionParent->GetTo()) };

			//check if this neighbor is in the connections list of the current record
			auto connectionIt = std::find_if(connections.begin(), connections.end(),[&neighborParent, this](T_ConnectionType* c)
				{
					return neighborParent == this->m_pGraph->GetNode(c->GetTo());
				});

			if (connectionIt != connections.end())
			{
				costToNeighbor += currentConnectionParent->GetCost();
				//check if this neighbor has a connection with the goal node.
				std::list<T_ConnectionType*> connectionsOfConnection = m_pGraph->GetNodeConnections((*connectionIt)->GetTo());
				auto goalIt = std::find_if(connectionsOfConnection.begin(), connectionsOfConnection.end(), [&neighbor, this](T_ConnectionType* c)
					{
						return neighbor == this->m_pGraph->GetNode(c->GetTo());
					});
				if (goalIt != connectionsOfConnection.end())
				{
					costToNeighbor += (*goalIt)->GetCost();
					return costToNeighbor;
				}
			}
		}
		return 0.0f;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline float JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}

	template<class T_NodeType, class T_ConnectionType>
	inline T_NodeType* JPS<T_NodeType, T_ConnectionType>::Jump(NodeRecord currentRecord, Elite::Vector2 direction, T_NodeType* pStartNode, T_NodeType* pEndNode, float& costSoFar)
	{
		Elite::Vector2 nextNodePos{ currentRecord.pNode->GetPosition() + direction };

		auto nextNodeIdx = m_pGraph->GetNodeFromWorldPos(nextNodePos);
		if (nextNodeIdx < 0)
		{
			return nullptr;
		}
		
		auto connection = m_pGraph->GetConnection(currentRecord.pNode->GetIndex(), nextNodeIdx);
		NodeRecord nextNode;
		nextNode.pNode = m_pGraph->GetNode(nextNodeIdx);

		if (nextNode.pNode == pEndNode)
		{
			return nextNode.pNode;
		}

		//diagonal direction
		if (direction.x != 0 && direction.y != 0)
		{
			//TODO: check for diagonal forced neighbors
			//if (/*check for diagonal forced neighbors*/)
			//{
			//	return nextNode;
			//}

			if (Jump(nextNode, Elite::Vector2(direction.x, 0.f), pStartNode, pEndNode, costSoFar) != nullptr ||
				Jump(nextNode, Elite::Vector2(0.f, direction.y), pStartNode, pEndNode, costSoFar) != nullptr)
			{
				return nextNode.pNode;
			}
		}
		else
		{
			//horizontal direction
			if (direction.x != 0)
			{
				//TODO: check for horizontal forced neighbors
				//if (/*check for horizontal forced neighbors*/)
				//{
				//	return nextNode;
				//}
			}
			//vertical direction
			else
			{
				//TODO: check for vertical forced neighbors
				//if (/*check for vertical forced neighbors*/)
				//{
				//	return nextNode;
				//}
			}
		}

		costSoFar += connection->GetCost();
		return Jump(nextNode, direction, pStartNode, pEndNode, costSoFar);
	}
}