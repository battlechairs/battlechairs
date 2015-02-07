// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "BattleChairsPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLECHAIRS_API ABattleChairsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void UpdateRotation(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Pawn")
		FRotator GetViewRotation() const;

	UFUNCTION(BlueprintCallable, Category = "Pawn")
		virtual void SetViewRotation(const FRotator& NewRotation);

	virtual void SetControlRotation(const FRotator& NewRotation) override;

protected:

	/**
	*  View & Movement direction are now separate.
	*  The controller rotation will determine which direction we will move.
	*  ViewRotation represents where we are looking.
	*/
	UPROPERTY()
		FRotator ViewRotation;

	FRotator previousRotation = FRotator();
};
