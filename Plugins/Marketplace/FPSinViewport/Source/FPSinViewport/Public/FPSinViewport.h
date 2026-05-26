// Copyright 2024, Irrelevant Pixel, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFPSinViewportModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;

	void EditorIsFullyLoaded() const;
	FDelegateHandle MyHandle;
private:
	static void ToggleViewportFPS();
};
