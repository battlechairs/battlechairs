// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "MyProject2.h"
#include "MyProject2GameMode.h"
#include "MyProject2HUD.h"
#include "MyProject2Character.h"

AMyProject2GameMode::AMyProject2GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/MyCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMyProject2HUD::StaticClass();
}
