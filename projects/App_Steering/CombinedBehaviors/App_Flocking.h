#ifndef STEERINGBEHAVIORS_APPLICATION_H
#define STEERINGBEHAVIORS_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "../SteeringBehaviors.h"
#include "TheFlock.h"

class SteeringAgent;
class Flock;

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_Flocking final : public IApp
{
public:
	//Constructor & Destructor
	App_Flocking();
	virtual ~App_Flocking();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	TargetData m_MouseTarget = {};
	bool m_UseMouseTarget = true;
	bool m_VisualizeMouseTarget = true;
	
	float m_TrimWorldSize = 100.f;

	//C++ make the class non-copyable
	App_Flocking(const App_Flocking&) = delete;
	App_Flocking& operator=(const App_Flocking&) = delete;

	//Agent to avoid
	SteeringAgent* m_pAvoidedAgent;
	Wander* m_pWander;

	//Create the Flock
	Flock m_TheFlock;
};
#endif