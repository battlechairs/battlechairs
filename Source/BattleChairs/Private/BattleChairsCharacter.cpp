// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "UnrealNetwork.h"
#include "BattleChairsCharacter.h"
#include "BattleChairsProjectile.h"
#include "ProjectileParent.h"
#include "Animation/AnimInstance.h"



//////////////////////////////////////////////////////////////////////////
// ABattleChairsCharacter

// hardware event enumerations
#define ENCODER_TOP_UP      0x001
#define ENCODER_TOP_DOWN    0x002
#define BUTTON_TOP_UP       0x004
#define BUTTON_TOP_DOWN     0x008
#define ENCODER_MIDDLE_UP   0x010
#define ENCODER_MIDDLE_DOWN 0x020
#define BUTTON_BOTTOM_UP    0x040
#define BUTTON_BOTTOM_DOWN  0x080
#define ENCODER_BOTTOM_UP   0x100
#define ENCODER_BOTTOM_DOWN 0x200

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
	thrusterFV = FVector(0);
	thrusterLV = FVector(0);
	thrusterRV = FVector(0);
	lift = 0;
	firerate = 5;
	knockback = -10000;
	turnrate = 5;
	uniqueID = GetUniqueID();
	speed = GetVelocity();


	rotationalVelocity = 0.f;
	rotationalVelocityPositive = 0.f;
	rotationalVelocityNegative = 0.f;
	rotationalVelocityMaximum = 10.f;
	rotationalVelocityIncrement = 2.f;
	rotationalDrag = 3.f;
	chairDirection = GetActorRotation();
	cameraStart = FVector(-45.8, 0, 153.8);

	// Create a CameraComponent	
	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	//FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	//thrusterFPS = ObjectInitializer.CreateDefaultSubobject<UParticleSystem>(this, TEXT("thrusterFPS"));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(0.0f, 0.0f, 0.0f);

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
		LPCWSTR portName = L"\\\\.\\COM3";
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
	uint32 ID = uniqueID;
	FVector speed = GetVelocity();
	if (Role == ROLE_Authority)
	{
		LeftFire(ID, speed);
	}
}
/* Fire projectile */
void ABattleChairsCharacter::LeftFire(int32 ID, FVector Speed)
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = FRotator(0, chairDirection.Yaw, 0); //GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		FVector offSet = FVector(0.0f, -180.0f, 0.0f);
		FRotator turn = FRotator(0.0);

		rotationalVelocityPositive += rotationalVelocityIncrement;

		if (rightFire) {
			LaunchPawn(knockback * GetWorld()->GetDeltaSeconds() * GetActorForwardVector(), false, false);
		}
		//const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset) + SpawnRotation.RotateVector(offSet);
		FVector testGunOffset = FVector(175.0f, 85.0f, 20.0f); //(150.0f, 75.0f, 35.0f);
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(testGunOffset) + SpawnRotation.RotateVector(offSet);
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			setplayer(World->SpawnActor<ABattleChairsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation), ID, Speed);
		}

		leftFire = true;
	}
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
		const FRotator SpawnRotation = FRotator(0, chairDirection.Yaw, 0);
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		FRotator turn = FRotator(0.0);

		rotationalVelocityNegative += rotationalVelocityIncrement;

		if (leftFire) {
			LaunchPawn(knockback * GetWorld()->GetDeltaSeconds() * GetActorForwardVector(), false, false);
		}
		//const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);
		FVector testGunOffset = FVector(175.0f, 85.0f, 20.0f);
		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(testGunOffset);
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<ABattleChairsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}

		rightFire = true;
	}

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
	UE_LOG(LogTemp, Warning, TEXT("ID: %d"), GetUniqueID());
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
		LeftFire(uniqueID,GetVelocity());
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
	if (thrusterF > .09f && thrusterF < 0.2f) thrusterF = 0.f;
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
	if (thrusterL > .09f && thrusterL < 0.2f) thrusterL = 0.f;
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
	if (thrusterR > .09f && thrusterR < 0.2f) thrusterR = 0.f;
}

bool ABattleChairsCharacter::ThrusterFON()
{
	return (thrusterF > 0);
}


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

