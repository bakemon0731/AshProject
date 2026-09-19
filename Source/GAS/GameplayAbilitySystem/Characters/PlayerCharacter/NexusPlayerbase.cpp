// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusPlayerbase.h"


// Sets default values
ANexusPlayerbase::ANexusPlayerbase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANexusPlayerbase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANexusPlayerbase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ANexusPlayerbase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// 詠唱開始通知を受け取ったら、UIにCallする。
void ANexusPlayerbase::NotifyStartCast_Implementation(float ChargeTime)
{
	OnCastStarted.Broadcast(ChargeTime);
}

// 詠唱キャンセル通知を受け取ったら、UIにCallする。
void ANexusPlayerbase::NotifyCancelCast_Implementation()
{
	OnCastCanceled.Broadcast();
}



