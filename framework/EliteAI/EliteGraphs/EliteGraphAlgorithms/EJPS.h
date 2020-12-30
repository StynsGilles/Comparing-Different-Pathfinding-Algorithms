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
			T_NodeType* pNodeJumpedFrom = nullptr;
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

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& openListRender, std::vector<T_NodeType*>& closedListRender);

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
	inline std::vector<T_NodeType*> JPS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& openListRender, std::vector<T_NodeType*>& closedListRender)
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
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pDestinationNode);
		openList.push_back(startRecord);

		while (!openList.empty())
		{
			std::sort(openList.begin(), openList.end());
			currentRecord = openList.front();

			if (currentRecord.pNode == pDestinationNode)
			{
				foundPath = true;
				break;
			}

			std::vector<NodeRecord> successors;
			IdentifySuccessors(currentRecord, pStartNode, pDestinationNode, successors);
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
				newNode.pNodeJumpedFrom = successor.pNodeJumpedFrom;
				newNode.pConnection = successor.pConnection;
				newNode.costSoFar = costSoFar;
				newNode.estimatedTotalCost = newNode.costSoFar + GetHeuristicCost(newNode.pNode, pDestinationNode);
				openList.push_back(newNode);
				openListRender.push_back(successor.pNode);
			}

			closedList.push_back(currentRecord);
			closedListRender.push_back(currentRecord.pNode);
			openList.erase(openList.begin());
		}

		if (!foundPath)
		{
			float lowestCost{ FLT_MAX };
			NodeRecord nearestNodeToEnd{};
			for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
			{
				NodeRecord& recordIterator = *it;
				float costNodeToGoal = GetHeuristicCost(recordIterator.pNode, pDestinationNode);
				if (costNodeToGoal < lowestCost && recordIterator.pNode != pDestinationNode)
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
				if (recordIterator.pNode == currentRecord.pNodeJumpedFrom)
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

			float directionX{ m_pGraph->GetNodePos(neighbor.pNode).x - m_pGraph->GetNodePos(currentRecord.pNode).x };
			float directionY{ m_pGraph->GetNodePos(neighbor.pNode).y - m_pGraph->GetNodePos(currentRecord.pNode).y };
			Elite::Vector2 directionVector{ directionX, directionY };
		
			T_NodeType* jumpNode = Jump(currentRecord, directionVector, pStartNode, pEndNode, costSoFar);
			if (jumpNode)
			{
				Elite::Vector2 previousNodePos{ jumpNode->GetPosition() - directionVector };
				auto previousNodePosGraph{ m_pGraph->GetNodeWorldPos(int(previousNodePos.x), int(previousNodePos.y)) };
				auto previousNodeIdx = m_pGraph->GetNodeFromWorldPos(previousNodePosGraph);
				auto connectionToParent = m_pGraph->GetConnection(previousNodeIdx, jumpNode->GetIndex());
				NodeRecord successor;
				successor.pNode = jumpNode;
				successor.pNodeJumpedFrom = currentRecord.pNode;
				successor.costSoFar = costSoFar;
				successor.pConnection = connectionToParent;
				successors.push_back(successor);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::PruneNeighbors(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pGoalNode)
	{
		if (currentRecord.pConnection== nullptr)
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
		int parentIndex{ currentRecord.pConnection->GetFrom() };
		auto parent = m_pGraph->GetNode(parentIndex);
		auto parentConnections = m_pGraph->GetNodeConnections(parent->GetIndex());
		Elite::Vector2 orientationVectorParent{ Clamp(m_pGraph->GetNodePos(currentRecord.pNode).x - m_pGraph->GetNodePos(parent).x  , -1.f, 1.f) ,
												Clamp(m_pGraph->GetNodePos(currentRecord.pNode).y - m_pGraph->GetNodePos(parent).y  , -1.f, 1.f) };
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
			//diagonal movement
			if (orientationVectorParent.x != 0 && orientationVectorParent.y != 0)
			{
				float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()), neighbor, parent) };
				if (costToNeighborNoCurrentRecord < costToNeighbor &&
					costToNeighborNoCurrentRecord > 0.f)
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
			//horizontal and vertical movement
			else
			{
				float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()), neighbor, parent) };
				if (costToNeighborNoCurrentRecord <= costToNeighbor&&
					costToNeighborNoCurrentRecord > 0.f)
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
		auto nextNodePosGraph{ m_pGraph->GetNodeWorldPos(int(nextNodePos.x), int(nextNodePos.y)) };
		auto nextNodeIdx = m_pGraph->GetNodeFromWorldPos(nextNodePosGraph);
		if (nextNodeIdx < 0)
		{
			return nullptr;
		}
		
		auto connection = m_pGraph->GetConnection(currentRecord.pNode->GetIndex(), nextNodeIdx);
		if (connection == nullptr)
		{
			return nullptr;
		}
		NodeRecord nextNode;
		nextNode.pNode = m_pGraph->GetNode(nextNodeIdx);

		if (nextNode.pNode == pEndNode)
		{
			return nextNode.pNode;
		}

		auto connectionsNextNode = m_pGraph->GetConnections(nextNodeIdx);

		//diagonal direction
		if (direction.x != 0 && direction.y != 0)
		{
			//check for diagonal forced neighbors
			for (auto connection : connectionsNextNode)
			{
				auto neighbor = m_pGraph->GetNode(connection->GetTo());

				if(neighbor == currentRecord.pNode)
				{
					continue;
				}

				Elite::Vector2 directionToNeighbor{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(nextNode.pNode).x  , -1.f, 1.f) ,
													Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(nextNode.pNode).y  , -1.f, 1.f) };
				if (directionToNeighbor.x != 0 && directionToNeighbor.y != 0)
				{
					float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(nextNodeIdx), neighbor, currentRecord.pNode) };
					float costToNeighbor{ m_pGraph->GetConnection(nextNodeIdx, currentRecord.pNode->GetIndex())->GetCost() + connection->GetCost() };
					if (costToNeighborNoCurrentRecord < costToNeighbor&&
						costToNeighborNoCurrentRecord > 0.f)
					{
						continue;
					}
					return nextNode.pNode;
				}
				else
				{
					continue;
				}
			}

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
				//check for horizontal forced neighbors
				for (auto connection : connectionsNextNode)
				{
					auto neighbor = m_pGraph->GetNode(connection->GetTo());

					if (neighbor == currentRecord.pNode)
					{
						continue;
					}

					Elite::Vector2 directionToNeighbor{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(nextNode.pNode).x  , -1.f, 1.f) ,
														Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(nextNode.pNode).y  , -1.f, 1.f) };
					if (directionToNeighbor.x != 0 && directionToNeighbor.y == 0)
					{
						float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(nextNodeIdx), neighbor, currentRecord.pNode) };
						float costToNeighbor{ m_pGraph->GetConnection(nextNodeIdx, currentRecord.pNode->GetIndex())->GetCost() + connection->GetCost() };
						if (costToNeighborNoCurrentRecord <= costToNeighbor &&
							costToNeighborNoCurrentRecord > 0.f)
						{
							continue;
						}
						return nextNode.pNode;
					}
					else
					{
						continue;
					}
				}
			}
			//vertical direction
			else
			{
				//check for vertical forced neighbors
				for (auto connection : connectionsNextNode)
				{
					auto neighbor = m_pGraph->GetNode(connection->GetTo());

					if (neighbor == currentRecord.pNode)
					{
						continue;
					}

					Elite::Vector2 directionToNeighbor{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(nextNode.pNode).x  , -1.f, 1.f) ,
														Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(nextNode.pNode).y  , -1.f, 1.f) };
					if (directionToNeighbor.x == 0 && directionToNeighbor.y != 0)
					{
						float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(nextNodeIdx), neighbor, currentRecord.pNode) };
						float costToNeighbor{ m_pGraph->GetConnection(nextNodeIdx, currentRecord.pNode->GetIndex())->GetCost() + connection->GetCost() };
						if (costToNeighborNoCurrentRecord <= costToNeighbor &&
							costToNeighborNoCurrentRecord > 0.f)
						{
							continue;
						}
						return nextNode.pNode;
					}
					else
					{
						continue;
					}
				}
			}
		}
		costSoFar += connection->GetCost();
		return Jump(nextNode, direction, pStartNode, pEndNode, costSoFar);
	}
}