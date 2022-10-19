#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

using namespace Elite;

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 averagePosition{ m_pFlock->GetAverageNeighborPos() };
	m_Target.Position = averagePosition;
	return Seek::CalculateSteering(deltaT, pAgent);
}

SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	int nrOfNeighbours{ m_pFlock->GetNrOfNeighbors() };
	if (nrOfNeighbours == 0)
	{
		steering.IsValid = false;
		return steering;
	}
	Vector2 separateDirection{};
	const std::vector<SteeringAgent*>& neighbours{ m_pFlock->GetNeighbors() };
	for (int neighbourIndex{ 0 }; neighbourIndex < nrOfNeighbours; ++neighbourIndex)
	{
		Vector2 neighbourDirection{ pAgent->GetPosition() - neighbours[neighbourIndex]->GetPosition() };
		float distance{ neighbourDirection.MagnitudeSquared() };
		separateDirection += (neighbourDirection / distance);
	}
	separateDirection /= nrOfNeighbours;
	m_Target.Position = pAgent->GetPosition() + separateDirection;
	return Seek::CalculateSteering(deltaT, pAgent);
}

VelocityMatch::VelocityMatch(Flock* pFlock)
	: m_pFlock{ pFlock }
{
}

SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Vector2 velocity{ m_pFlock->GetAverageNeighborVelocity() };
	velocity.Normalize();
	velocity *= pAgent->GetMaxLinearSpeed();
	return steering;
}
