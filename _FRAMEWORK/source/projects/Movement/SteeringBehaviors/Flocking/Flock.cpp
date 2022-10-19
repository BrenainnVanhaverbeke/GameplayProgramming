#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "../SpacePartitioning/SpacePartitioning.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(int flockSize, float worldSize, SteeringAgent* pAgentToEvade, bool trimWorld)
	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld{ trimWorld }
	, m_pAgentToEvade{ pAgentToEvade }
	, m_NeighborhoodRadius{ 15.0f }
	, m_NrOfNeighbors{ 0 }
	, m_CanDebugRender{ false }
	, m_CanRenderNeighbourhood{ false }
	, m_CanRenderPartitioning{ false }
	, m_IsEvadeAgentEnabled{ true }
	, m_IsUsingPartitioning{ true }
	, m_pCellSpace{ new CellSpace(
		worldSize,
		worldSize,
		30,
		30,
		flockSize
	) }
	, m_DefaultSeekWeight{ 0.01f }
	, m_DefaultSeparationWeight{ 0.45f }
	, m_DefaultCohesionWeight{ 0.42f }
	, m_DefaultVelMatchWeight{ 0.2f }
	, m_DefaultWanderWeight{ 0.6f }
{
	m_Agents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize);
	InitialiseMovementBehaviour();
	InitialiseAgents(flockSize);
}

Flock::~Flock()
{
	//SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);
	SAFE_DELETE(m_pCellSpace);
	for (auto pAgent : m_Agents)
		SAFE_DELETE(pAgent);
	m_Agents.clear();
}

void Flock::Update(float deltaT)
{
	size_t nrOfAgents{ m_Agents.size() };
	if (m_IsEvadeAgentEnabled)
		UpdateToEvadeAgent(deltaT);
	for (size_t agentIndex{ 0 }; agentIndex < nrOfAgents; ++agentIndex)
	{
		SteeringAgent*& agent{ m_Agents.at(agentIndex) };
		if (m_IsEvadeAgentEnabled)
			m_pEvadeBehavior->SetTarget(UpdateEvadeTargetData());
		// TODO: See if cell updating can be disabled entirely when partitioning is off.
		// if (m_IsUsingPartitioning)
			m_pCellSpace->UpdateAgentCell(agent, agent->GetOldPosition());
		RegisterNeighbors(agent);
		agent->SetOldPosition(agent->GetPosition());
		agent->Update(deltaT);
		if (m_TrimWorld)
			agent->TrimToWorld(m_WorldSize);
		UpdateRenderBehaviour(deltaT, agent);
	}
}

void Flock::Render(float deltaT)
{
	if (m_pAgentToEvade)
		m_pAgentToEvade->Render(deltaT);
	if (m_CanRenderPartitioning)
		m_pCellSpace->RenderCells();
}

void Flock::UpdateAndRenderUI()
{
	SetupUI();
	DisplayControlsUI();
	DisplayStatsUI();
	DisplayFlockingUI();
	EndUI();
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	if (m_IsUsingPartitioning)
		m_pCellSpace->RegisterNeighbors(pAgent, m_NeighborhoodRadius);
	const std::vector<SteeringAgent*>& agents{ (m_IsUsingPartitioning ? m_pCellSpace->GetNeighbors() : m_Agents) };
	const size_t nrOfAgents{ m_IsUsingPartitioning ? m_pCellSpace->GetNrOfNeighbors() : m_Agents.size() };
	m_NrOfNeighbors = 0;
	for (size_t agentIndex{ 0 }; agentIndex < nrOfAgents; ++agentIndex)
	{
		SteeringAgent* agent{ agents[agentIndex] };
		if (pAgent != agent)
		{
			float distanceSquared{ DistanceSquared(agent->GetPosition(), pAgent->GetPosition()) };
			if (distanceSquared <= m_NeighborhoodRadius * m_NeighborhoodRadius)
			{
				m_Neighbors[m_NrOfNeighbors] = agent;
				++m_NrOfNeighbors;
			}
		}
	}
}

int Flock::GetNrOfNeighbors() const
{
	return /*m_IsUsingPartitioning ? m_pCellSpace->GetNrOfNeighbors() :*/ m_NrOfNeighbors;
}

const std::vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	return /*m_IsUsingPartitioning ? m_pCellSpace->GetNeighbors() :*/ m_Neighbors;
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	const int nrOfNeighbours{ GetNrOfNeighbors() };
	const std::vector<SteeringAgent*>& neighbours{ GetNeighbors() };
	Vector2 averagePosition{};
	for (size_t i{ 0 }; i < nrOfNeighbours; ++i)
		averagePosition += neighbours[i]->GetPosition();
	averagePosition /= nrOfNeighbours;
	return averagePosition;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	const int nrOfNeighbours{ GetNrOfNeighbors() };
	const std::vector<SteeringAgent*>& neighbours{ GetNeighbors() };
	Vector2 averageVelocity{};
	for (size_t i{ 0 }; i < nrOfNeighbours; ++i)
		averageVelocity += neighbours[i]->GetLinearVelocity();
	averageVelocity /= nrOfNeighbours;
	return averageVelocity;
}

void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior)
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if (it != weightedBehaviors.end())
			return &it->weight;
	}
	return nullptr;
}

inline void Flock::InitialiseAgents(int flockSize)
{
	for (int i{ 0 }; i < flockSize; ++i)
		m_Agents[i] = CreateAgent();
}

