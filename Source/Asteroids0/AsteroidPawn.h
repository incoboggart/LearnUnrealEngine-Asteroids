// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "AsteroidPawn.generated.h"

UCLASS()
class ASTEROIDS0_API AAsteroidPawn : public APawn
{
	GENERATED_BODY()

private:
    

public:
	// Sets default values for this pawn's properties
	AAsteroidPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
};
