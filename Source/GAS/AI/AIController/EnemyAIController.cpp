// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GAS/GameplayAbilitySystem/Characters/EnemyCharacter/NexusEnemybase.h"
#include "GAS/Interface/Damageable.h"


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
	
	//Seekingを始める時間の変数の初期値
	TimeToSeeAfterLosingSight = 3.0f;
}

void AEnemyAIController::ChangeStateEffect(TSubclassOf<class UGameplayEffect> NewStateEffect)
{
	//Pawn参照からPawn（GetControlledPawnノード）を取得
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;
	
	// ControlldPawnがGASを持っているか確認。
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn);
	if (!ASI) return;
	
	// Controlled Pawnから AbilitySystemComponent を取得
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;
	
	// 現在のStateエフェクトを削除
	if(CurrentStateEffect.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(CurrentStateEffect,-1);
		// ハンドルを安全のために初期化
		CurrentStateEffect.Invalidate();
	}
	
	// Stateエフェクトを付与する
	if(NewStateEffect)
	{
		//エフェクトのコンテキストを作成
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(NewStateEffect,0.0f,EffectContext);
		if (SpecHandle.IsValid())
		{
			//GameplayEffect（ApplyGameplayEffectSpecToSelf）を適用し、戻り値をCurrentStateEffect変数に保存。
			CurrentStateEffect = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

//--------------------------------------------------
// GetCurrentState（現在のStateタグ取得）
// --------------------------------------------------
FGameplayTag AEnemyAIController::GetCurrentState() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return FGameplayTag::EmptyTag;
	
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);
			
			// "State" をルートに持つタグを取得する
			FGameplayTag RootStateTag = FGameplayTag::RequestGameplayTag(FName("State"));
			
			for (const FGameplayTag& Tag : OwnedTags)
			{
				if (Tag.MatchesTag(RootStateTag))
				{
					return Tag;
				}
			}
		}
	}
	return FGameplayTag::EmptyTag;
}

// --------------------------------------------------
// OnSameTeam（チーム判定）
// --------------------------------------------------
bool AEnemyAIController::OnSameTeam(AActor* OtherActor) const
{
	if (!OtherActor) return false;
	
	// 自分（制御対象のPawn）と対象アクターを IDamageable にキャスト
	const IDamageable* MyDamageable = Cast<IDamageable>(GetPawn());
	const IDamageable* TargetDamageable = Cast<IDamageable>(OtherActor);
	
	// 両者が IDamageable インターフェースを実装していればチーム番号を比較
	if (MyDamageable && TargetDamageable)
	{
		return MyDamageable->GetTeamNumber() == TargetDamageable->GetTeamNumber();
	}
	return false;
}

void AEnemyAIController::SetStateAsPassive()
{
	ChangeStateEffect(PassiveStateEffect);
	
	//Blackboardのターゲットと保持変数をクリア
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(AttackRediusKeyName);
	}
	AttackTarget = nullptr;
}

void AEnemyAIController::SetStateAsAttacking(AActor* Actor)
{
	// 引数が無効な場合は、記憶している現在の TargetActor を使用する(Selectノードに相当)
	AActor* NewAttackTarget = IsValid(Actor) ? Actor : AttackTarget;
	
	// どちらも無効な場合は処理を中断
	if(!IsValid(NewAttackTarget))
	{
		return;
	}
	
	//ターゲットに State.Dead タグがついているか確認
	if (IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(NewAttackTarget))
	{
		if (UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent())
		{
			FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
			if (TargetASC->HasMatchingGameplayTag(DeadTag))
			{
				// ターゲットが死んでいれば Passive に戻して終了
				SetStateAsPassive();
				return;
			}
		}
	}
	
	// Blackboard の AttackTarget を更新
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB -> SetValueAsObject(AttackTargetKeyName,NewAttackTarget);
	}
	
	//ステートエフェクトを Attacking に変更
	ChangeStateEffect(AttackingStateEffect);
	
	//ターゲットをメンバ変数に保存
	AttackTarget = NewAttackTarget;
}

void AEnemyAIController::SetStateAsSeeking(FVector Location)
{
	//ステートエフェクトを Seeking に変更
	ChangeStateEffect(SeekingStateEffect);
	
	//Blackboard の PointOfInterest を更新 (Set Value as Vector)
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(PointOfInterestKeyName,Location); 
	}
}

