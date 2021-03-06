// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "BattleChairsCharacter.generated.h"

UCLASS(config=Game)
class ABattleChairsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	//UPROPERTY(VisibleAnywhere, Category=Mesh)
	//class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
public:
	//Public Variables
	bool rightFire;
	bool leftFire;
	float rightFireDelay;
	float leftFireDelay;
	UFUNCTION(BlueprintCallable, Category = "Functions")
		FString GetCurrentMapName();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attribute)
		float PlayerHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attribute)
		int32 uniqueID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attribute)
		FVector speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float firerate;
	float knockback;
	float turnrate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterF)
	float thrusterF;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterL)
	float thrusterL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterR)
	float thrusterR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterF)
	FVector thrusterFV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterL)
	FVector thrusterLV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterR)
	FVector thrusterRV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = thrusterF)
	float SpawnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	AActor* WeaponType;
	float lift;
	float ThrusterIncrement;
	float ThrusterMaximum;

	float rotationalVelocity;
	float rotationalVelocityPositive;
	float rotationalVelocityNegative;
	float rotationalVelocityMaximum;
	float rotationalVelocityIncrement;
	float rotationalDrag;
	FVector cameraStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float rotationalVelocityMultiplier = 2.f;

	FRotator chairDirection;

	//Mitch: these variables are for hardware communication
	HANDLE hSerial;
	COMSTAT status;
	DWORD32 errors;
	bool connected = false;
	char controlBuffer[100];
	unsigned int controlBufferPos = 0;
	WCHAR portNameBuffer[16];

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
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
	void LeftFire(int32 ID, FVector Speed);

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

	/** Makes the character can fly */
	//UFUNCTION(NetMulticast, Reliable, WithValidation)
		//void Server_AttemptLift();
	//void LiftPlayer();

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

	void AddControllerPitchInput(float val);
	void AddControllerYawInput(float val);

	float min(float a, float b, float c);

	void ABattleChairsCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	//Mitch: protected function to perform hardware commands
	void processHardwareEvent();

	//Mitch: protected function to perform verification on a COM port (uses hSerial)
	bool verifyCurrentPort();

public:
	/** Returns Mesh1P subobject **/
	//FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UFUNCTION(BlueprintImplementableEvent, Category = "DmgSystem")
		void setplayer(AActor* bullet, int32 ID, FVector vel);

	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	void UpdateOculusCamera(const FRotator& viewRotation, const FVector& viewPosition);
};

