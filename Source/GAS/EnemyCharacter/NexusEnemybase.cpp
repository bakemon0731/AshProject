// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusEnemybase.h"


// Sets default values
ANexusEnemybase::ANexusEnemybase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANexusEnemybase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANexusEnemybase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ANexusEnemybase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

