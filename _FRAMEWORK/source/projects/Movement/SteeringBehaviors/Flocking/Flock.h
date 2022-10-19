#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class CellSpace;

class Flock final
{
public:
	Flock(
		int flockSize, 
		float worldSize, 
		SteeringAgent* pAgentToEvade, 
		bool trimWorld);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI() ;
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const;
	const std::vector<SteeringAgent*>& GetNeighbors() const;

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetTarget_Seek(TargetData target);
	void SetWorldTrimSize(float size) { m_WorldSize = size; }

private:
	// Initialising consts
	const float m_DefaultSeekWeight;
	const float m_DefaultSeparationWeight;
	const float m_DefaultCohesionWeight;
	const float m_DefaultVelMatchWeight;
	const float m_DefaultWanderWeight;
	
	//Datamembers
	int m_FlockSize = 0;
	std::vector<SteeringAgent*> m_Agents;
	std::vector<SteeringAgent*> m_Neighbors;


	bool m_TrimWorld;
	float m_WorldSize;
	bool m_CanDebugRender;
	bool m_CanRenderNeighbourhood;
	bool m_CanRenderPartitioning;
	bool m_IsEvadeAgentEnabled;
	bool m_IsUsingPartitioning;

	float m_NeighborhoodRadius;
	int m_NrOfNeighbors = 0;

	SteeringAgent* m_pAgentToEvade = nullptr;
	
	//Steering Behaviors
	Seek* m_pSeekBehavior = nullptr;
	Separation* m_pSeparationBehavior = nullptr;
	Cohesion* m_pCohesionBehavior = nullptr;
	VelocityMatch* m_pVelMatchBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Evade* m_pEvadeBehavior = nullptr;
	CellSpace* m_pCellSpace;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;


	// Functions 

	float* GetWeight(ISteeringBehavior* pBehaviour);

	Flock(const Flock& other) = delete;
	Flock& operator=(const Flock& other) = delete;

	// Initialising helpers
	inline void InitialiseAgents(int flockSize);
	inline void InitialiseMovementBehaviour();
	inline SteeringAgent* CreateAgent();


	// Update helpers
	inline void UpdateRenderBehaviour(float deltaT, SteeringAgent*& agent);
	inline void UpdateToEvadeAgent(float deltaT);
	inline TargetData UpdateEvadeTargetData();

	// Render helpers
	inline void RenderNeighbourhood(float deltaT);

	// UI helpers
	inline void SetupUI();
	inline void DisplayControlsUI();
	inline void DisplayStatsUI();
	inline void DisplayFlockingUI();
	inline void DisplayToggles();
	inline void DisplayFlockingSliders();
	inline void EndUI();
	void InsertSpacing(int amount = 1);
};