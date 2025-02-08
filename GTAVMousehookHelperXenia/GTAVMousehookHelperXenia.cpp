// GTAVMousehookHelperXenia.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

__declspec(dllexport) float mouse_x = 0;
__declspec(dllexport) float mouse_y = 0;


__declspec(dllexport)  bool mousehook_ShouldPullAroundWhenUsingMouse = false;

__declspec(dllexport) int vehicle_type_player;

#define DTOR 0.017453292

#define RTOD 57.2958


template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
// Returns a CPed?
typedef int (__fastcall* FindPlayerT)();
FindPlayerT FindPlayer = (FindPlayerT)(0x82618130);

// Returns a CVehicle
typedef int (__fastcall* FindPlayerVehicleT)(int player);
FindPlayerVehicleT FindPlayerVehicle = (FindPlayerVehicleT)(0x82638CB0);

typedef bool (*IsGamePausedT)();
IsGamePausedT IsGamePaused = (IsGamePausedT)(0x822F55D0);


struct GameVersionAddresses {
    DWORD check_addr;
    const char* valutocomapreaddresds;
    DWORD FindPlayerVehicle_addr;
	DWORD IsGamePaused_addr;
	DWORD MAYBEComputeOrbitOrientation_addr;
	DWORD write_delta_addr;
	DWORD UpdateMapScreenInput_addr;
	DWORD vNewPos_x_addr;
};

GameVersionAddresses TU26 = {
    0x8201CD74, // check_addr
    "GTAV Xbox360 Final %s", // valutocomapreaddresds
    0x82638CB0, // FindPlayerVehicle_addr
	0x822F55D0, // IsGamePaused_addr
	0x82361090, // MAYBEComputeOrbitOrientation_addr
	0x82362650, // write_delta_addr
	NULL,
	NULL,
};

GameVersionAddresses TU27 = {
    0x8201CD48, // check_addr
    "GTAV Xbox360 Final %s", // valutocomapreaddresds
    0x82638B40, // FindPlayerVehicle_addr
	0x822F5400, // IsGamePaused_addr
	0x82360EC0, // MAYBEComputeOrbitOrientation_addr
	0x82362480, // write_delta_addr
	0x822EA818, // UpdateMapScreenInput_addr
	0x839F7270, // vNewPos_x_addr
};

const int MAX_VERSIONS = 10; 
GameVersionAddresses gameVersions[MAX_VERSIONS] = {
    TU26,
	TU27,

};

int numVersions = 2;
GameVersionAddresses* DetermineGameVersion() {
    for (int i = 0; i < numVersions; ++i) {
        const char* memoryValue = reinterpret_cast<const char*>(gameVersions[i].check_addr);
        if (strcmp(memoryValue, gameVersions[i].valutocomapreaddresds) == 0) {
            return &gameVersions[i];
        }
    }
    return nullptr;
}


typedef void (*MAYBEComputeOrbitOrientationT)(int camerapointer, float *orbitHeading, float *orbitPitch);
Detour MAYBEComputeOrbitOrientation_detour;


typedef int (*write_deltaT)(int a1, int a2);
Detour write_delta_detour;

typedef int (*UpdateMapScreenInputT)(char bInitialEntry);
Detour UpdateMapScreenInput_detour;


float Y_delta() {
	return -mouse_y;
}

float X_delta() {
	return -mouse_x;
}
#if LTCG || _DEBUG
static bool run = true;
static float timer = 0.f;
#endif


void print() {
	DbgPrint("mouse_x addr: 0x%X \n mouse_y addr: 0x%X \n",&mouse_x,&mouse_y);
	DbgPrint("mousehook_ShouldPullAroundWhenUsingMouse addr: 0x%X \n",&mousehook_ShouldPullAroundWhenUsingMouse);
	DbgPrint("vehicle_type_player addr: 0x%X \n",&vehicle_type_player);

	#if LTCG || _DEBUG
	DbgPrint("Timer addr: 0x%X \n",&timer);
	#endif

}

DWORD __fastcall getvehicletype(int vehicle)
{
	return *(DWORD*)(vehicle + 0x664);
}

int UpdateMapScreenInput_Hook(char bInitialEntry1){
	//auto* version = DetermineGameVersion();
	auto OriginalFunction = UpdateMapScreenInput_detour.GetOriginal<UpdateMapScreenInputT>();
	/*
	float *x = (float*)(version->vNewPos_x_addr);
	float *y = (float*)(version->vNewPos_x_addr + 0x4);
	*x = *x + (X_delta() * RTOD);
	*y = *y + (X_delta() * RTOD);*/
	return OriginalFunction(bInitialEntry1);
}

void ComputeOrbitOrientation_Hook(int camerapointer, float *orbitHeading, float *orbitPitch) {
	auto OriginalFunction = MAYBEComputeOrbitOrientation_detour.GetOriginal<MAYBEComputeOrbitOrientationT>();
	int v17 = *(DWORD*)(camerapointer + 0x340);
	float *lookAroundInputEnvelopeLevel = (float*)(v17 + 0x64);
	BYTE* bDisablePullAroundCameraForWeapon = (BYTE*)(camerapointer + 0x3D5);
	//DbgPrint("Campointer: 0x%X, lookAroundInputEnvelopeLevel: %f \n",camerapointer,*lookAroundInputEnvelopeLevel);
	*bDisablePullAroundCameraForWeapon = mousehook_ShouldPullAroundWhenUsingMouse;
	//DbgPrint("bDisablePullAroundCameraForWeapon: 0x%X \n",*bDisablePullAroundCameraForWeapon);
	if (mousehook_ShouldPullAroundWhenUsingMouse)
	*lookAroundInputEnvelopeLevel = 1.f;
	OriginalFunction(camerapointer,orbitHeading,orbitPitch);
	//DbgPrint("Updated: Campointer: 0x%X, lookAroundInputEnvelopeLevel: %f \n",camerapointer,*lookAroundInputEnvelopeLevel);



	return; 
}

