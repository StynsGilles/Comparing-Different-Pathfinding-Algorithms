/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
class SteeringAgent;
using namespace Elite;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetWanderOffset(float offset) { m_Offset = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_AngleChange = rad; }

protected:
	float m_Offset = 6.f;
	float m_Radius = 4.f;
	float m_AngleChange = ToRadians(45);
	float m_WanderAngle = 0.f;

private:
	//void SetTarget(const TargetData* mTarget)override {}
};

//////////////////////////
//FLEE
//******
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	float m_FleeRadius = 15.f;
};

//////////////////////////
//ARRIVE
//******
class Arrive : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//FACE
//******
class Face: public Seek
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//PURSUIT
//******
class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Pursuit behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//EVADE
//******
class Evade : public Pursuit
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Evade behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	float m_EvadeRadius = 15.f;
};
#endif