void AEnemyAIController::SetStateAsInvestigating(FVector Location)
{
	//ステートエフェクトを Investgating に変更
	ChangeStateEffect(InvestigatingStateEffect);
	
	//Blackboard の PointOfInterest を更新 (Set Value as Vector)
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(PointOfInterestKeyName,Location); 
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
		
		//SetTimerByEventのセット(CheckIfForgottenSeeActor)
		GetWorldTimerManager().SetTimer(
			CheckForgottenActorTimer,
			this,
			&AEnemyAIController::CheckIfForgottenSeeActor,
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
	
	//Clear and Invalidate Timer by Handleノード。CheckTargetAliveTimer変数を停止。
	GetWorldTimerManager().ClearTimer(CheckTargetAliveTimer);
}

void AEnemyAIController::CheckIfForgottenSeeActor()
{
	if (!PerceptionComp) return;
	
	// BPの「Get Known Perceived Actors (Sense: Sight)」に相当する処理
	TArray<AActor*> CurrentlyPerceivedActors;
	PerceptionComp->GetKnownPerceivedActors(UAISense_Sight::StaticClass(),CurrentlyPerceivedActors);
	
	// BPの「Length != Length」の判定
	if (KnownSeenActors.Num() != CurrentlyPerceivedActors.Num())
	{
		// C++特有の注意点：配列(KnownSeenActors)をループで回しながら
		// 中身をRemoveすると要素がズレてクラッシュする危険があるため、
		// 配列の「末尾(Num() - 1)」から「先頭(0)」に向かって逆順でループを回します。
		for (int32 i = KnownSeenActors.Num() -1; i >= 0; i--)
		{
			AActor* SeenActor = KnownSeenActors[i];
			
			// BPの「Find == -1 (見つからなかった場合)」に相当する処理
			// Containsは配列内に指定要素が存在するかをboolで返します
			if (!CurrentlyPerceivedActors.Contains(SeenActor))
			{
				HandleForgotActor(SeenActor);
			}
		}
	}
}

void AEnemyAIController::HandleForgotActor(AActor* Actor)
{
	if (!Actor) return;
	
	// BPの「Remove (Item)」に相当
	KnownSeenActors.Remove(Actor);
	
	// BPの「Actor == AttackTarget」の判定
	if (Actor == AttackTarget)
	{
		// ターゲットを完全に見失ったため、ステートをPassiveに戻す
		SetStateAsPassive();
	}
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

// Perception更新時のメイン処理（BPの ForEachLoop & Sequence を代替)
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
	// 検知したActorが無効なら処理しない
	if (!Actor) return;
	
	// 視覚したアクターを配列に追加 (BPの AddUnique)
	KnownSeenActors.AddUnique(Actor);
	
	// 同じチームなら処理を終了 (早期リターンでネストを防止)
	if (OnSameTeam(Actor)) return;
	
	//現在のStateを取得
	FGameplayTag CurrentState = GetCurrentState();
	
	//判定対象のStateタグを準備
	FGameplayTag TagPassive = FGameplayTag::RequestGameplayTag(FName("State.Passive"));
	FGameplayTag TagInvestigating = FGameplayTag::RequestGameplayTag(FName("State.Investigating"));
	FGameplayTag TagSeeking = FGameplayTag::RequestGameplayTag(FName("State.Seeking"));
	FGameplayTag TagAttacking = FGameplayTag::RequestGameplayTag(FName("State.Attacking"));
	
	// ステートごとの分岐処理 (Switch on Gameplay Tag の代替)
	if (CurrentState.MatchesTag(TagPassive)||
		CurrentState.MatchesTag(TagInvestigating)||
		CurrentState.MatchesTag(TagSeeking))
	{
		// Passive, Investigating, Seeking のいずれかなら攻撃ステートへ移行
		// 攻撃ステート移行関数を呼び出し
		SetStateAsAttacking(Actor);
	}
	else if (CurrentState.MatchesTagExact(TagAttacking))
	{
		// 攻撃中の場合、見つけたActorが現在のAttackTargetと同じか確認
		if (Actor == AttackTarget)
		{
			// 再びターゲットを視認したのでSeekingのタイマーを停止
			GetWorldTimerManager().ClearTimer(SeekAttackTargetTimer);
			
			//ターゲットが死んでいるか常に確認
			if (IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Actor))
			{
				if (UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent())
				{
					FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
					if (TargetASC->HasMatchingGameplayTag(DeadTag))
					{
						// ターゲットが死んでいれば Passive に戻す
						SetStateAsPassive();
					}
				}
			}
		}
	}
}

