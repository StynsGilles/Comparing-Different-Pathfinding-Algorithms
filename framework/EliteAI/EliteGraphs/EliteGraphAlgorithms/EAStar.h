#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
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

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& openListRender, std::vector<T_NodeType*>& closedListRender);

	private:
		void GetNearestNodeToEnd(NodeRecord& nearestNodeToEnd, vector<NodeRecord>& closedList, T_NodeType* pDestinationNode) const;
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
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
	inline std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& openListRender, std::vector<T_NodeType*>& closedListRender)
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

			//loop over all neighbors of the currentrecord
			for (auto currentConnection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				float costSoFar = currentRecord.costSoFar + currentConnection->GetCost();
				auto neighbor{ m_pGraph->GetNode(currentConnection->GetTo()) };
				bool cheaperFound{ false };
				//find the neighbor in the closed list
				for (std::vector<NodeRecord>::iterator it{ closedList.begin() }; it != closedList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (neighbor == recordIterator.pNode)
					{
						//switch the record in the closed list out with the neighbor if this record is cheaper
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
				//find the neighbor in the closed list
				for (std::vector<NodeRecord>::iterator it{ openList.begin() }; it != openList.end();)
				{
					NodeRecord& recordIterator = *it;
					if (neighbor == recordIterator.pNode)
					{
						//switch the record in the open list out with the neighbor if this record is cheaper
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
				//if there is a cheaper record found in the open list, continue to the next neighbor
				if (cheaperFound)
				{
					continue;
				}
				//add the neighbor to the open list if it doesn't exist yet
				NodeRecord newNode{};
				newNode.pNode = neighbor;
				newNode.pConnection = currentConnection;
				newNode.costSoFar = costSoFar;
				newNode.estimatedTotalCost = newNode.costSoFar + GetHeuristicCost(newNode.pNode, pDestinationNode);
				openList.push_back(newNode);
				openListRender.push_back(neighbor);
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
				if (recordIterator.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom()))
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
	inline void AStar<T_NodeType, T_ConnectionType>::GetNearestNodeToEnd(NodeRecord& nearestNodeToEnd, vector<NodeRecord>& closedList, T_NodeType* pDestinationNode) const
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
	/// Get the cost between 2 nodes on the graph using the heuristic function
	/// </summary>
	/// <typeparam name="T_NodeType">The type of node used on the graph</typeparam>
	/// <typeparam name="T_ConnectionType"><The type of connection used on the graph/typeparam>
	/// <param name="pStartNode">the start node</param>
	/// <param name="pEndNode">the endnode</param>
	/// <returns>the cost between these nodes</returns>
	template<class T_NodeType, class T_ConnectionType>
	inline float AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}