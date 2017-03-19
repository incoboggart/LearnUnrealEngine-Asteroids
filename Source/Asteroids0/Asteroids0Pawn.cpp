// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Asteroids0.h"
#include "Asteroids0Pawn.h"
#include "Asteroids0Projectile.h"
#include "TimerManager.h"

const FName AAsteroids0Pawn::MoveForwardBinding("MoveForward");
const FName AAsteroids0Pawn::MoveRightBinding("MoveRight");
const FName AAsteroids0Pawn::FireForwardBinding("FireForward");

AAsteroids0Pawn::AAsteroids0Pawn()
{	
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
    auto shipMesh = _shipMesh.LoadSynchronous();
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(shipMesh);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
}

void AAsteroids0Pawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(FireForwardBinding);
}

void AAsteroids0Pawn::PostEditChangeProperty(FPropertyChangedEvent & e)
{
    Super::PostEditChangeProperty(e);

    static FName ShipMeshName(TEXT("_shipMesh"));
    if(e.Property->GetFName() == ShipMeshName)
    {
        UStaticMesh* shipMesh = nullptr;
        if(_shipMesh.IsValid())
        {
            UE_LOG(LogAsteroids0, Log, TEXT("Ship mesh changed to valid asset"))
            shipMesh = _shipMesh.Get();
        }
        
        UE_LOG(LogAsteroids0, Log, TEXT("%s"), *_shipMesh.ToString())
        ShipMeshComponent->SetStaticMesh(_shipMesh.LoadSynchronous());
    }
    else
    {
        UE_LOG(LogAsteroids0, Log, TEXT("%s"), *e.Property->GetFName().ToString())
    }
}

void AAsteroids0Pawn::Tick(float DeltaSeconds)
{
    // Read input
    const float movementInput = GetInputAxisValue(MoveForwardBinding);
    const float rotationInput = GetInputAxisValue(MoveRightBinding);

    // Get movement direction
    const FVector movementDirection = GetActorForwardVector();
    const FRotator rotationDirection = FRotator(0, 1, 0);
    
    const FVector movementDelta = movementDirection*_movementSpeed*movementInput*DeltaSeconds;
    const FRotator rotationDelta = rotationDirection*_rotationSpeed*rotationInput*DeltaSeconds;

    const FRotator rotation = GetActorRotation();

    if (movementDelta.SizeSquared() > 0.0f)
    {
        FHitResult Hit(1.f);
        RootComponent->MoveComponent(movementDelta, rotation + rotationDelta, true, &Hit);
    }
    else if(!rotationDelta.IsNearlyZero())
    {
        RootComponent->AddWorldRotation(rotationDelta);
    }

    const float fireInput = GetInputAxisValue(FireForwardBinding);
    const FVector fireDirection = GetActorForwardVector()*fireInput;

    if (!bCanFire)
    {
        return;
    }
    if (fireDirection.SizeSquared() > 0.0f)
    {
        const FRotator FireRotation = fireDirection.Rotation();
        // Spawn projectile at an offset from this pawn
        const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

        UWorld* const World = GetWorld();
        if (World != NULL)
        {
            // spawn the projectile
            World->SpawnActor<AAsteroids0Projectile>(SpawnLocation, FireRotation);
        }

        bCanFire = false;
        World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AAsteroids0Pawn::ShotTimerExpired, FireRate);

        bCanFire = false;
    }
}

void AAsteroids0Pawn::FireShot(FVector FireDirection)
{
	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<AAsteroids0Projectile>(SpawnLocation, FireRotation);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AAsteroids0Pawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void AAsteroids0Pawn::ShotTimerExpired()
{
	bCanFire = true;
}

