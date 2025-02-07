// GTAVMousehookHelperXenia.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

__declspec(dllexport) float mouse_x = 0;
__declspec(dllexport) float mouse_y = 0;


__declspec(dllexport)  bool mousehook_ShouldPullAroundWhenUsingMouse = false;

__declspec(dllexport) int vehicle_type_player;

#define DTOR 0.017453292
//#define RTOD = 57.2958


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

/*
typedef int (*Cameratesthook)(int *a1,float *a2, float *a3);
Detour CameratesthookDetour;

typedef void (*uhh_cameraT)(double a1,double a2, int a3, int a4, int a5);
Detour uhh_camera_detour;
*/
typedef void (*MAYBEComputeOrbitOrientationT)(int camerapointer, float *orbitHeading, float *orbitPitch);
Detour MAYBEComputeOrbitOrientation_detour;


typedef int (*write_deltaT)(int a1, int a2);
Detour write_delta_detour;
//t

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
/*
int camera_test(int* a1, float* a2, float* a3)
{

    auto OriginalFunction = CameratesthookDetour.GetOriginal<Cameratesthook>();
    if (OriginalFunction)
        return OriginalFunction(a1, a2, a3);
	
    return 0; 
}

void uhh_camera(double x_axis,double y_axis, int a3, int a4, int a5) {
	x_axis  += X_delta();
	y_axis  += Y_delta();
	y_axis = clamp(y_axis,-90 * DTOR,90  * DTOR);
	auto OriginalFunction = uhh_camera_detour.GetOriginal<uhh_cameraT>();
	if(OriginalFunction)
		return OriginalFunction(x_axis,y_axis,a3,a4,a5);

}
*/
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

void VehicleChecks() {
		while(true){
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
		Sleep(90);
	}
		

}

void startvehiclecheck() {
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
void InstallHook()
{

	MAYBEComputeOrbitOrientation_detour = Detour((void*)0x82361090, (const void*)ComputeOrbitOrientation_Hook);
	MAYBEComputeOrbitOrientation_detour.Install();

	write_delta_detour = Detour((void*)0x82362650, (const void*)write_delta_hooked);
	write_delta_detour.Install();


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
		//DbgPrint("XamGetSystemVersion() returns: 0x%X",XamGetSystemVersion());
		InstallHook();
		print();
		startvehiclecheck();
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    }
    return TRUE;
}
