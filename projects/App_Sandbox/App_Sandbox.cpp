//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Sandbox.h"
#include "SandboxAgent.h"

//Destructor
App_Sandbox::~App_Sandbox()
{
	SAFE_DELETE(m_pAgent);
}

//Functions
void App_Sandbox::Start()
{
	m_pAgent = new SandboxAgent();
}

void App_Sandbox::Update(float deltaTime)
{
	if (INPUTMANAGER->IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		const auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2  mousePos{ float(mouseData.X), float(mouseData.Y) };
		m_pAgent->SetTarget(DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(mousePos));
	}

	m_pAgent->Update(deltaTime);
}

void App_Sandbox::Render(float deltaTime) const
{
	m_pAgent->Render(deltaTime);
}