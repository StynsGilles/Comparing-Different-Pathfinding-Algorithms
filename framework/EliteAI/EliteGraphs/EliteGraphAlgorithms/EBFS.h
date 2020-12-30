#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& openListRender, std::vector<T_NodeType*>& closedListRender);
	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph{ pGraph }
		, m_HeuristicFunction{ hFunction }
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& openListRender, std::vector<T_NodeType*>& closedListRender)
	{
		//Here we will calculate our path using BFS
		bool foundPath{ false };
		std::queue<T_NodeType*> openList;
		std::map<T_NodeType*, T_NodeType*> closedList;

		openList.push(pStartNode);

		while (!openList.empty() && !foundPath)
		{
			T_NodeType* currentNode = openList.front();
			openList.pop();

			if (currentNode == pDestinationNode)
			{
				foundPath = true;
				break;
			}

			for (auto con : m_pGraph->GetNodeConnections(currentNode->GetIndex()))
			{
				T_NodeType* nextNode = m_pGraph->GetNode(con->GetTo());
				if (closedList.find(nextNode) == closedList.end())
				{
					openList.push(nextNode);
					openListRender.push_back(nextNode);
					closedList[nextNode] = currentNode;
					closedListRender.push_back(currentNode);
				}
			}
		}

		//Track back
		vector<T_NodeType*> path;

		T_NodeType* currentNode = nullptr;
		if (foundPath)
		{
			currentNode = pDestinationNode;
		}
		else
		{
			currentNode = pStartNode;
		}

		while (currentNode != pStartNode)
		{
			path.push_back(currentNode);
			currentNode = closedList[currentNode];
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}
	template<class T_NodeType, class T_ConnectionType>
	inline float BFS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}

