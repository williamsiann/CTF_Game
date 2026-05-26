// Copyright 2024, Irrelevant Pixel, All Rights Reserved.

#include "FPSinViewport.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "FFPSinViewportModule"


void FFPSinViewportModule::StartupModule()
{
	MyHandle = FCoreDelegates::OnEndFrame.AddRaw(this, &FFPSinViewportModule::EditorIsFullyLoaded);
}

void FFPSinViewportModule::EditorIsFullyLoaded() const
{
	if(GEngine && GUnrealEd)
	{
		FCoreDelegates::OnEndFrame.Remove(MyHandle);
		ToggleViewportFPS();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FPS in Viewport: GEngine or GUnrealEd is still do not exist for whatever reason..."));
	}
}
void FFPSinViewportModule::ToggleViewportFPS()
{
	const FString FPS = "FPS";
	GEngine->ExecEngineStat(GUnrealEd->GetWorld(), GUnrealEd->GetWorld()->GetGameViewport(), *FPS);
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFPSinViewportModule, FPSinViewport)
