// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "BattleChairsCharacter.h"
#include "BattleChairsHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"
DEFINE_LOG_CATEGORY(YourLog);

//const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)

ABattleChairsHUD::ABattleChairsHUD(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/Textures/Crosshair"));
	CrosshairTex = CrosshiarTexObj.Object;
	/*Set the textures for the different thruster and health bars*/
	static ConstructorHelpers::FObjectFinder<UTexture2D> HpBar(TEXT("/Game/Textures/RedTexture"));
	HealthBarTexture = HpBar.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterL(TEXT("/Game/Textures/Yellow"));
	ThrusterLTexture = ThrusterL.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterR(TEXT("/Game/Textures/Yellow"));
	ThrusterRTexture = ThrusterR.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterF(TEXT("/Game/Textures/Yellow"));
	ThrusterFTexture = ThrusterF.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> ThrusterTest(TEXT("/Game/Textures/Yellow"));
	thrusterTest = ThrusterTest.Object;
}
/*
int32 health1 = 300;
int32 thrusterl = 0;
int32 thrusterr = 0;
int32 thrusterf = 0;
float getThrusterF = 0;
float getThrusterL = 0;
float getThrusterR = 0;
float frontThruster;
float leftThruster;
float rightThruster;
*/

void ABattleChairsHUD::DrawHUD()
{
	Super::DrawHUD();
	/*
	float ScaleUI = Canvas->ClipY / Canvas->ClipX;
	ABattleChairsCharacter* MyPawn = Cast<ABattleChairsCharacter>(GetOwningPawn());
	if (MyPawn)
	{
		getThrusterF = MyPawn->thrusterF;
		getThrusterL = MyPawn->thrusterL;
		getThrusterR = MyPawn->thrusterR;
		health1 = MyPawn->PlayerHealth; 
		//UE_LOG(YourLog, Warning, TEXT("original front thruster value is %f"), getThrusterF);
	}
	
	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition((Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
		(Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)));

	//intial position of the left thruster bar
	const FVector2D ThrusterLPosition(Center.X * (0.10) , Center.Y * (1.25) );
	//initial position of the right thruster bar
	const FVector2D ThrusterRPosition(Center.X * (1.9), Center.Y * (1.25));
	//initial position of the front thruster bar
	const FVector2D ThrusterFPosition(Center.X * (0.75), 35);

	// draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);


	//new way of drawing the thruster bars
	FCanvasTileItem LeftThrusterBar(ThrusterLPosition, ThrusterLTexture->Resource, FLinearColor::White);
	LeftThrusterBar.BlendMode = SE_BLEND_Translucent;
	// flip the thruster item so it goes up when being used instead of left to right or up to down
	FRotator flipL = FRotator(0);
	flipL.Add(0, 180, 0);
	LeftThrusterBar.Rotation = flipL;
	//scale it
	LeftThrusterBar.Size.X *= .05;
	LeftThrusterBar.Size.Y *= 0;
	// Draw the item
	Canvas->DrawItem(LeftThrusterBar);

	if (GetAsyncKeyState(0x46) != 0 || GetAsyncKeyState(0x56) != 0){
		leftThruster = getThrusterL;
		leftThruster *= 150;
	}




	FCanvasTileItem RightThrusterBar(ThrusterRPosition, ThrusterRTexture->Resource, FLinearColor::White);
	RightThrusterBar.BlendMode = SE_BLEND_Translucent;
	// flip the thruster item so it goes up when being used instead of left to right or up to down
	FRotator flipR = FRotator(0);
	flipR.Add(0, 180, 0);
	RightThrusterBar.Rotation = flipR;
	//scale it
	RightThrusterBar.Size.X *= .05;
	RightThrusterBar.Size.Y *= 0;
	// Draw the item
	Canvas->DrawItem(RightThrusterBar);

	if (GetAsyncKeyState(0x48) != 0 || GetAsyncKeyState(0x4e) != 0){
		rightThruster = getThrusterR;
		rightThruster *= 150;
	}

	FCanvasTileItem FrontThrusterBar(ThrusterFPosition, ThrusterFTexture->Resource, FLinearColor::White);
	FrontThrusterBar.BlendMode = SE_BLEND_Translucent;
	//Rotate the item 270 degrees so it goes left to right on the top of the screen 
	FRotator flipF = FRotator(0);
	flipF.Add(0, 270, 0);
	FrontThrusterBar.Rotation = flipF;
	//scale
	FrontThrusterBar.Size.X *= .05;
	FrontThrusterBar.Size.Y *= 0;
	// Draw the item
	Canvas->DrawItem(FrontThrusterBar);


	if (GetAsyncKeyState(0x54) != 0 || GetAsyncKeyState(0x47) != 0){
		frontThruster = getThrusterF;
		frontThruster *= 150;
		//UE_LOG(YourLog, Warning, TEXT("front thruster after 'T' is %f"), frontThruster);
	}


	//update the left thruster item
	LeftThrusterBar.Size.Y = leftThruster * 1;
	Canvas->DrawItem(LeftThrusterBar);

	//update the right thruster
	RightThrusterBar.Size.Y = rightThruster * 1;
	Canvas->DrawItem(RightThrusterBar);

	//update the front thruster
	FrontThrusterBar.Size.Y = frontThruster * 1;
	Canvas->DrawItem(FrontThrusterBar);

	//call the draw health bar function
	drawHealthBar();
	*/
}

//function to create the health bar
void ABattleChairsHUD::drawHealthBar()
{
	/*
	//Get scale for the UI
	float ScaleUI = Canvas->ClipY / Canvas->ClipX;
	//Make the health bar icon and draw it in the top left of the screen
	float length = health1 * 5;
	if (length <= 0) length = 0;
	FCanvasIcon HealthBarIcon = UCanvas::MakeIcon(HealthBarTexture, 0, 0, length, 20);
	Canvas->DrawIcon(HealthBarIcon, 10, 20, ScaleUI);
	*/
}

