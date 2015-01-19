// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "BattleChairsCharacter.h"
#include "BattleChairsHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

//const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)

ABattleChairsHUD::ABattleChairsHUD(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/Textures/Crosshair"));
	CrosshairTex = CrosshiarTexObj.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> HpBar(TEXT("/Game/Textures/RedTexture"));
	HealthBarTexture = HpBar.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterL(TEXT("/Game/Textures/Yellow"));
	ThrusterLTexture = ThrusterL.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterR(TEXT("/Game/Textures/Yellow"));
	ThrusterRTexture = ThrusterR.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterF(TEXT("/Game/Textures/Yellow"));
	ThrusterFTexture = ThrusterF.Object;
}

int32 health = 200;
int32 thrusterl = 0;
int32 thrusterr = 0;
int32 thrusterf = 0;



void ABattleChairsHUD::DrawHUD()
{
	Super::DrawHUD();
	float ScaleUI = Canvas->ClipY / Canvas->ClipX;

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition((Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
		(Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)));

	// draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
	//github is dumb
	/* Left thruster bar that appears when you use the left thruster*/
	FCanvasIcon ThrusterLIcon = UCanvas::MakeIcon(ThrusterLTexture, 0, 0, 20, thrusterl);
	if (thrusterl != 0)
		Canvas->DrawIcon(ThrusterLIcon, 10, 250, ScaleUI);


	if (GetAsyncKeyState(VK_RIGHT) != 0 && health < 196)
		health += 5;

	if (GetAsyncKeyState(0x25) != 0 && health > 5)
		health -= 5;

	if (GetAsyncKeyState(0x46) != 0){
		thrusterl += 5;
		if (thrusterl < 5 && thrusterl > 0)
			thrusterl = 0;
	}

	if (GetAsyncKeyState(0x56) != 0 && thrusterl > 4){
		thrusterl -= 5;
		if (thrusterl < 0)
			thrusterl = 0;
		if (thrusterl < 5 && thrusterl > 0)
			thrusterl = 0;
	}

	if (thrusterl < 5 && thrusterl > 0)
		thrusterl = 0;
	/* End of left thruster code*/

	/* Right thruster bar that appears when you use the left thruster*/
	FCanvasIcon ThrusterRIcon = UCanvas::MakeIcon(ThrusterRTexture, 0, 0, 20, thrusterr);
	if (thrusterr != 0)
		Canvas->DrawIcon(ThrusterRIcon, 775, 250, ScaleUI);

	if (GetAsyncKeyState(0x48) != 0){
		thrusterr += 5;
		if (thrusterr < 5 && thrusterr > 0)
			thrusterr = 0;
	}

	if (GetAsyncKeyState(0x4e) != 0 && thrusterr > 4){
		thrusterr -= 5;
		if (thrusterr < 0)
			thrusterr = 0;
		if (thrusterr < 5 && thrusterr > 0)
			thrusterr = 0;
	}

	if (thrusterr < 5 && thrusterr > 0)
		thrusterr = 0;
	/* End of right thruster code*/

	/* Right thruster bar that appears when you use the left thruster*/
	FCanvasIcon ThrusterFIcon = UCanvas::MakeIcon(ThrusterFTexture, 0, 0, thrusterf, 20);
	if (thrusterf != 0)
		Canvas->DrawIcon(ThrusterFIcon, Canvas->SizeX / 2.5, 10, ScaleUI);

	if (GetAsyncKeyState(0x54) != 0){
		thrusterf += 5;
		if (thrusterf < 5 && thrusterf > 0)
			thrusterf = 0;
	}

	if (GetAsyncKeyState(0x47) != 0 && thrusterf > 4){
		thrusterf -= 5;
		if (thrusterf < 0)
			thrusterf = 0;
		if (thrusterf < 5 && thrusterf > 0)
			thrusterf = 0;
	}
	if (thrusterf < 5 && thrusterf > 0)
		thrusterf = 0;
	/* End of left thruster code*/
	drawHealthBar();
}

void ABattleChairsHUD::drawHealthBar()
{
	float ScaleUI = Canvas->ClipY / Canvas->ClipX;




	FCanvasIcon HealthBarIcon = UCanvas::MakeIcon(HealthBarTexture, 0, 0, health, 20);
	Canvas->DrawIcon(HealthBarIcon, 10, 20, ScaleUI);


}

