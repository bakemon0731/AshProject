// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GamePlayAbilitySystem/Characters/NexusCharacterBase.h"// 親クラスのヘッダー
#include "NexusEnemybase.generated.h"

UCLASS()
class GAS_API ANexusEnemybase : public ANexusCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANexusEnemybase();
	
	// 詳細パネルで設定できる BehaviorTree 変数
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	class UBehaviorTree* BehaviorTree;
	
	// 攻撃範囲と防御範囲を直接持たせる(元BPI_EnemyAI)
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float AttackRadius = 150.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="AI")
	float DefendRadius = 300.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
