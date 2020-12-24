#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class GraphAlgorithm
	{
	public:
		GraphAlgorithm(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		virtual std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	protected:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};
	template<class T_NodeType, class T_ConnectionType>
	GraphAlgorithm<T_NodeType, T_ConnectionType>::GraphAlgorithm(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> GraphAlgorithm<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		return std::vector<T_NodeType*>();
	}

	template<class T_NodeType, class T_ConnectionType>
	float GraphAlgorithm<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}


