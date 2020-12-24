//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "TheFlock.h"

App_Flocking::App_Flocking()
	: m_pWander{ new Wander() }
	, m_pAvoidedAgent{ new SteeringAgent() }
	, m_TheFlock{ 3000, m_TrimWorldSize, m_pAvoidedAgent, false }
{
}

//Destructor
App_Flocking::~App_Flocking()
{	
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pAvoidedAgent);
}

//Functions
void App_Flocking::Start()
{
	m_pAvoidedAgent->SetPosition(Elite::Vector2(rand() % int(m_TrimWorldSize * 2) - m_TrimWorldSize, rand() % int(m_TrimWorldSize * 2) - m_TrimWorldSize));
	m_pAvoidedAgent->SetMaxLinearSpeed(20.f);
	m_pAvoidedAgent->SetAutoOrient(true);
	m_pAvoidedAgent->SetMass(1.f);
	m_pAvoidedAgent->SetBodyColor({ 1, 0, 0 });
	m_pAvoidedAgent->SetSteeringBehavior(m_pWander);

	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 2));
}

void App_Flocking::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}
	m_pAvoidedAgent->Update(deltaTime);
	m_pAvoidedAgent->TrimToWorld(m_TrimWorldSize);
	m_TheFlock.Update(deltaTime, m_MouseTarget);
	m_TheFlock.UpdateAndRenderUI();
}

void App_Flocking::Render(float deltaTime) const
{
	m_TheFlock.Render(deltaTime);
	//m_pAvoidedAgent->Render(deltaTime);

	std::vector<Elite::Vector2> points =
	{
		{ -m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,-m_TrimWorldSize },
		{-m_TrimWorldSize,-m_TrimWorldSize }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
