#pragma once
#include "EGraphAlgorithm.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class JPS : public GraphAlgorithm<T_NodeType, T_ConnectionType>
	{
	public:
		JPS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode) override;

	private:
	};
	template<class T_NodeType, class T_ConnectionType>
	inline JPS<T_NodeType, T_ConnectionType>::JPS(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: GraphAlgorithm(pGraph, hFunction)
	{
	}
	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> Elite::JPS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		//Here we will calculate our path using Jump Point Search
		vector<T_NodeType*> finalPath;

		return finalPath;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::JPS<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}