// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "UnrealNetwork.h"
#include "BattleChairsCharacter.h"
#include "BattleChairsProjectile.h"
#include "ProjectileParent.h"
#include "Animation/AnimInstance.h"



//////////////////////////////////////////////////////////////////////////
// ABattleChairsCharacter




ABattleChairsCharacter::ABattleChairsCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	leftFire = false;
	rightFire = false;
	leftFireDelay = firerate;
	rightFireDelay = firerate;
	thrusterF = 0;
	thrusterL = 0;
	thrusterR = 0;
	lift = 0;
	firerate = 5;
	knockback = -100;
	turnrate = 5;

	// Create a CameraComponent	
	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(0.0f, 0.0f, 0.0f);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(false);			// only the owning player will see this mesh
	Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -150.f);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//Mitch: destructor disconnects hardware
ABattleChairsCharacter::~ABattleChairsCharacter() {
	if (connected) {
		CloseHandle(hSerial);
		UE_LOG(LogTemp, Warning, TEXT("disconnected from Arduino hardware"));
	}
}

//////////////////////////////////////////////////////////////////////////
// Bind Input

void ABattleChairsCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	//Thruster Controls
	InputComponent->BindAction("ThrusterFUpButton", IE_Pressed, this, &ABattleChairsCharacter::ThrusterFUp);
	InputComponent->BindAction("ThrusterFDownButton", IE_Pressed, this, &ABattleChairsCharacter::ThrusterFDown);
	InputComponent->BindAction("ThrusterLUpButton", IE_Pressed, this, &ABattleChairsCharacter::ThrusterLUp);
	InputComponent->BindAction("ThrusterLDownButton", IE_Pressed, this, &ABattleChairsCharacter::ThrusterLDown);
	InputComponent->BindAction("ThrusterRUpButton", IE_Pressed, this, &ABattleChairsCharacter::ThrusterRUp);
	InputComponent->BindAction("ThrusterRDownButton", IE_Pressed, this, &ABattleChairsCharacter::ThrusterRDown);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	InputComponent->BindAction("Fire", IE_Pressed, this, &ABattleChairsCharacter::Server_AttemptLeftFire);
	InputComponent->BindAction("SecondaryFire", IE_Pressed, this, &ABattleChairsCharacter::Server_AttemptRightFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ABattleChairsCharacter::Server_AttemptStopLeftFire);
	InputComponent->BindAction("SecondaryFire", IE_Released, this, &ABattleChairsCharacter::Server_AttemptStopRightFire);

	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ABattleChairsCharacter::TouchStarted);

	InputComponent->BindAxis("MoveForward", this, &ABattleChairsCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABattleChairsCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ABattleChairsCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ABattleChairsCharacter::LookUpAtRate);

	//Mitch: ---START OF HARDWARE BLOCK--

	//Mitch: if hardware already connected (not sure how), disconnect
	if (connected) {
		CloseHandle(hSerial);
		UE_LOG(LogTemp, Warning, TEXT("disconnected from Arduino hardware"));
	}

	//Mitch: if not connected to hardware (shouldn't be anyway), connect
	if (!connected) {

		//Mitch: connect to memory-mapped file, I think (might not always be COM6)
		LPCWSTR portName = L"\\\\.\\COM6";
		hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE,
			0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		//Mitch: check if CreateFile worked
		if (hSerial == INVALID_HANDLE_VALUE) {
			UE_LOG(LogTemp, Warning, TEXT("unable to connect to Arduino hardware"));
		}
		else {
			connected = true;
			DCB dcbSerialParams = { 0 };

			if (!GetCommState(hSerial, &dcbSerialParams)) {
				UE_LOG(LogTemp, Warning, TEXT("unable to get Arduino serial port"));
			}
			else {
				//Mitch: ensure these settings match settings in device mananger
				dcbSerialParams.BaudRate = CBR_9600;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;

				if (!SetCommState(hSerial, &dcbSerialParams)) {
					UE_LOG(LogTemp, Warning, TEXT("unable to set Arduino serial port parameters"));
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("successfully connected to Arduino hardware"));
				}
			}
		}
	}

	//Mitch: ---END OF HARDWARE BLOCK--
}

//////////////////////////////////////////////////////////////////////////
// Left Fire

// Don't worry if there are red squigly lines on the functions below, it will still compile

