//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //Rescale to Max Speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 wanderDistance{ OrientationToVector(pAgent->GetOrientation()) };
	wanderDistance.Normalize();
	wanderDistance *= m_Offset;
	Elite::Vector2 circlePos{ pAgent->GetPosition().x + wanderDistance.x, pAgent->GetPosition().y + wanderDistance.y };

	srand(static_cast<unsigned int>(time(nullptr)));
	int angle{ rand() % (int(ToDegrees(m_AngleChange)) * 2) - int(ToDegrees(m_AngleChange)) };
	m_WanderAngle += ToRadians(float(angle));

	Elite::Vector2 wanderTarget{ circlePos.x + cos(m_WanderAngle) * m_Radius, circlePos.y + sin(m_WanderAngle) * m_Radius };
	steering.LinearVelocity = (wanderTarget - pAgent->GetPosition());
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), wanderDistance, 5, { 1,0,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), wanderDistance, 5, { 0,1,0,0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawCircle(circlePos, m_Radius, Elite::Color(1.f, 0.f, 0.f), 0.4f);
		DEBUGRENDERER2D->DrawCircle(wanderTarget, 0.1f, Elite::Color(0.f, 1.f, 0.f), 0.4f);
	}
	return steering;
}

//FLEE (base> SEEK)
//******
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	if (distanceToTarget > m_FleeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}

	SteeringOutput steering = Seek::CalculateSteering(deltaT, pAgent);
	steering *= -1.f;

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

//ARRIVE (base> SEEK)
//******
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const float arrivalRadius{ 1.f };
	const float slowRadius{ 15.f };

	steering.LinearVelocity = (m_Target).Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	const float distance{ float(sqrt(powf(pAgent->GetPosition().x - m_Target.Position.x,2) + pow(pAgent->GetPosition().y - m_Target.Position.y,2))) };
	if (distance <= slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius);
	}
	else if (distance<=arrivalRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	
	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, slowRadius, Elite::Color(1.f, 0.f, 0.f), 0.4f);
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, arrivalRadius, Elite::Color(0.f, 1.f, 0.f), 0.4f);
	}

	return steering;
}

//FACE (base> SEEK)
//******
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 target{ m_Target.Position - pAgent->GetPosition() };
	
	steering.AngularVelocity = Elite::OrientationToVector(pAgent->GetOrientation()).AngleWith(target);
	steering.AngularVelocity *= pAgent->GetMaxAngularSpeed();

	pAgent->SetAutoOrient(false);

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

//EVADE (base> PURSUIT)
//******
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	if (distanceToTarget > m_EvadeRadius)
	{
		SteeringOutput steering;
		steering.IsValid = false;
		return steering;
	}

	SteeringOutput steering = { Pursuit::CalculateSteering(deltaT,pAgent) };
	steering *= -1.f;

	return steering;
}

//PURSUIT
//******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 targetDirection{ m_Target.Position.x + m_Target.LinearVelocity.x, m_Target.Position.y + m_Target.LinearVelocity.y };

	steering.LinearVelocity = targetDirection - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
		DEBUGRENDERER2D->DrawDirection(m_Target.Position, m_Target.LinearVelocity, 5, { 0, 1, 0 }, 0.4f);
	}

	return steering;
}
