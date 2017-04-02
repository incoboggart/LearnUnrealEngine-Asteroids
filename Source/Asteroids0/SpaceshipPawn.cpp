// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Asteroids0.h"
#include "SpaceshipPawn.h"
#include "Asteroids0Projectile.h"
#include "TimerManager.h"

const FName ASpaceshipPawn::MoveForwardBinding("MoveForward");
const FName ASpaceshipPawn::MoveRightBinding("MoveRight");
const FName ASpaceshipPawn::FireForwardBinding("FireForward");

ASpaceshipPawn::ASpaceshipPawn()
{	
    auto shipMesh = _shipMesh.LoadSynchronous();
	// Create the mesh component
    ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(shipMesh);

    RootComponent = ShipMeshComponent;
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;
    _movementSpeed = 1000.f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
}

void ASpaceshipPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(FireForwardBinding);
}

void ASpaceshipPawn::PostEditChangeProperty(FPropertyChangedEvent & e)
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

void ASpaceshipPawn::Tick(float DeltaSeconds)
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
        World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ASpaceshipPawn::ShotTimerExpired, FireRate);

        bCanFire = false;
    }
}

void ASpaceshipPawn::FireShot(FVector FireDirection)
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
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ASpaceshipPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void ASpaceshipPawn::ShotTimerExpired()
{
	bCanFire = true;
}

