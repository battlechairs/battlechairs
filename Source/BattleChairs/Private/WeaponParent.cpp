// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleChairs.h"
#include "WeaponParent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AWeaponParent::AWeaponParent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
{
	float recoil = 0;
}

float getRecoil() {
	//return recoil;
	return 0.0f;
}

void setRecoil(float newRecoil) {
	//recoil = newRecoil;
}