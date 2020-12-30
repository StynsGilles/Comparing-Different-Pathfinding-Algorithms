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
		void GetNearestNodeToEnd(NodeRecord& nearestNodeToEnd, vector<NodeRecord>& closedList, T_NodeType* pDestinationNode) const;
		void IdentifySuccessors(NodeRecord currentRecord, T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<NodeRecord>& successors);
		void PruneNeighbors(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pDestinationNode);
		float GetCostNoCurrentRecord(std::list<T_ConnectionType*> connections, T_NodeType* neighbor, T_NodeType* parent) const;
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pDestinationNode) const;
		T_NodeType* Jump(NodeRecord currentRecord, Elite::Vector2 direction, T_NodeType* pStartNode, T_NodeType* pDestinationNode, float& costSoFar);

		GridGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline JPS<T_NodeType, T_ConnectionType>::JPS(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph{ pGraph }
		, m_HeuristicFunction{ hFunction }
	{
	}

	/// <summary>
	/// find a path from 2 given nodes on the graph
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="pStartNode">The start node we want to find a path FROM</param>
	/// <param name="pDestinationNode">The destination node we want to find a path TO</param>
	/// <param name="openListRender">vector of nodes that gets filled up to visualize the open list in the renderer</param>
	/// <param name="closedListRender">vector of nodes that gets filled up to visualize the closed list in the renderer</param>
	/// <returns>returns the path between the 2 nodes</returns>
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
			//sort the openlist by estimated total cost, then set the currentrecord to the first one(lowest cost)
			std::sort(openList.begin(), openList.end());
			currentRecord = openList.front();

			// if the currentrecord is the goal node we no longer have to keep searching for a path, so we can stop this while loop
			if (currentRecord.pNode == pDestinationNode)
			{
				foundPath = true;
				break;
			}

			//find all successors to jump to and loop over them
			std::vector<NodeRecord> successors;
			IdentifySuccessors(currentRecord, pStartNode, pDestinationNode, successors);
			for (auto successor : successors)
			{
				float costSoFar = successor.costSoFar;
				bool cheaperFound{ false };
				//find the successor in the closed list
				for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (successor.pNode == recordIterator.pNode)
					{
						//switch the record in the closed list out with the successor if this record is cheaper
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
				//if there is a cheaper record found in the closed list, continue to the next successor
				if (cheaperFound)
				{
					continue;
				}
				cheaperFound = false;
				//find the successor in the closed list
				for (std::vector<NodeRecord>::iterator it{ openList.begin() }; it != openList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (successor.pNode == recordIterator.pNode)
					{
						//switch the record in the open list out with the successor if this record is cheaper
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
				//if there is a cheaper record found in the open list, continue to the next successor
				if (cheaperFound)
				{
					continue;
				}
				//add the successor to the open list if it doesn't exist yet
				NodeRecord newNode{};
				newNode.pNode = successor.pNode;
				newNode.pNodeJumpedFrom = successor.pNodeJumpedFrom;
				newNode.pConnection = successor.pConnection;
				newNode.costSoFar = costSoFar;
				newNode.estimatedTotalCost = newNode.costSoFar + GetHeuristicCost(newNode.pNode, pDestinationNode);
				openList.push_back(newNode);
				openListRender.push_back(successor.pNode);
			}
			//add the current record to the closed list and remove it from the open list
			closedList.push_back(currentRecord);
			closedListRender.push_back(currentRecord.pNode);
			openList.erase(openList.begin());
		}

		//if there is no path possible, set the current record to the nearest node to the end
		if (!foundPath)
		{
			GetNearestNodeToEnd(currentRecord, closedList, pDestinationNode);
		}

		//loop over the closed list and create a path to the current node
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
		//add the start node to the finalpath and reverse the path (nodes got added in reverse order)
		finalPath.push_back(pStartNode);
		std::reverse(finalPath.begin(), finalPath.end());

		return finalPath;
	}

	/// <summary>
	/// Find the node closest to the destination node on the closed list
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="nearestNodeToEnd">noderecord to put the closest node into</param>
	/// <param name="closedList">closedlist to loop over</param>
	/// <param name="pDestinationNode">destination node of the path</param>
	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::GetNearestNodeToEnd(NodeRecord& nearestNodeToEnd, vector<NodeRecord>& closedList, T_NodeType* pDestinationNode) const
	{
		float lowestCost{ FLT_MAX };
		for (vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
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
	}

	/// <summary>
	/// find the potential successors of the current record
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="currentRecord">the record we want to get the successors of</param>
	/// <param name="pStartNode">The start node on the graph</param>
	/// <param name="pDestinationNode">The end node on the graph</param>
	/// <param name="successors">returns the successors of the current record</param>
	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::IdentifySuccessors(NodeRecord currentRecord, T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<NodeRecord>& successors)
	{
		//Get all relevant neighbors of the current record
		std::vector<NodeRecord> neighbors;
		PruneNeighbors(currentRecord, neighbors, pDestinationNode);

		//loop over the neighbors
		for (auto neighbor : neighbors)
		{
			float costSoFar = currentRecord.costSoFar;

			//Direction the neighbor is in relation with the current record
			float directionX{ m_pGraph->GetNodePos(neighbor.pNode).x - m_pGraph->GetNodePos(currentRecord.pNode).x };
			float directionY{ m_pGraph->GetNodePos(neighbor.pNode).y - m_pGraph->GetNodePos(currentRecord.pNode).y };
			Elite::Vector2 directionVector{ directionX, directionY };
		
			//Jump in the direction of this neighbor until we can't find a node anymore to find the jump point
			T_NodeType* jumpNode = Jump(currentRecord, directionVector, pStartNode, pDestinationNode, costSoFar);
			//if there is a jump node found, add it to the successors
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

	/// <summary>
	/// Prune the neighbors to only get the ones relevant for the jump point search algorithm
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="currentRecord">The record of which the neighbors get evaluated</param>
	/// <param name="prunedNeighbors">The vector where the neighbors get added to</param>
	/// <param name="pDestinationNode">The destination node on the graph that we want to get to</param>
	template<class T_NodeType, class T_ConnectionType>
	inline void JPS<T_NodeType, T_ConnectionType>::PruneNeighbors(NodeRecord currentRecord, std::vector<NodeRecord>& prunedNeighbors, T_NodeType* pDestinationNode)
	{
		//if the current record has no connection to a parent (the start node), add all his neighbors to the vector
		if (currentRecord.pConnection== nullptr)
		{
			for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pDestinationNode);
				prunedNeighbors.push_back(neighBorRecord);
			}
			return;
		}
		//get the parent of the record to find the direction it's in relation with this record
		int parentIndex{ currentRecord.pConnection->GetFrom() };
		auto parent = m_pGraph->GetNode(parentIndex);
		auto parentConnections = m_pGraph->GetNodeConnections(parent->GetIndex());
		Elite::Vector2 orientationVectorParent{ Clamp(m_pGraph->GetNodePos(currentRecord.pNode).x - m_pGraph->GetNodePos(parent).x  , -1.f, 1.f) ,
												Clamp(m_pGraph->GetNodePos(currentRecord.pNode).y - m_pGraph->GetNodePos(parent).y  , -1.f, 1.f) };
		//loop over all connections of the record
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

			//if the currently evalued neighbor is in the same direction as the record is from the parent, add it to the pruned neighbors
			Elite::Vector2 orientationVectorNeighbor{ Clamp(m_pGraph->GetNodePos(neighbor).x - m_pGraph->GetNodePos(currentRecord.pNode).x, -1.f, 1.f) ,
													  Clamp(m_pGraph->GetNodePos(neighbor).y - m_pGraph->GetNodePos(currentRecord.pNode).y, -1.f, 1.f) };
			if (orientationVectorNeighbor == orientationVectorParent)
			{
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pDestinationNode);
				prunedNeighbors.push_back(neighBorRecord);
				continue;
			}
			float costToNeighbor{ currentRecord.pConnection->GetCost() + currentConnection->GetCost() };
			//diagonal movement
			if (orientationVectorParent.x != 0 && orientationVectorParent.y != 0)
			{
				//if the cost to get to te currently evalued neighbor is (strictly) lower without going through the current record than with, ignore it (prune)
				//otherwise add it to the neighbors
				float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()), neighbor, parent) };
				if (costToNeighborNoCurrentRecord < costToNeighbor)
				{
					continue;
				}
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pDestinationNode);
				prunedNeighbors.push_back(neighBorRecord);
				continue;
			}
			//horizontal and vertical movement
			else
			{
				//if the cost to get to te currently evalued neighbor is lower or equal without going through the current record than with, ignore it (prune)
				//otherwise add it to the neighbors
				float costToNeighborNoCurrentRecord{ GetCostNoCurrentRecord(m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()), neighbor, parent) };
				if (costToNeighborNoCurrentRecord <= costToNeighbor)
				{
					continue;
				}
				NodeRecord neighBorRecord;
				neighBorRecord.pNode = neighbor;
				neighBorRecord.pConnection = currentConnection;
				neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pDestinationNode);
				prunedNeighbors.push_back(neighBorRecord);
				continue;
			}

			//forced neighbors
			NodeRecord neighBorRecord;
			neighBorRecord.pNode = neighbor;
			neighBorRecord.pConnection = currentConnection;
			neighBorRecord.costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
			neighBorRecord.estimatedTotalCost = neighBorRecord.costSoFar + GetHeuristicCost(neighBorRecord.pNode, pDestinationNode);
			prunedNeighbors.push_back(neighBorRecord);
		}
	}

	/// <summary>
	/// Gets the cost to get to a neighbor, without going through the current record
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="connections">all the connections of the current record</param>
	/// <param name="neighbor">evalued neighbor</param>
	/// <param name="parent">parent of the current record</param>
	/// <returns>returns the cost to the neighbor without going through the current record (if possible)</returns>
	template<class T_NodeType, class T_ConnectionType>
	inline float JPS<T_NodeType, T_ConnectionType>::GetCostNoCurrentRecord(std::list<T_ConnectionType*> connections, T_NodeType* neighbor, T_NodeType* parent) const
	{
		//loop over the connections of the parent
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
		//return 0 if there is no possible (low enough) path from the parent to this neighbor
		return 0.0f;
	}

	/// <summary>
	/// Get the cost between 2 nodes on the graph using the heuristic function
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="pStartNode">the start node</param>
	/// <param name="pEndNode">the endnode</param>
	/// <returns>the cost between these nodes</returns>
	template<class T_NodeType, class T_ConnectionType>
	inline float JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}

	/// <summary>
	/// Jump in a certain direction until it is no longer possible to continue
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="currentRecord">The record to jump from</param>
	/// <param name="direction">The direction to jump into</param>
	/// <param name="pStartNode">The start node of the path to find</param>
	/// <param name="pEndNode">The destination node of the path to find</param>
	/// <param name="costSoFar">Cost of the jump</param>
	/// <returns>The found node to jump to</returns>
	template<class T_NodeType, class T_ConnectionType>
	inline T_NodeType* JPS<T_NodeType, T_ConnectionType>::Jump(NodeRecord currentRecord, Elite::Vector2 direction, T_NodeType* pStartNode, T_NodeType* pDestinationNode, float& costSoFar)
	{
		Elite::Vector2 nextNodePos{ currentRecord.pNode->GetPosition() + direction };
		auto nextNodePosGraph{ m_pGraph->GetNodeWorldPos(int(nextNodePos.x), int(nextNodePos.y)) };
		auto nextNodeIdx = m_pGraph->GetNodeFromWorldPos(nextNodePosGraph);
		//if there is no node to jump to, return nullptr
		if (nextNodeIdx < 0)
		{
			return nullptr;
		}
		
		//if there is a node, but no way to jump to it (no connection), return nullptr
		auto connection = m_pGraph->GetConnection(currentRecord.pNode->GetIndex(), nextNodeIdx);
		if (connection == nullptr)
		{
			return nullptr;
		}
		NodeRecord nextNode;
		nextNode.pNode = m_pGraph->GetNode(nextNodeIdx);

		//if the node to jump to is the destination node, return it
		if (nextNode.pNode == pDestinationNode)
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

				//if the neighbor to check is the original node, go to the next connection
				if(neighbor == currentRecord.pNode)
				{
					continue;
				}

				//if the neighbor is diagonal, check if it's forced
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
			}

			//check for horizontal and vertical forced neighbors
			if (Jump(nextNode, Elite::Vector2(direction.x, 0.f), pStartNode, pDestinationNode, costSoFar) != nullptr ||
				Jump(nextNode, Elite::Vector2(0.f, direction.y), pStartNode, pDestinationNode, costSoFar) != nullptr)
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

					//if the neighbor to check is the original node, go to the next connection
					if (neighbor == currentRecord.pNode)
					{
						continue;
					}

					//if the neighbor is horizontal, check if it's forced
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
				}
			}
			//vertical direction
			else
			{
				//check for vertical forced neighbors
				for (auto connection : connectionsNextNode)
				{
					auto neighbor = m_pGraph->GetNode(connection->GetTo());

					//if the neighbor to check is the original node, go to the next connection
					if (neighbor == currentRecord.pNode)
					{
						continue;
					}

					//if the neighbor is vertical, check if it's forced
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
				}
			}
		}
		//increment the cost of the jump with the cost of the current connection and jump to the next node
		costSoFar += connection->GetCost();
		return Jump(nextNode, direction, pStartNode, pDestinationNode, costSoFar);
	}
}