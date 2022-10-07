/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../SteeringHelpers.h"
class SteeringAgent;
class Obstacle;

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

///////////////////////////////////////
//FLEE
//****
class Flee : public ISteeringBehavior
{
public:
	Flee();
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	const float m_SafeRadius;
};

///////////////////////////////////////
//ARRIVE
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive();
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	const float m_ArrivalRadius;
	const float m_SlowRadius;
};

///////////////////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

///////////////////////////////////////
//WANDER
//****
class Wander : public Seek
{
public:
	Wander();
	Wander(float radius, float circleOffset, float maxAngleChange);
	virtual ~Wander() = default;

	//Wander Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

protected:
	float m_Radius;
	float m_Offset;
	float m_MaxAngleChange;
	float m_CurrentAngle;

	void SetTargetWithVector(SteeringAgent* pAgent);
	void SetTargetWithAngle(SteeringAgent* pAgent);
	void RenderDebug(Elite::Vector2& circleCenter, SteeringAgent* pAgent);
};

#endif