// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "BattleChairsHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

ABattleChairsHUD::ABattleChairsHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/Textures/Crosshair"));
	CrosshairTex = CrosshiarTexObj.Object;

	//Set the Health Bar 
	static ConstructorHelpers::FObjectFinder<UTexture2D> HealthTexObj(TEXT("/Game/StarterContent/Textures/T_Wood_Oak_D"));
	HealthTex = HealthTexObj.Object;
}


void ABattleChairsHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
	const FVector2D TopLeft(0, 0);

	const FVector2D HealthDrawPosition((TopLeft.X), (TopLeft.Y));

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
										   (Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)) );

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );

	// draw the Health
	FCanvasTileItem NewItem(HealthDrawPosition, HealthTex->Resource, FLinearColor::White);
	NewItem.Size *= .1;
	NewItem.Size.Y *= .1;
	NewItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(NewItem);
}

