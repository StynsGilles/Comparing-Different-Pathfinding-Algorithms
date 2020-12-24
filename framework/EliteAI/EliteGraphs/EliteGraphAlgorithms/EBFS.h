#pragma once
#include "EGraphAlgorithm.h"

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS : public GraphAlgorithm<T_NodeType, T_ConnectionType>
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode) override;
	private:
	};

	template<class T_NodeType, class T_ConnectionType>
	inline BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		:GraphAlgorithm(pGraph, hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		//Here we will calculate our path using BFS
		bool foundPath{ false };
		std::queue<T_NodeType*> openList;
		std::map<T_NodeType*, T_NodeType*> closedList;

		openList.push(pStartNode);

		std::cout << "getting nodes from the open list" << std::endl;
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
					closedList[nextNode] = currentNode;
				}
			}
		}

		std::cout << "tracking back" << std::endl;
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
}

