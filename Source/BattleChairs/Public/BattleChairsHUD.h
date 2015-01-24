// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "BattleChairsHUD.generated.h"
DECLARE_LOG_CATEGORY_EXTERN(YourLog, Log, All);

UCLASS()
class ABattleChairsHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABattleChairsHUD(const FObjectInitializer& ObjectInitializer);

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
	void drawHealthBar();
	UTexture2D* HealthBarTexture;
	UTexture2D* ThrusterLTexture;
	UTexture2D* ThrusterRTexture;
	UTexture2D* ThrusterFTexture;
	UTexture2D* thrusterTest;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

