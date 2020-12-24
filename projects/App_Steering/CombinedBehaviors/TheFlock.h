#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT, TargetData mouseTarget);
	void UpdateAndRenderUI();
	void Render(float deltaT) const;


	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	
	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// partioning variables
	CellSpace m_CellSpace;
	bool m_UsePartioning;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;
	vector<Elite::Vector2> m_oldPositions;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;
	
	// steering Behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr; 
	Seek* m_pSeek;
	Wander* m_pWander;
	Cohesion* m_pCohesion;
	Alignment* m_pAlignment;
	Separation* m_pSeparation;
	Evade* m_pEvade;

	// render booleans
	bool m_RenderNeighborhood;
	bool m_RenderPartitions;

	// private functions
	void InitializeFlock(const int& flockSize);
	void AddAgentsToCellSpace();
	void RegisterNeighborsWithoutPartitioning(SteeringAgent* pAgent);
	float* GetWeight(ISteeringBehavior* pBehaviour);
	void UpdateRenderSteeringAgents(const bool& renderDebug);

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};