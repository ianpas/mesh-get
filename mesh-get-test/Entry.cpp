#include "stdafx.h"


int main()
{
	auto& fbx_manager = Glorious::FbxManager::GetSingleton();
	fbx_manager.ImportMesh(R"(K:\Test\FBX-Test\plane.fbx)");
	auto mesh = fbx_manager.GetMesh();

	return 0;

}