void AEnemyAIController::HandleLostSight(AActor* Actor)
{
	// 引数が無効な場合は処理しない
	if (!Actor) return;
	
	//見失ったアクターが現在のターゲット(AttackTarget)か確認
	if (Actor == AttackTarget)
	{
		// 現在のステートを取得
		FGameplayTag CurrentState = GetCurrentState();
		
		//判定対象のStateタグを準備
		FGameplayTag TagAttacking = FGameplayTag::RequestGameplayTag(FName("State.Attacking"));
		FGameplayTag TagInvestigating = FGameplayTag::RequestGameplayTag("State.Investigating");
		
		// 現在のステートが Attacking または Investigating の場合
		if (CurrentState.MatchesTagExact(TagAttacking) || CurrentState.MatchesTagExact(TagInvestigating))
		{
			// すでに動いているタイマーがあればリセットする (Clear and Invalidate Timer)
			GetWorldTimerManager().ClearTimer(SeekAttackTargetTimer);
			
			// 指定秒数後に SeekAttackTarget 関数を実行するタイマーをセット (Set Timer by Event)
			GetWorldTimerManager().SetTimer(
				SeekAttackTargetTimer,
				this,
				&AEnemyAIController::SeekAttackTarget,//呼び出す関数（Create Event）
				TimeToSeeAfterLosingSight,//待機時間
				false//ループしない
				);
		}
	}
}

void AEnemyAIController::SeekAttackTarget()
{
	// TargetActorが有効か確認
	if (IsValid(AttackTarget))
	{
		// ターゲットの現在位置を取得
		FVector TargetLocation = AttackTarget -> GetActorLocation();
		
		// 取得したLocationを渡してSeekingステートへ移行
		SetStateAsSeeking(TargetLocation);
	}
	
	// BPの「Clear and Invalidate Timer by Handle」に相当
	GetWorldTimerManager().ClearTimer(SeekAttackTargetTimer);
}


void AEnemyAIController::HandleSensedSound(FVector Location)
{
	
	// 現在のステートを取得
	FGameplayTag CurrentState = GetCurrentState();
	
	//判定対象のStateタグを準備
	FGameplayTag TagPassive = FGameplayTag::RequestGameplayTag("State.Passive");
	FGameplayTag TagInvestigating = FGameplayTag::RequestGameplayTag("State.Investigating");
	FGameplayTag TagSeeking = FGameplayTag::RequestGameplayTag("State.Seeking");
	
	if (CurrentState.MatchesTagExact(TagPassive) || CurrentState.MatchesTagExact(TagInvestigating) || CurrentState.MatchesTagExact(TagSeeking))
	{
		SetStateAsInvestigating(Location);
	}
}

void AEnemyAIController::HandleSenseDamage(AActor* Actor)
{
	// 引数が無効な場合は処理しない
	if (!Actor) return;
	
	// 同じチームなら処理を終了 (早期リターンでネストを防止)
	if (OnSameTeam(Actor)) return;
	
	// 現在のステートを取得
	FGameplayTag CurrentState = GetCurrentState();
	
	//判定対象のStateタグを準備
	FGameplayTag TagPassive = FGameplayTag::RequestGameplayTag("State.Passive");
	FGameplayTag TagInvestigating = FGameplayTag::RequestGameplayTag("State.Investigating");
	FGameplayTag TagSeeking = FGameplayTag::RequestGameplayTag("State.Seeking");
	
	if (CurrentState.MatchesTagExact(TagPassive) || CurrentState.MatchesTagExact(TagInvestigating) || CurrentState.MatchesTagExact(TagSeeking))
	{
		SetStateAsAttacking(Actor);
	}
}

// Called every frame
void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

