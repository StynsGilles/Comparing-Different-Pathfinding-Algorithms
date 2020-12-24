#pragma once
#include "../SteeringBehaviors.h"

class Flock;

//SEPARATION - FLOCKING
//*********************
class Separation : public ISteeringBehavior
{
public:
	Separation(Flock* theFlock);

	//Separation Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pTheFlock;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public ISteeringBehavior
{
public:
	Cohesion(Flock* theFlock);
	virtual ~Cohesion() = default;

	//Cohesion Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pTheFlock;
};

//VELOCITY MATCH - FLOCKING
//************************
class Alignment : public ISteeringBehavior
{
public:
	Alignment(Flock* theFlock);
	virtual ~Alignment() = default;

	//VelocityMatch Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	Flock* m_pTheFlock;
};