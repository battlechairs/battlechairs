// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "BattleChairsCharacter.generated.h"

UCLASS(config=Game)
class ABattleChairsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
public:
	//Public Variables
	bool rightFire;
	bool leftFire;
	int rightFireDelay;
	int leftFireDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attribute)
		float PlayerHealth;

	float firerate;
	float knockback;
	float turnrate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterF)
	float thrusterF;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterL)
	float thrusterL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterR)
	float thrusterR;
	float lift;

	float rotationalVelocity;
	float rotationalDrag;

	//Mitch: these variables are for hardware communication
	HANDLE hSerial;
	COMSTAT status;
	DWORD32 errors;
	bool connected = false;
	char controlBuffer[100];
	unsigned int controlBufferPos = 0;

	ABattleChairsCharacter(const FObjectInitializer& ObjectInitializer);

	//Mitch: destructor disconnects from hardware
	~ABattleChairsCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ABattleChairsProjectile> ProjectileClass;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AProjectileParent> BulletClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

protected:
	/** Fires a projectile (left click) with server validation*/
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttemptLeftFire();
	void LeftFire();

	/** Stops left fire with server validation */
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttemptStopLeftFire();
	void StopLeftFire();

	/** Fires a projectile (right click) with dedicated server validation.*/
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttemptRightFire();
	void RightFire();

	/** Stops right fire with server validation */
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_AttemptStopRightFire();
	void StopRightFire();

	
	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	void ThrusterFUp();
	void ThrusterFDown();
	void ThrusterLUp();
	void ThrusterLDown();
	void ThrusterRUp();
	void ThrusterRDown();
	bool ThrusterFON();
	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	float min(float a, float b, float c);

	void ABattleChairsCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};

