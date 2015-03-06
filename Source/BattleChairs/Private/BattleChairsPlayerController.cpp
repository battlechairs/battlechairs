// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleChairs.h"
#include "BattleChairsPlayerController.h"
#include "BattleChairsCharacter.h"

#include "InputCoreTypes.h"

// Make sure you include this!!
#include "IHeadMountedDisplay.h"

void ABattleChairsPlayerController::UpdateRotation(float DeltaTime)
{
	// Calculate Delta to be applied on ViewRotation
	FRotator DeltaRot(RotationInput);

	FRotator NewControlRotation = GetControlRotation();

	if (PlayerCameraManager)
	{
		//PlayerCameraManager->ProcessViewRotation(DeltaTime, NewControlRotation, DeltaRot);
	}

	//SetControlRotation(NewControlRotation);

	if (!PlayerCameraManager || !PlayerCameraManager->bFollowHmdOrientation)
	{
		if (GEngine->HMDDevice.IsValid() && GEngine->HMDDevice->IsHeadTrackingAllowed())
		{
			FQuat HMDOrientation;
			FVector HMDPosition;

			// Disable bUpdateOnRT if using this method.
			GEngine->HMDDevice->GetCurrentOrientationAndPosition(HMDOrientation, HMDPosition);

			FRotator NewViewRotation = HMDOrientation.Rotator();

			// Only keep the yaw component from the controller.
			//NewViewRotation.Yaw += NewControlRotation.Yaw;

			//SetViewRotation(NewViewRotation);

			ABattleChairsCharacter* MYC = Cast<ABattleChairsCharacter>(GetPawnOrSpectator());
			if (MYC)
			{
				//MYC->UpdateOculusCamera(ViewRotation, HMDPosition);
			}

		}
	}
	
}

void ABattleChairsPlayerController::SetControlRotation(const FRotator& NewRotation)
{
	ControlRotation = NewRotation;

	// Anything that is overriding view rotation will need to 
	// call SetViewRotation() after SetControlRotation().
	//SetViewRotation(NewRotation);

	if (RootComponent && RootComponent->bAbsoluteRotation)
	{
		//RootComponent->SetWorldRotation(GetControlRotation());
	}
}

void ABattleChairsPlayerController::SetViewRotation(const FRotator& NewRotation)
{
	ViewRotation = NewRotation;
}

FRotator ABattleChairsPlayerController::GetViewRotation() const
{
	return ViewRotation;
}
