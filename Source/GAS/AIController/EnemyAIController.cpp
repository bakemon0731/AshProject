// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GAS/EnemyCharacter/NexusEnemybase.h"// 敵専用クラスのヘッダー
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"


// Sets default values
AEnemyAIController::AEnemyAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Perceptionコンポーネントの生成と設定
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	
	// 視覚 (Sight) コンフィグの設定
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		//ターゲットに気づく最大視野距離。
		SightConfig->SightRadius = 1500.0f;
		//すでに目撃されたターゲットに気づく最大視野距離。
		SightConfig->LoseSightRadius = 2000.0f;
		//周辺視野の半分の角度。
		SightConfig->PeripheralVisionAngleDegrees = 60.0f;
		//刺激の期限切れの時間。０は何もしない。
		SightConfig->SetMaxAge(20.f);
		
		// 検知対象フラグをON
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		
		PerceptionComp->ConfigureSense(*SightConfig);
		// 主要な感覚に指定
		PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	}
	
	// 聴覚 (Hearing) コンフィグの設定
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		//音が聞こえる最大範囲。
		HearingConfig->HearingRange = 500.0f;
		//刺激の期限切れの時間。０は何もしない。
		HearingConfig->SetMaxAge(3.0f);
		
		// 検知対象フラグをON
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		
		PerceptionComp->ConfigureSense(*HearingConfig);
	}
	
	// ダメージ (Damage) コンフィグの設定
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	if(DamageConfig)
	{
		//刺激の期限切れの時間。０は何もしない。
		DamageConfig->SetMaxAge(5.0f);
		
		PerceptionComp->ConfigureSense(*DamageConfig);
	}
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

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();
	
	//Clear and Invalidate Timer by Handleノード。CheckForgottenActorTimer変数を停止。
	GetWorldTimerManager().ClearTimer(CheckForgottenActorTimer);
}

void AEnemyAIController::CheckIfForgottenSeeActor()
{
}

bool AEnemyAIController::CanSenseActor(AActor* Actor, TSubclassOf<class UAISense> Sense, FAIStimulus& OutStimulus)
{
	PerceptionComp = GetPerceptionComponent();
	// 対象アクター、Senseクラス、Perceptionコンポーネントのいずれかが無効なら弾く
	if (!Actor || !Sense || !PerceptionComp)
	{
		return false;
	}
	
	FActorPerceptionBlueprintInfo Info;
	
	// 対象アクターに関するPerception情報（過去に検知した全感覚のリスト）を取得
	//GetActorsPercceptionノード
	if (PerceptionComp->GetActorsPerception(Actor,Info))
	{
		// 引数で指定されたSense（例：視覚）の固有IDを取得
		FAISenseID ActorSenseID = UAISense::GetSenseID(Sense);
		
		// 検知した感覚情報のリストをループして探す
		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			// Senseの種類が一致したら、そのデータをOutStimulusに入れて結果を返す
			if (Stimulus.Type == ActorSenseID)
			{
				OutStimulus = Stimulus;
				//「現在進行形でターゲットを視界に捉えているか（または音が聞こえているか）」の true/false を関数の結果として返す
				return Stimulus.WasSuccessfullySensed();
			}
		}
	}
	// 情報が見つからなければFalse
	return false;
}

// Called when the game starts or when spawned
void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	// バインドイベント。AIが五感を刺激されときHandlePerceptionUpdated関数を実行。
	if (PerceptionComp)
	{
		PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::HandlePerceptionUpdated);
	}
}

// Perception更新時のメイン処理（BPの ForEachLoop & Sequence を代替
void AEnemyAIController::HandlePerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor) continue;
		
		FAIStimulus Stimulus;
		// --------------------------------------------------
		// 1. 視覚 (Sight) の判定
		// --------------------------------------------------
		if (CanSenseActor(Actor,UAISense_Sight::StaticClass(),Stimulus))
		{
			// True: 視界に入った(HandleSensedSightを実行)
			HandleSensedSight(Actor);
		}
		else
		{
			// False: 視界から外れた（ロスト）
			HandleLostSight(Actor);
		}
		
		// --------------------------------------------------
		// 2. 聴覚 (Hearing) の判定
		// --------------------------------------------------
		if (CanSenseActor(Actor,UAISense_Hearing::StaticClass(),Stimulus))
		{
			// 音が鳴った場所（StimulusLocation）を渡して実行
			HandleSensedSound(Stimulus.StimulusLocation);
		}
		
		// --------------------------------------------------
		// 3. ダメージ (Damage) の判定
		// --------------------------------------------------
		if (CanSenseActor(Actor,UAISense_Damage::StaticClass(),Stimulus))
		{
			HandleSenseDamage(Actor);
		}
	}
}

void AEnemyAIController::HandleSensedSight(AActor* Actor)
{
}

void AEnemyAIController::HandleLostSight(AActor* Actor)
{
}

void AEnemyAIController::HandleSensedSound(FVector Location)
{
}

void AEnemyAIController::HandleSenseDamage(AActor* Actor)
{
}

// Called every frame
void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

