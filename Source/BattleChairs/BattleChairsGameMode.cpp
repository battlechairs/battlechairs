// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "BattleChairsGameMode.h"
#include "BattleChairsHUD.h"
#include "BattleChairsCharacter.h"

ABattleChairsGameMode::ABattleChairsGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/MyCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ABattleChairsHUD::StaticClass();
}
