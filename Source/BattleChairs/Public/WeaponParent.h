// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "WeaponParent.generated.h"

/**
 * 
 */
UCLASS(config = Game)
class AWeaponParent : public AActor
{
	GENERATED_BODY()
		
	public:
		AWeaponParent(const FObjectInitializer& ObjectInitializer);

		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay);
		float recoil;

	protected:
		float getRecoil();
		void setRecoil(float newRecoil);
		
		
		//AMyProjectileParent bullet;
	
	
};
