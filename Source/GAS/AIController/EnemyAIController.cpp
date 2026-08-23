// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GAS/EnemyCharacter/NexusEnemybase.h"// 敵専用クラスのヘッダー


// Sets default values
AEnemyAIController::AEnemyAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::SetStateAsPassive()
{
	//Pawn参照からPawn（GetControlledPawnノード）を取得
	APawn* ControlledPawn = GetPawn();
	//Pawnが有効ではない場合は実行しない。
	if (!ControlledPawn) return;
	
	// ControlldPawnがGASを持っているか確認。
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn);
	//ASIが有効かどうか確認。有効な場合は実行。
	if (ASI)
	{
		// Controlled Pawnから AbilitySystemComponent を取得
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		//ASCが有効かどうか確認。有効な場合は実行。
		if (ASC)
		{
			// 現在のStateエフェクトの削除
			//CurrentStateEffect変数が有効かどうか。有効な場合は実行。
			if (CurrentStateEffect.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(CurrentStateEffect,-1);
			}
			
			//新しいStateエフェクトを付与。
			//PassiveStateEffectが有効かどうか。有効な場合は実行。
			if (PassiveStateEffect)
			{
				//エフェクトのコンテキストを作成
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				
				FGameplayEffectSpecHandle SpecHandle = ASC -> MakeOutgoingSpec(PassiveStateEffect,0.0f,EffectContext);
				//SpecHandleが有効かどうか。有効な場合は実行。
				if (SpecHandle.IsValid())
				{
					//GameplayEffect（ApplyGameplayEffectSpecToSelf）を適用し、戻り値をCurrentStateEffect変数に保存。
					CurrentStateEffect = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	//Pawnではない場合実行しない
	if (!InPawn) return;
	
	// 敵専用のC++クラスにキャストする
	ANexusEnemybase* EnemyPawn = Cast<ANexusEnemybase>(InPawn);
	
	//EnemyPawnが有効かどうか確認
	if (EnemyPawn)
	{
		// BehaviorTree変数を取得して有効か確認し実行
		if (IsValid(EnemyPawn->BehaviorTree))
		{
			RunBehaviorTree(EnemyPawn->BehaviorTree);
		}
		// BehaviorTree変数が有効ではない場合
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("その敵に有効なビヘイビアツリーは無い"));
		}
		
		//Passive状態への移行する関数
		SetStateAsPassive();
		
			//BlackboardComponentが有効かどうか確認。有効な場合は実行。
			if (UBlackboardComponent* BB = GetBlackboardComponent())
			{
				BB->SetValueAsFloat(AttackRediusKeyName,EnemyPawn->AttackRadius);
				BB->SetValueAsFloat(DefendRadiusKeyName,EnemyPawn->DefendRadius);
			}
		
		//SetTimerByEventのセット
		GetWorldTimerManager().SetTimer(CheckForgottenActorTimer,
			this,&AEnemyAIController::CheckIfForgottenSeeActor,
			0.5f,
			true
			);
	}
	
}

void AEnemyAIController::CheckIfForgottenSeeActor()
{
}

// Called when the game starts or when spawned
void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