/* Validation */
bool ABattleChairsCharacter::Server_AttemptLeftFire_Validate()
{
	return true; // We can insert code here to test if they are allowed to fire, for example if they have ammo or not
}
/* Attempt to fire projectile */
void ABattleChairsCharacter::Server_AttemptLeftFire_Implementation()
{
	if (Role == ROLE_Authority)
	{
		LeftFire();
	}
}
/* Fire projectile */
void ABattleChairsCharacter::LeftFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		FVector offSet = FVector(0.0f, -150.0f, 0.0f);
		FRotator turn = FRotator(0.0);
		if (rightFire == false){
			turn.Add(0.0f, turnrate, 0.0f);
		}
		else {
			LaunchPawn(knockback * GetActorForwardVector(), false, false);
		}
		//const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset) + SpawnRotation.RotateVector(offSet);
		FVector testGunOffset = FVector(150.0f, 75.0f, 35.0f);
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(testGunOffset) + SpawnRotation.RotateVector(offSet);
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<ABattleChairsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			//World->SpawnActor<AProjectileParent>(BulletClass, SpawnLocation, SpawnRotation);
		}

		ClientSetRotation(SpawnRotation - turn);
		//LaunchPawn(-1000 * GetActorForwardVector(), false, false);
		leftFire = true;
	}
	/*
	// try and play the sound if specified
	if (FireSound != NULL)
	{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	*/
	/*
	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
	// Get the animation object for the arms mesh
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL)
	{
	AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	}
	*/
}

/* Validation */
bool ABattleChairsCharacter::Server_AttemptStopLeftFire_Validate()
{
	return true; //dont know why we need the validation to stop firing, but it doesnt work without it
}

/* Attempt to stop firing */
void ABattleChairsCharacter::Server_AttemptStopLeftFire_Implementation()
{
	if (Role == ROLE_Authority)
	{
		StopLeftFire();
	}
}

/* Stop firing */
void ABattleChairsCharacter::StopLeftFire()
{
	leftFire = false;
	leftFireDelay = firerate;
}

//////////////////////////////////////////////////////////////////////////
// Right Fire

// dont worry if there are red squigly lines on the functions below, it will still compile

/* Validation */
bool ABattleChairsCharacter::Server_AttemptRightFire_Validate()
{
	return true; //We can insert code here to test if they are allowed to fire, for example if they have ammo or not
}

/* Attempt to fire projectile */
void ABattleChairsCharacter::Server_AttemptRightFire_Implementation()
{
	if (Role == ROLE_Authority)
	{
		RightFire();
	}
}

/* Fire projectile */
void ABattleChairsCharacter::RightFire()
{

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		//FVector offSet = FVector(0.0f, -60.0f, 0.0f);
		FRotator turn = FRotator(0.0);
		if (leftFire == false){
			turn.Add(0.0f, -turnrate, 0.0f);
		}
		else {
			LaunchPawn(knockback * GetActorForwardVector(), false, false);
		}
		//const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);
		FVector testGunOffset = FVector(150.0f, 75.0f, 35.0f);
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(testGunOffset);
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<ABattleChairsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}

		ClientSetRotation(SpawnRotation - turn);
		//LaunchPawn(-1000 * GetActorForwardVector(), false, false);
		rightFire = true;
	}
	/*
	// try and play the sound if specified
	if (FireSound != NULL)
	{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	*/
	/*
	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
	// Get the animation object for the arms mesh
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL)
	{
	AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	}
	*/
}
/* Validation */
bool ABattleChairsCharacter::Server_AttemptStopRightFire_Validate()
{
	return true; //dont know why we need the validation to stop firing, but it doesnt work without it
}

/* Attempt to stop firing */
void ABattleChairsCharacter::Server_AttemptStopRightFire_Implementation()
{
	if (Role == ROLE_Authority)
	{
		StopRightFire();
	}
}

/* Stop firing */
void ABattleChairsCharacter::StopRightFire()
{
	rightFire = false;
	rightFireDelay = firerate;
}

void ABattleChairsCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// only fire for first finger down
	if (FingerIndex == 0)
	{
		LeftFire();
	}
}

void ABattleChairsCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ABattleChairsCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ABattleChairsCharacter::ThrusterFUp()
{
	UE_LOG(LogTemp, Warning, TEXT("thrusterFUp"));
	if (thrusterF <= 3) thrusterF += 0.1f;
}

void ABattleChairsCharacter::ThrusterFDown()
{
	UE_LOG(LogTemp, Warning, TEXT("thrusterFDown"));
	if (thrusterF >= .1f) thrusterF -= 0.1f;
}

void ABattleChairsCharacter::ThrusterLUp()
{
	UE_LOG(LogTemp, Warning, TEXT("thrusterLUp"));
	if (thrusterL <= 3) thrusterL += 0.1f;
}

void ABattleChairsCharacter::ThrusterLDown()
{
	UE_LOG(LogTemp, Warning, TEXT("thrusterLDown"));
	if (thrusterL >= 0.1) thrusterL -= 0.1f;
}

void ABattleChairsCharacter::ThrusterRUp()
{
	UE_LOG(LogTemp, Warning, TEXT("thrusterRUp"));
	if (thrusterR <= 3) thrusterR += 0.1f;
}

void ABattleChairsCharacter::ThrusterRDown()
{
	UE_LOG(LogTemp, Warning, TEXT("thrusterRDown"));
	if (thrusterR >= 0.1) thrusterR -= 0.1f;
}

