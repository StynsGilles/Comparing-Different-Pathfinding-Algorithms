#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"
#include "TheFlock.h"

//*********************
//SEPARATION (FLOCKING)
Separation::Separation(Flock* theFlock)
	: m_pTheFlock{ theFlock }
{
}

SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	//FIX THIS, ERROR IF U DONT FIX THIS PLEAAASE
	SteeringOutput steering = {};
	float strength{}, decayCoefficient{ 500.f };
	float distanceSquared{};
	Elite::Vector2 direction{};

	for (int idx = 0; idx < m_pTheFlock->GetNrOfNeighbors(); idx++)
	{
		direction = m_pTheFlock->GetNeighbors()[idx]->GetPosition() - pAgent->GetPosition();
		distanceSquared = (direction).MagnitudeSquared();
		strength = min(decayCoefficient / distanceSquared, pAgent->GetMaxLinearSpeed());
		direction.Normalize();
		direction *= -1.f;
		steering.LinearVelocity += strength * direction;
	}

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0 }, 0.4f);

	return steering;
}

//*******************
//COHESION (FLOCKING)
Cohesion::Cohesion(Flock* theFlock)
	: m_pTheFlock{ theFlock }
{
}

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_pTheFlock->GetAverageNeighborPos() - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0 }, 0.4f);

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
Alignment::Alignment(Flock* theFlock)
	: m_pTheFlock{ theFlock }
{
}

SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pTheFlock->GetAverageNeighborVelocity();

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0 }, 0.4f);

	return steering;
}