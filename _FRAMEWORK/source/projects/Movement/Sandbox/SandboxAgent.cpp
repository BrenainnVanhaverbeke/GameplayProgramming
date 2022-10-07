#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent() 
	: BaseAgent()
	, m_MaxSpeed{ 25.0f }
	, m_ArrivalRadius{ 1.0f }
	, m_SlowRadius{ 15.0f }
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	Vector2 toTarget{ m_Target - GetPosition() };

	const float distanceSquared{ toTarget.Magnitude() };

	if (ShouldStop(distanceSquared, m_ArrivalRadius))
		return;

	Vector2 velocity{ toTarget.GetNormalized() * m_MaxSpeed };

	if (distanceSquared < m_SlowRadius * m_SlowRadius)
		velocity *= m_MaxSpeed * distanceSquared / (m_SlowRadius * m_SlowRadius);

	SetLinearVelocity(velocity);
	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		SetRotation(atan2(velocity.y, velocity.x) + E_PI_2);
	}

	SetRotation(GetRotation() + E_PI_2);
}

bool SandboxAgent::ShouldStop(float distanceSquared, float arrivalRadius)
{
	if (distanceSquared < (arrivalRadius * arrivalRadius))
	{
		SetLinearVelocity({});
		return true;
	}
	return false;
}
