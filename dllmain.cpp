#include "includes.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <sstream>
#include <math.h>


// data
void* d3d9Device[119];
BYTE EndSceneBytes[7]{ 0 };
tEndScene oEndScene = nullptr;
extern LPDIRECT3DDEVICE9 pDevice = nullptr;
Hack* hack;

// hook function
void APIENTRY hkEndScene(LPDIRECT3DDEVICE9 o_pDevice) {
	if (!pDevice)
		pDevice = o_pDevice;

	// drawing stuff
	DrawText("First ESP!", windowWidth / 2, windowHeight - 20, D3DCOLOR_ARGB(255, 255, 255, 255));
	std::vector<Vec3> targets;
	for (int i = 1; i < 32; i++)
	{
		uintptr_t curEnt = *(uintptr_t*)(0x18E73C0 + (i * 0x88)); // loops the entity list
		if (!curEnt) continue;
		uintptr_t isZombie = *(uintptr_t*)(curEnt + 0x4);
		if (isZombie < 2) continue;
		int healthL = *(int*)(curEnt + 0x1C8);
		float health = *(float*)(curEnt + 0x1C8);
		float round_health = *(float*)(curEnt + 0x1CC);// specfies health and adds the offset
		if (isnan(health)) continue;

	

		targets.push_back(*(Vec3*)(curEnt + 0x154));

		D3DCOLOR color;
		color = D3DCOLOR_ARGB(255, 255, 0, 0); // line color
		
		D3DCOLOR colorL;
		colorL = D3DCOLOR_ARGB(255, 255, 0, 0);// box color

		Vec2 entPos2D, entHead2D;


		Vec3 feet_pos = *(Vec3*)(curEnt + 0x18);
		Vec3 head_pos = *(Vec3*)(curEnt + 0x154);
		
		head_pos.x -= 15;
		head_pos.y -= 15;

		if (hack->WorldToScreen(feet_pos, entPos2D)) {
			if (hack->settings.snaplines) {
				DrawLine(entPos2D.x, entPos2D.y, windowWidth / 2, 1000, 1, color); // draws the line using world to screen
			}
		}
			if (hack->WorldToScreen(head_pos, entHead2D)) {
				if (hack->settings.box2D) {
					DrawEspBox2D(entPos2D, entHead2D, 2, colorL);
					
					int height = ABS(entPos2D.y - entHead2D.y);
					int dX = (entPos2D.x - entHead2D.x);

					float healthPerc = health / round_health;  // this has to be an int

					Vec2 botHealth, topHealth;

					float healthHeight = height * healthPerc;

					botHealth.y = entPos2D.y;

					botHealth.x = entPos2D.x - (height / 4) - 8;

					topHealth.y = entHead2D.y + height - healthHeight;

					topHealth.x = entPos2D.x - (height / 4) - 8 - (dX * healthPerc);

					DrawLine(botHealth, topHealth, 1, D3DCOLOR_ARGB(255, 46, 139, 87));
				}

			}

			if (hack->settings.statusText) {
				std::stringstream s1, s2;
				s1 << healthL;
				std::string t1 = "HP: " + s1.str();
				char* healthMsg = (char*)t1.c_str();
				DrawText(healthMsg, entPos2D.x, entPos2D.y, D3DCOLOR_ARGB(255, 255, 255, 255));
			}
	}

	// crosshair
	DrawFilledRect(windowWidth / 2 - 2, windowHeight / 2 - 2, 4, 4, D3DCOLOR_ARGB(255, 255, 255, 255)); 


	// call og function
	oEndScene(pDevice);
}

DWORD WINAPI HackThread(HMODULE hModule) {

	// hook
	if (GetD3D9Device(d3d9Device, sizeof(d3d9Device))) {
		memcpy(EndSceneBytes, (char*)d3d9Device[42], 7);

		oEndScene = (tEndScene)TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
	}

	hack = new Hack();
	hack->Init();

	// hack loop
	while (!GetAsyncKeyState(VK_END)) {
		hack->Update();
	}

	// unhook
	Patch((BYTE*)d3d9Device[42], EndSceneBytes, 7);

	// uninject
	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpr) {
	if (reason == DLL_PROCESS_ATTACH)
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, 0));
	return TRUE;
}