static signed int time_until_uncenter = 0;

#define THREAD_SLEEP 90

void centering(){
    #define timer_to_reset 2430
    if(X_delta() != 0.f || Y_delta() != 0.f) {
        mousehook_ShouldPullAroundWhenUsingMouse = 1;
        time_until_uncenter = timer_to_reset;
		DbgPrint("HITTING");
    }
    
	else if(mousehook_ShouldPullAroundWhenUsingMouse == 1 && time_until_uncenter > 0) {
		time_until_uncenter = std::max<int>(0, time_until_uncenter - THREAD_SLEEP);
    }

    if(time_until_uncenter <= 0)
        mousehook_ShouldPullAroundWhenUsingMouse = 0;

DbgPrint("centering() called, X_delta: %f, Y_delta: %f, time_until_uncenter: %d\n",
         X_delta(), Y_delta(), time_until_uncenter);

}


void VehicleChecks() {
		while(true){
		//centering();
		int temp = 420;
		if(IsGamePaused())
			temp = 421;
		int vehicle = FindPlayerVehicle(0);
		//DbgPrint("Vehicle: 0x%X \n",vehicle);
		if(vehicle && !IsGamePaused()){
		temp = getvehicletype(vehicle);
		//DbgPrint("Vehicle type: 0x%X \n",temp);

		}
		vehicle_type_player = temp;
		Sleep(THREAD_SLEEP);
	}
		

}

void startloop() {
	HANDLE hThread; 
	DWORD threadId;
	ExCreateThread(&hThread, 0, &threadId, nullptr, (LPTHREAD_START_ROUTINE)VehicleChecks, NULL, 0x2);
	XSetThreadProcessor(hThread, 4);
	//SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(hThread);
}

int write_delta_hooked(int a1, int a2) {
			auto OriginalFunction = write_delta_detour.GetOriginal<write_deltaT>();
			int result = OriginalFunction(a1,a2);
			float *x_delta = (float*)(a1 + 0x5C);
			float *y_delta = (float*)(a1 + 0x60);
			if(X_delta() || Y_delta())
			{
			*x_delta = X_delta();
			*y_delta = Y_delta();
			}
			return result;
}

void SetFunctionCallsAddrs(GameVersionAddresses *addr) {
	FindPlayerVehicle = (FindPlayerVehicleT)addr->FindPlayerVehicle_addr;
	IsGamePaused = (IsGamePausedT)addr->IsGamePaused_addr;
}

bool InstallHook()
{
	auto* version = DetermineGameVersion();
	if(!version)
		return false;
	MAYBEComputeOrbitOrientation_detour = Detour((void*)version->MAYBEComputeOrbitOrientation_addr, (const void*)ComputeOrbitOrientation_Hook);
	MAYBEComputeOrbitOrientation_detour.Install();

	write_delta_detour = Detour((void*)version->write_delta_addr, (const void*)write_delta_hooked);
	write_delta_detour.Install();

	if(version->UpdateMapScreenInput_addr){
	//UpdateMapScreenInput_detour = Detour((void*)version->UpdateMapScreenInput_addr, (const void*)UpdateMapScreenInput_Hook);
	//UpdateMapScreenInput_detour.Install();
	
	}


	SetFunctionCallsAddrs(version);

	return true;
}

void loveXBDM(){

	char *buttons;
	SHORT x;
	SHORT y;
	SHORT wheel;
DmGetMouseChanges(nullptr,&x,&y,&wheel);
DbgPrint("buttons: %d, %d,%d, %d",buttons,x,y,wheel);
DbgPrint("mate x addr: 0x%X",&x);
}

#if LTCG || _DEBUG
void PullAroundTimer(){
	while(run){
	//loveXBDM();
	DbgPrint("test");
	float deltaTime = (*(float*)0x83DDD878);

	if(X_delta()||Y_delta()) {
		mousehook_ShouldPullAroundWhenUsingMouse = true;
		timer = 1.25f;
	}
	if(timer > 0.f || mousehook_ShouldPullAroundWhenUsingMouse) {
		timer -=deltaTime;
		if(timer <= 0.f)
			mousehook_ShouldPullAroundWhenUsingMouse = false;
	}
	Sleep(deltaTime * 1000);
	}
}

void start() {
	HANDLE hThread; 
	DWORD threadId;
	ExCreateThread(&hThread, 0, &threadId, nullptr, (LPTHREAD_START_ROUTINE)PullAroundTimer, NULL, 0x2);
	XSetThreadProcessor(hThread, 4);
	SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(hThread);
}
#endif
BOOL WINAPI DllMain(HANDLE hInstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
#if LTCG || _DEBUG
		start();
#endif
		loveXBDM();
		InstallHook();
		print();
		startloop();
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    }
    return TRUE;
}