inline void Flock::InitialiseMovementBehaviour()
{
	m_pSeekBehavior = new Seek();
	m_pSeparationBehavior = new Separation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pWanderBehavior = new Wander();
	m_pEvadeBehavior = new Evade();
	m_pBlendedSteering = new BlendedSteering({
			{ m_pSeekBehavior, m_DefaultSeekWeight },
			{ m_pSeparationBehavior, m_DefaultSeparationWeight },
			{ m_pCohesionBehavior, m_DefaultCohesionWeight },
			{ m_pVelMatchBehavior, m_DefaultVelMatchWeight },
			{ m_pWanderBehavior, m_DefaultWanderWeight }
		});
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });
}

inline SteeringAgent* Flock::CreateAgent()
{
	const float randomX{ randomFloat(0, m_WorldSize) };
	const float randomY{ randomFloat(0, m_WorldSize) };
	SteeringAgent* pAgent{ new SteeringAgent() };
	pAgent->SetPosition(Vector2{ randomX, randomY });
	pAgent->SetSteeringBehavior(m_pPrioritySteering);
	pAgent->SetMaxLinearSpeed(70.0f);
	pAgent->SetMaxAngularSpeed(25.0f);
	pAgent->SetAutoOrient(true);
	pAgent->SetMass(1.0f);
	return pAgent;
}

inline void Flock::UpdateRenderBehaviour(float deltaT, SteeringAgent*& agent)
{
	agent->SetRenderBehavior(m_CanDebugRender);
	if (m_CanRenderNeighbourhood && agent == m_Agents[0])
		RenderNeighbourhood(deltaT);
}

inline void Flock::UpdateToEvadeAgent(float deltaT)
{
	m_pAgentToEvade->Update(deltaT);
	if (m_TrimWorld)
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
}

inline TargetData Flock::UpdateEvadeTargetData()
{
	TargetData targetData{};
	targetData.Position = m_pAgentToEvade->GetPosition();
	targetData.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	targetData.AngularVelocity = m_pAgentToEvade->GetAngularVelocity();
	return targetData;
}

void Flock::SetupUI()
{
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);
}

void Flock::DisplayControlsUI()
{
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();
	InsertSpacing();
	ImGui::Separator();
	InsertSpacing(2);
}

void Flock::DisplayStatsUI()
{
	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();
	InsertSpacing();
	ImGui::Separator();
	InsertSpacing(2);
}

inline void Flock::RenderNeighbourhood(float deltaT)
{
	SteeringAgent*& agent{ m_Agents[0] };
	if (m_CanRenderPartitioning)
		m_pCellSpace->RenderNeighbourhoodCells();
	DEBUGRENDERER2D->DrawCircle(agent->GetPosition(), m_NeighborhoodRadius, Color{ 0, 0, 1.0f }, 0);
	DEBUGRENDERER2D->DrawSolidCircle(agent->GetPosition(), 1.5f, Vector2{}, Color{ 0, 0, 1.0f }, -1);
	for (int neighbourIndex{ 0 }; neighbourIndex < GetNrOfNeighbors(); ++neighbourIndex)
	{
		const SteeringAgent* neighbour{ GetNeighbors()[neighbourIndex] };
		DEBUGRENDERER2D->DrawSolidCircle(neighbour->GetPosition(), 1.5f, Vector2{}, Color{ 0, 1.0f, 0 }, -1);
	}
}

void Flock::DisplayFlockingUI()
{
	ImGui::Text("Flocking");
	InsertSpacing();
	DisplayToggles();
	DisplayFlockingSliders();
}

inline void Flock::DisplayToggles()
{
	ImGui::Text("Render toggles");
	InsertSpacing();
	ImGui::Checkbox("Debug render steering: ", &m_CanDebugRender);
	InsertSpacing();
	ImGui::Checkbox("Debug render neighbourhood:", &m_CanRenderNeighbourhood);
	InsertSpacing();
	ImGui::Checkbox("Debug render partitioning:", &m_CanRenderPartitioning);
	InsertSpacing(2);
	ImGui::Text("Behaviour toggles");
	InsertSpacing();
	ImGui::Checkbox("Enable evade agent: ", &m_IsEvadeAgentEnabled);
	ImGui::Checkbox("Enable partitioning: ", &m_IsUsingPartitioning);
	InsertSpacing(2);
}

inline void Flock::DisplayFlockingSliders()
{
	ImGui::Text("Behaviour weights");
	InsertSpacing();
	ImGui::SliderFloat("Seek", GetWeight(m_pSeekBehavior), 0, 1.0f);
	ImGui::SliderFloat("Separation", GetWeight(m_pSeparationBehavior), 0, 1.0f);
	ImGui::SliderFloat("Cohesion", GetWeight(m_pCohesionBehavior), 0, 1.0f);
	ImGui::SliderFloat("Velocity Match", GetWeight(m_pVelMatchBehavior), 0, 1.0f);
	ImGui::SliderFloat("Wander", GetWeight(m_pWanderBehavior), 0, 1.0f);
	ImGui::SliderFloat("Neighbourhood radius", &m_NeighborhoodRadius, 0, 100.0f);
	InsertSpacing();
}

inline void Flock::EndUI()
{
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
}

void Flock::InsertSpacing(int amount)
{
	for (int i{ 0 }; i < amount; ++i)
		ImGui::Spacing();
}