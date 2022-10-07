//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Sandbox.h"
#include "SandboxAgent.h"

using namespace Elite;

//Destructor
App_Sandbox::~App_Sandbox()
{
	SAFE_DELETE(m_pActor);
}

//Functions
void App_Sandbox::Start()
{
	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	m_pActor = new SandboxAgent();
}

void App_Sandbox::Update(float deltaTime)
{
	CheckInput();
	DisplayUI();
	m_pActor->Update(deltaTime);
}

void App_Sandbox::Render(float deltaTime) const
{
	m_pActor->Render(deltaTime);
}

void App_Sandbox::CheckInput()
{
	if (INPUTMANAGER->IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		MouseData mouseData = INPUTMANAGER->GetMouseData(
			InputType::eMouseButton, 
			InputMouseButton::eLeft);
		m_pActor->SetTarget(DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(
			Vector2((float)mouseData.X, (float)mouseData.Y)));
	}
}

void App_Sandbox::DisplayUI()
{
#ifdef PLATFORM_WINDOWS
	//UI
	{
		//Setup
		int const menuWidth = 200;
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
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#endif
}
