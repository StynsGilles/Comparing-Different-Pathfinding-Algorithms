#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
	, m_pSeek{ new Seek() }
	, m_pWander{ new Wander() }
	, m_pCohesion{ new Cohesion(this) }
	, m_pSeparation{ new Separation(this) }
	, m_pAlignment{ new Alignment(this) }
	, m_pEvade{ new Evade() }
	, m_RenderNeighborhood{ true }
	, m_RenderPartitions{ true }
	, m_CellSpace{ worldSize, worldSize , 25, 25, m_FlockSize }
	, m_UsePartioning{ true }
{
	srand(static_cast<unsigned int>(time(nullptr)));
	InitializeFlock(flockSize);
	AddAgentsToCellSpace();
}

Flock::~Flock()
{
	for (SteeringAgent* agent : m_Agents)
	{
		SAFE_DELETE(agent);
	}
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeparation);
	SAFE_DELETE(m_pAlignment);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
}

void Flock::InitializeFlock(const int& flockSize)
{
	m_pBlendedSteering = new BlendedSteering({ {m_pSeek, 0.2f}, {m_pWander, 0.2f}, {m_pCohesion,0.2f}, {m_pSeparation,0.2f}, {m_pAlignment,0.2f} });
	m_pPrioritySteering = new PrioritySteering({ m_pEvade, m_pBlendedSteering });
	for (int idx = 0; idx < flockSize; idx++)
	{
		SteeringAgent* newAgent = new SteeringAgent();
		newAgent->SetPosition(Elite::Vector2(rand() % int(m_WorldSize * 2) - m_WorldSize, rand() % int(m_WorldSize * 2) - m_WorldSize));
		newAgent->SetMaxLinearSpeed(20.f);
		newAgent->SetAutoOrient(true);
		newAgent->SetMass(1.f);
		newAgent->SetBodyColor({ 1, 1, 0 });
		newAgent->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents.push_back(newAgent);
		m_oldPositions.push_back(newAgent->GetPosition());
	}
	m_Neighbors.reserve(flockSize);
	m_Neighbors.resize(flockSize);
}

void Flock::AddAgentsToCellSpace()
{
	for (SteeringAgent* agent : m_Agents)
	{
		m_CellSpace.AddAgent(agent);
	}
}

void Flock::Update(float deltaT, TargetData mouseTarget)
{
	m_pSeek->SetTarget(mouseTarget);
	m_pEvade->SetTarget(m_pAgentToEvade->GetPosition());
	for (size_t idx = 0; idx < m_Agents.size(); idx++)
	{
		m_CellSpace.UpdateAgentCell(m_Agents[idx], m_oldPositions[idx]);
		RegisterNeighbors(m_Agents[idx]);
		m_Agents[idx]->Update(deltaT);
		m_Agents[idx]->TrimToWorld(m_WorldSize);
		m_oldPositions[idx] = m_Agents[idx]->GetPosition();
	}
}

void Flock::Render(float deltaT) const
{
	if (m_RenderNeighborhood)
	{
		DEBUGRENDERER2D->DrawCircle(m_Agents.back()->GetPosition(), m_NeighborhoodRadius, Elite::Color(1.f, 0.f, 0.f), 0.4f);
		for (int idx = 0; idx < m_NrOfNeighbors; idx++)
		{
			DEBUGRENDERER2D->DrawSolidCircle(m_Neighbors[idx]->GetPosition(), m_Neighbors[idx]->GetRadius(), { 0,0 }, Elite::Color(0.f, 1.f, 0.f, 0.5f));
		}
	}
	if (m_RenderPartitions)
	{
		m_CellSpace.RenderCells();
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	bool renderSteering = m_Agents[0]->CanRenderBehavior();
	ImGui::Checkbox("Debug render steering", &renderSteering);
	ImGui::Checkbox("Debug render neighborhood", &m_RenderNeighborhood);
	ImGui::Checkbox("Debug render partitions", &m_RenderPartitions);
	ImGui::Checkbox("Use Cell partioning", &m_UsePartioning);
	UpdateRenderSteeringAgents(renderSteering);

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	if (m_UsePartioning)
	{
		m_CellSpace.RegisterNeighbors(pAgent->GetPosition(), m_NeighborhoodRadius);
		m_Neighbors = m_CellSpace.GetNeighbors();
		m_NrOfNeighbors = m_CellSpace.GetNrOfNeighbors();
	}
	else
	{
		RegisterNeighborsWithoutPartitioning(pAgent);
	}
}

void Flock::RegisterNeighborsWithoutPartitioning(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	Elite::Vector2 distanceToAgent{};
	for (SteeringAgent* potentialNeighbor : m_Agents)
	{
		distanceToAgent = potentialNeighbor->GetPosition() - pAgent->GetPosition();
		if (pAgent != potentialNeighbor)
		{
			if (distanceToAgent.MagnitudeSquared() < pow(m_NeighborhoodRadius, 2))
			{
				m_Neighbors[m_NrOfNeighbors] = potentialNeighbor;
				m_NrOfNeighbors++;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	float totalX{}, totalY{};
	for (int idx = 0; idx < m_NrOfNeighbors; idx++)
	{
		totalX += m_Neighbors[idx]->GetPosition().x;
		totalY += m_Neighbors[idx]->GetPosition().y;
	}
	return Elite::Vector2{ totalX / m_NrOfNeighbors, totalY / m_NrOfNeighbors };
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 totalVelocity{};
	if (m_NrOfNeighbors > 0)
	{
		for (int idx = 0; idx < m_NrOfNeighbors; idx++)
		{
			totalVelocity += m_Neighbors[idx]->GetLinearVelocity();
		}
		return totalVelocity / float(m_NrOfNeighbors);
	}
	return Elite::Vector2(0.f, 0.f);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::UpdateRenderSteeringAgents(const bool& renderDebug)
{
	for (SteeringAgent* agent : m_Agents)
	{
		agent->SetRenderBehavior(renderDebug);
	}
}