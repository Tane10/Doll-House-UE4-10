// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProject2.h"
#include "MyCharacter.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//create audio for breathing
	Breathing = CreateDefaultSubobject<UAudioComponent>(TEXT("player breathing"));
	Breathing->AttachTo(RootComponent);
	Breathing->bAutoActivate = true;
	Breathing->Play(0.0f);

	//create audio for heavy breathing
	HardBreathing = CreateDefaultSubobject<UAudioComponent>(TEXT("player hard breathing"));
	HardBreathing->AttachTo(RootComponent);
	HardBreathing->bAutoActivate = false;
	HardBreathing->Play(0.0f);

	// Create a springArm and attatch to root
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 100.0f; // The camera follows at this distance behind the character
	SpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

											   // Create a follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//attach camera to boom
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

											 // Take control of the default player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	walkSpeed = 150.0f;
	walkTimeStart = false;
	sprintTimer = 0.0f;
	secondarySprintTimer = 0.0f;
	
}

// Called every frame
void AMyCharacter::Tick( float DeltaTime )
{
	//Rotate our actor's yaw, which will turn our camera because we're attached to it
	{
		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw += CameraInput.X;
		SetActorRotation(NewRotation);
	}

	//Rotate our camera's pitch, but limit it so we're always looking downward
	{
		FRotator NewRotation = SpringArm->GetComponentRotation();
		//lock rotation so cant look all the way up or down
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + CameraInput.Y, -80.0f, 70.0f);
		SpringArm->SetWorldRotation(NewRotation);
	}

	Super::Tick(DeltaTime);

	{
		//Scale our movement input axis values by current walkspeed, update location
		MovementInput = MovementInput.GetSafeNormal() * walkSpeed;
		NewLocation = GetActorLocation();
		NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
		NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
		SetActorLocation(NewLocation);
	}

	//only sprint for a certain amount of time then back to walking
	//starts a timer after certain time to keep track of when the player can start sprinting again
	if (bSprint == true)
	{
		walkTimeStart = true;
		if (sprintTimer <= 200.0f)
		{
			walkSpeed = 500.0f;
		}
		else
		{
			secondarySprintTimer += 1.0f;
			walkTimeStart = false;
			walkSpeed = 150.0f;
		}
	}
	//make player walk and start timer if player lets go of shift key
	else if (bSprint == false)
	{
		walkSpeed = 150.0f;
		secondarySprintTimer += 1.0f;
		walkTimeStart = false;
	}

	if (walkTimeStart == true)
	{
		sprintTimer += 1.0f;
	}
	//as long as secondarytimer isnt default value and bigger than 100 then stop player from running
	//if timer is less than 100 then enough time hasnt passed and player will not be able to sprint again
	else if (walkTimeStart == false)
	{
		if ((secondarySprintTimer <= 100.0f) && (secondarySprintTimer >= 1.0f))
		{
			//normal breathing when walking around
			//maybe delay before deactivating hard breathing after sprinting
			HardBreathing->Deactivate();
			Breathing->Activate();
			sprintTimer = 201.0f;
		}
		else if (secondarySprintTimer>100.0f)
		{
			//panting when running
			Breathing->Deactivate();
			HardBreathing->Activate();
			sprintTimer = 0.0f;
			secondarySprintTimer = 0.0f;
		}
		else if (secondarySprintTimer == 0.0f)
		{
			sprintTimer = 0.0f;
		}
	}


}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	InputComponent->BindAxis("CameraPitch", this, &AMyCharacter::PitchCamera);
	InputComponent->BindAxis("CameraYaw", this, &AMyCharacter::YawCamera);

	InputComponent->BindAction("FastWalk", IE_Pressed, this, &AMyCharacter::FastWalk);
	InputComponent->BindAction("FastWalk", IE_Released, this, &AMyCharacter::FastWalkRelease);

}

void AMyCharacter::FastWalk()
{
	bSprint = true;
}

void AMyCharacter::FastWalkRelease()
{
	bSprint = false;
}

void AMyCharacter::MoveForward(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AMyCharacter::MoveRight(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AMyCharacter::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void AMyCharacter::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}
