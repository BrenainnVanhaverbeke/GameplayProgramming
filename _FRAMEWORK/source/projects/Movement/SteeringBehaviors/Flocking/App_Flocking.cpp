//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flocking.h"
#include "../SteeringAgent.h"
#include "Flock.h"

using namespace Elite;

App_Flocking::App_Flocking()
	: m_UseMouseTarget{ true }
	, m_VisualizeMouseTarget{ true }
	, m_TrimWorldSize{ 500.0f }
	, m_FlockSize{ 5000 }
	, m_pFlock{ nullptr }
	, m_pAgentToEvade{ nullptr }
	, m_pAgentToEvadeBehaviour{ nullptr }
{
}

//Destructor
App_Flocking::~App_Flocking()
{
	SAFE_DELETE(m_pFlock);
	SAFE_DELETE(m_pAgentToEvadeBehaviour);
	SAFE_DELETE(m_pAgentToEvade);
}

//Functions
void App_Flocking::Start()
{
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(55.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(m_TrimWorldSize / 1.5f, m_TrimWorldSize / 2));
	m_pAgentToEvade = CreateAgentToEvade();
	m_pFlock = new Flock(m_FlockSize, m_TrimWorldSize, m_pAgentToEvade, true);
}

void App_Flocking::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

	m_pFlock->UpdateAndRenderUI();
	m_pFlock->Update(deltaTime);
	if (m_UseMouseTarget)
		m_pFlock->SetTarget_Seek(m_MouseTarget);
}

void App_Flocking::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	m_pFlock->Render(deltaTime);

	//Render Target
	if (m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f }, -0.8f);
}

inline SteeringAgent* App_Flocking::CreateAgentToEvade()
{
	const float randomX{ randomFloat(0, m_TrimWorldSize) };
	const float randomY{ randomFloat(0, m_TrimWorldSize) };
	SteeringAgent* pAgent{ new SteeringAgent() };
	pAgent->SetPosition(Vector2{ randomX, randomY });
	pAgent->SetSteeringBehavior(m_pAgentToEvadeBehaviour = new Wander());
	pAgent->SetMaxLinearSpeed(70.0f);
	pAgent->SetMaxAngularSpeed(25.0f);
	pAgent->SetAutoOrient(true);
	pAgent->SetMass(1.0f);
	return pAgent;
}