/*
void ABattleChairsCharacter::ThrusterF(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		UE_LOG(LogTemp, Warning, TEXT("thrusterF"));
		if (thrusterF <= 3 && Value >= 0) thrusterF += 0.1;
		if (thrusterF < 0) thrusterF = 0;
	}
	
}

void ABattleChairsCharacter::ThrusterL(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (thrusterL <= 3) thrusterL += 0.1;
		if (thrusterL <= 0) thrusterL = 0;
	}
}

void ABattleChairsCharacter::ThrusterR(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (thrusterR <= 3) thrusterR += 0.1;
		if (thrusterR <= 0) thrusterR = 0;
	}
}
*/

void ABattleChairsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABattleChairsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABattleChairsCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (leftFire) {
		leftFireDelay--;
		if (leftFireDelay <= 0) {
			LeftFire();
			leftFireDelay = firerate;
		}
	}
	if (rightFire) {
		rightFireDelay--;
		if (rightFireDelay <= 0) {
			RightFire();
			rightFireDelay = firerate;
		}
	}
	AddMovementInput(-1 * GetActorForwardVector(), thrusterF);

	FRotator LeftThrusterOffSet = FRotator(0.0);
	LeftThrusterOffSet.Add(0.0f, 45.0f, 0.0f);
	FVector LeftThrusterDir = LeftThrusterOffSet.RotateVector(GetActorForwardVector());

	FRotator RightThrusterOffSet = FRotator(0.0);
	RightThrusterOffSet.Add(0.0f, -45.0f, 0.0f);
	FVector RightThrusterDir = RightThrusterOffSet.RotateVector(GetActorForwardVector());

	AddMovementInput(LeftThrusterDir, thrusterL);
	AddMovementInput(RightThrusterDir, thrusterR);

	lift = min(thrusterF, thrusterL, thrusterR);
	if (lift > .4) {
		lift = 25 + sqrt(lift * 100) - (GetActorLocation().Z)/120;
		FVector up = FVector(0, 0, lift);
		LaunchCharacter(up, false, false);
	}

	//Mitch: ---START OF HARDWARE BLOCK--

	//Mitch: temporary variables to read from hardware
	DWORD32 bytesRead;
	unsigned int toRead;
	unsigned int nbChar = 100;
	char buffer[100];

	//Mitch: clear temporary buffer
	for (unsigned int i = 0; i < 100; i++) buffer[i] = '\0';

	//Mitch: don't know, apparently needed
	ClearCommError(hSerial, (LPDWORD)(&errors), &status);

	//Mitch: if data broadcast from hardware, read it
	if (status.cbInQue > 0) {

		//Mitch: make sure number of bytes to read does not exceed buffer size
		if (status.cbInQue > nbChar) {
			toRead = nbChar;
		} else {
			toRead = status.cbInQue;
		}

		//Mitch: try to read bytes from hardware, put into into temporary buffer
		if (ReadFile(hSerial, (LPVOID)buffer, toRead, (LPDWORD)(&bytesRead), NULL) != 0) {

			//Mitch: for debugging, can ignore this
			//UE_LOG(LogTemp, Warning, TEXT("%s\n"), ANSI_TO_TCHAR(buffer));

			//Mitch: push bytes from temporary buffer into control buffer
			for (DWORD32 i = 0; i < bytesRead; i++) {
				controlBuffer[controlBufferPos] = buffer[i];
				controlBufferPos++;

				//Mitch: when semi-colon is found, parse entire input command
				if (buffer[i] == ';') {
					UE_LOG(LogTemp, Warning, TEXT("Arduino input: %s\n"), ANSI_TO_TCHAR(controlBuffer));

					//Mitch: initialized input variables, may be a problem later on
					int button=0, encoderR=0, encoderL=0;

					//Mitch: format string "button,encoderR,encoderL;" should be read
					sscanf(controlBuffer, "%d,%d,%d;", &button, &encoderR, &encoderL);

					//Mitch: set leftFire to button value (similar to holding mouse button)
					leftFire = button ? 1 : 0;

					//Mitch: set thrusterR to scaled down encoderR value
					float tthrustR = (float)encoderR / 10.f;
					if (tthrustR < 0.f) tthrustR = 0.f;
					if (tthrustR > 3.f) tthrustR = 3.f;
					thrusterR = tthrustR;

					//Mitch: set thrusterL to scaled down encoderL value
					float tthrustL = (float)encoderL / 10.f;
					if (tthrustL < 0.f) tthrustL = 0.f;
					if (tthrustL > 3.f) tthrustL = 3.f;
					thrusterL = tthrustL;

					//Mitch: for debugging, can ignore this
					//UE_LOG(LogTemp, Warning, TEXT("button = %d\n"), button);
					//UE_LOG(LogTemp, Warning, TEXT("encoderL = %d encoderR = %d\n"), encoderL, encoderR);

					//Mitch: clear control buffer after successfully reading command, prepare for new input
					for (unsigned int i = 0; i < controlBufferPos; i++) controlBuffer[i] = '\0';
					controlBufferPos = 0;
				}
			}

		}
	}

	//Mitch: ---END OF HARDWARE BLOCK--
}

float ABattleChairsCharacter::min(float a, float b, float c) {
	if (a < b && a < c) return a;
	if (b < c) return b;
	return c;
}
