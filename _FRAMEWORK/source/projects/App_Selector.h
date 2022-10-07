#pragma once
// Here we determine which application is currently active
// Create the define here using the "ActiveApp_..." convention and set up the correct include and typedef in the #ifdef below.

//#define ActiveApp_Sandbox
#define ActiveApp_Steering


//---------- Registered Applications -----------

#ifdef ActiveApp_Steering
#include "projects//Movement/SteeringBehaviors/Steering/App_SteeringBehaviors.h"
typedef App_SteeringBehaviors CurrentApp;
#endif

#include "projects/Movement/Sandbox/App_Sandbox.h"
#ifdef ActiveApp_Sandbox
typedef App_Sandbox CurrentApp;
#endif


class App_Selector {
public: 
	static IApp* CreateApp() {
		IApp* myApp = new CurrentApp();
		return myApp;
	}
};