void ABattleChairsCharacter::AddControllerPitchInput(float val)
{
	// use quaternions to rotate camera
	FQuat oldRotation = FirstPersonCameraComponent->GetRelativeTransform().GetRotation();
	FQuat deltaRotation = FQuat(oldRotation.GetAxisY(), val / 50.f * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	FirstPersonCameraComponent->SetRelativeRotation(FRotator(deltaRotation * oldRotation));
}

void ABattleChairsCharacter::AddControllerYawInput(float val)
{
	// use quaternions to rotate camera
	FQuat oldRotation = FirstPersonCameraComponent->GetRelativeTransform().GetRotation();
	FQuat deltaRotation = FQuat(FVector(0.f, 0.f, 1.f), val / 50.f * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	FirstPersonCameraComponent->SetRelativeRotation(FRotator(deltaRotation * oldRotation));
}

void ABattleChairsCharacter::UpdateOculusCamera(const FRotator& viewRotation, const FVector& viewPosition) {
	FVector newPosition = cameraStart - GetActorRotation().operator-().RotateVector(viewPosition) + viewPosition;
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(newPosition, viewRotation);
}

inline void ABattleChairsCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (leftFire) {
		leftFireDelay -= (int)ceil(10.0 * DeltaTime);
		if (leftFireDelay <= 0) {
			if (rightFire) rightFireDelay = 0;
			LeftFire(uniqueID, GetVelocity());
			leftFireDelay = firerate;
		}
	}
	if (rightFire) {
		rightFireDelay -= (int)ceil(10.0 * DeltaTime);
		if (rightFireDelay <= 0) {
			RightFire();
			rightFireDelay = firerate;
		}
	}
	AddMovementInput(-1 * GetActorForwardVector(), thrusterF);
	FRotator turn = FRotator(0.0);

	float currentRotationalDrag;

	rotationalVelocity = rotationalVelocityPositive - rotationalVelocityNegative;

	if (rightFire != leftFire) {// XOR operator
		currentRotationalDrag = 0.5 * rotationalDrag;
	}
	else {
		if (rightFire && leftFire)
			rotationalVelocity = rotationalVelocityPositive = rotationalVelocityNegative = 0.f;
		currentRotationalDrag = rotationalDrag;
	}

	if (abs(rotationalVelocity) > 0.0001f) {
		if (rotationalVelocity < -rotationalVelocityMaximum) {
			rotationalVelocity = -rotationalVelocityMaximum;
		}
		else if (rotationalVelocity > rotationalVelocityMaximum) {
			rotationalVelocity = rotationalVelocityMaximum;
		}

		const FRotator SpawnRotation = GetControlRotation();
		
		turn.Add(0.f, rotationalVelocity, 0.f);

		rotationalVelocityPositive -= rotationalVelocityPositive * currentRotationalDrag * DeltaTime;
		if (rotationalVelocityPositive < 0.f) rotationalVelocityPositive = 0.f;
		rotationalVelocityNegative -= rotationalVelocityNegative * currentRotationalDrag * DeltaTime;
		if (rotationalVelocityNegative < 0.f) rotationalVelocityNegative = 0.f;

		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);
		chairDirection = SpawnRotation - turn;
		AddActorLocalRotation(chairDirection - SpawnRotation);
	}
	ClientSetRotation(GetActorRotation() - turn);

	FRotator LeftThrusterOffSet = FRotator(0.0);
	LeftThrusterOffSet.Add(0.0f, 45.0f, 0.0f);
	FVector LeftThrusterDir = LeftThrusterOffSet.RotateVector(GetActorForwardVector());

	FRotator RightThrusterOffSet = FRotator(0.0);
	RightThrusterOffSet.Add(0.0f, -45.0f, 0.0f);
	FVector RightThrusterDir = RightThrusterOffSet.RotateVector(GetActorForwardVector());

	AddMovementInput(LeftThrusterDir, thrusterL);
	AddMovementInput(RightThrusterDir, thrusterR);



	//lift = min(thrusterF, thrusterL, thrusterR);
	lift = (thrusterF + thrusterL + thrusterR) / 3.0;
	SpawnRate = (ceil(lift * 4));
	thrusterFV = FVector(0, 0, thrusterF * -300);
	thrusterLV = FVector(0, 0, thrusterL * -300);
	thrusterRV = FVector(0, 0, thrusterR * -300);

	/*
	if (lift > .4)
	{
	lift = 30 + sqrt(lift * 100) - (GetActorLocation().Z) / 120;
	FVector up = FVector(0, 0, lift);
	AddMovementInput(GetActorUpVector(), lift);
	}
	*/

	//gravity
	AddMovementInput(GetActorUpVector(), lift - 2);

	//Server_AttemptLift();
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
		}
		else {
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
					processHardwareEvent();
				}
			}

		}
	}
	//Mitch: ---END OF HARDWARE BLOCK--
}

//Mitch: protected function to perform hardware commands
void ABattleChairsCharacter::processHardwareEvent() {
	UE_LOG(LogTemp, Warning, TEXT("Arduino input: %s\n"), ANSI_TO_TCHAR(controlBuffer));

	//Mitch: initialized input variables, may be a problem later on
	int event = 0, hash = 0, readValues = 0;

	//Mitch: format string "event:<event>,hash:<hash>;" should be read
	readValues = sscanf(controlBuffer, "event:%d,hash:%d;", &event, &hash);

	if (readValues != 2 || hash != (event + 1)) {
		//Mitch: ignore invalid command, clear control buffer, and prepare for new input
		for (unsigned int i = 0; i < controlBufferPos; i++) controlBuffer[i] = '\0';
		controlBufferPos = 0;
		return;
	}

	//Mitch: call functions related to read events
	if (event & ENCODER_TOP_UP) ThrusterRUp();
	if (event & ENCODER_TOP_DOWN) ThrusterRDown();
	if (event & BUTTON_TOP_UP) Server_AttemptStopRightFire();
	if (event & BUTTON_TOP_DOWN) Server_AttemptRightFire();
	if (event & ENCODER_MIDDLE_UP) ThrusterFUp();
	if (event & ENCODER_MIDDLE_DOWN) ThrusterFDown();
	if (event & BUTTON_BOTTOM_UP) Server_AttemptStopLeftFire();
	if (event & BUTTON_BOTTOM_DOWN) Server_AttemptLeftFire();
	if (event & ENCODER_BOTTOM_UP) ThrusterLUp();
	if (event & ENCODER_BOTTOM_DOWN) ThrusterLDown();

	//Mitch: for debugging, can ignore this
	//UE_LOG(LogTemp, Warning, TEXT("button = %d\n"), button);
	//UE_LOG(LogTemp, Warning, TEXT("encoderL = %d encoderR = %d\n"), encoderL, encoderR);

	//Mitch: clear control buffer after successfully reading command, prepare for new input
	for (unsigned int i = 0; i < controlBufferPos; i++) controlBuffer[i] = '\0';
	controlBufferPos = 0;
}

inline float ABattleChairsCharacter::min(float a, float b, float c) {
	if (a < b && a < c) return a;
	if (b < c) return b;
	return c;
}
