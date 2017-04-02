// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Asteroids0.h"
#include "Asteroids0GameMode.h"
#include "SpaceshipPawn.h"

AAsteroids0GameMode::AAsteroids0GameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = ASpaceshipPawn::StaticClass();
}

