//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

using namespace Elite;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1.0f, 0 });
	return steering;
}

//FLEE
//****

Flee::Flee(float safeRadius)
	: m_SafeRadius{ safeRadius }
{
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Vector2& linearVelocity{ steering.LinearVelocity };
	linearVelocity = m_Target.Position - pAgent->GetPosition();
	linearVelocity.Normalize();
	linearVelocity *= -pAgent->GetMaxLinearSpeed();
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1.0f, 0 });
	float distance{ m_Target.Position.Distance(pAgent->GetPosition()) };
	if (m_SafeRadius < distance)
	{
		linearVelocity = {};
		steering.IsValid = false;
	}
	return steering;
}

float Flee::GetSafeRadius()
{
	return m_SafeRadius;
}

void Flee::SetSafeRadius(float safeRadius)
{
	m_SafeRadius = safeRadius;
}

Arrive::Arrive()
	: m_ArrivalRadius{ 1.0f }
	, m_SlowRadius{ 15.0f }
{
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Vector2& linearVelocity{ steering.LinearVelocity };
	Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };

	const float distance{ toTarget.Magnitude() };
	const float maxSpeed{ pAgent->GetMaxLinearSpeed() };
	if (distance < (m_ArrivalRadius * m_ArrivalRadius))
	{
		linearVelocity = {};
		return steering;
	}
	Vector2 velocity{ toTarget.GetNormalized() * maxSpeed };

	if (distance < m_SlowRadius * m_SlowRadius)
		velocity *= distance / (m_SlowRadius * m_SlowRadius);
	linearVelocity = velocity;
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, Elite::Color{ 0, 1.0f, 0 });
	return steering;
}

void Arrive::SetTargetRadius(float arrivalRadius)
{
	m_ArrivalRadius = arrivalRadius;
}

void Arrive::SetSlowRadius(float slowRadius)
{
	m_SlowRadius = slowRadius;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	Vector2 agentDirection{ cosf(pAgent->GetRotation()), sinf(pAgent->GetRotation()) };
	constexpr float stopAngle{ ToRadians(0.1f) };
	constexpr float slowAngle{ ToRadians(50.0f) };
	float angleBetween{ AngleBetween(toTarget, agentDirection) };
	pAgent->SetAutoOrient(false);
	if (stopAngle < angleBetween)
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
	else if (angleBetween < stopAngle)
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	if (-slowAngle < angleBetween && angleBetween < slowAngle)
		steering.AngularVelocity *= abs(angleBetween) / slowAngle;
	if (-stopAngle < angleBetween && angleBetween < stopAngle)
		steering.AngularVelocity = 0;
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), toTarget, 5, Color{ 0, 1.0f, 0 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), agentDirection, 5, Color{ 0, 0, 1.0f });
	}
	return steering;
}

Wander::Wander()
	: Wander(4.0f, 6.0f, ToRadians(45.0f))
{
}

Wander::Wander(float radius, float circleOffset, float maxAngleChange)
	: m_Radius{ radius }
	, m_Offset{ circleOffset }
	, m_MaxAngleChange{ maxAngleChange }
	, m_CurrentAngle{}
{
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float radius{ 5.0f };
	const float circleOffset{ radius + (pAgent->GetRadius() * 2) };
	SteeringOutput steering{};
	//SetTargetWithVector(pAgent);
	SetTargetWithAngle(pAgent);
	return Seek::CalculateSteering(deltaT, pAgent);
}

void Wander::SetWanderOffset(float wanderOffset)
{
	m_Offset = wanderOffset;
}

void Wander::SetTargetWithVector(SteeringAgent* pAgent)
{
	Vector2 circleCenter{ pAgent->GetPosition() + (pAgent->GetDirection() * m_Offset) };
	Vector2 randomPoint{ randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f) };
	randomPoint.Normalize();
	randomPoint *= m_Radius;
	randomPoint += circleCenter;
	SetTarget(randomPoint);
	RenderDebug(circleCenter, pAgent);
}

void Wander::SetTargetWithAngle(SteeringAgent* pAgent)
{
	Vector2 circleCenter{ pAgent->GetPosition() + (pAgent->GetDirection() * m_Offset) };
	float angleOffset{ randomFloat(-m_MaxAngleChange, m_MaxAngleChange) };
	m_CurrentAngle += angleOffset;
	Vector2 randomPoint{ cosf(m_CurrentAngle), sinf(m_CurrentAngle) };
	randomPoint *= m_Radius;
	randomPoint += circleCenter;
	SetTarget(randomPoint);
	RenderDebug(circleCenter, pAgent);
}

void Wander::RenderDebug(Vector2& circleCenter, SteeringAgent* pAgent)
{
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, Color{ 1.0f, 0, 0 }, 0);
		DEBUGRENDERER2D->DrawPoint(m_Target.Position, 5, Color{ 0, 0, 1.0f }, 0);
	}
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	float timeEstimate{ distance / pAgent->GetMaxLinearSpeed() };
	Vector2& predictedPosition{ m_Target.Position + (m_Target.LinearVelocity * timeEstimate) };
	m_Target.Position = predictedPosition;
	return Seek::CalculateSteering(deltaT, pAgent);
}

Evade::Evade(float safeRadius)
	: Flee(safeRadius)
{
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distance{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	float timeEstimate{ distance / pAgent->GetMaxLinearSpeed() };
	Vector2& predictedPosition{ m_Target.Position + (m_Target.LinearVelocity * timeEstimate) };
	m_Target.Position = predictedPosition;
	return Flee::CalculateSteering(deltaT, pAgent);
}
