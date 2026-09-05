// Fill out your copyright notice in the Description page of Project Settings.


#include "BTD_StateGameplayTag.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"

UBTD_StateGameplayTag::UBTD_StateGameplayTag()
{
	NodeName = "StateGameplayTag_CPP";
    
    
    //UE_LOG(LogTemp, Warning, TEXT("UBTD_StateGameplayTag Constructor called"));

	
	// Observer Aborts: Self 相当。BP版と同じ挙動にする
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
	FlowAbortMode = EBTFlowAbortMode::Self;
	
	//Tickでのポーリングを行わない
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = false;
}

UAbilitySystemComponent* UBTD_StateGameplayTag::GetASCFromOwnerComp(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return nullptr;

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn) return nullptr;

    return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
}

void UBTD_StateGameplayTag::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    FNodeMemory* Memory = reinterpret_cast<FNodeMemory*>(NodeMemory);
    new (Memory) FNodeMemory();
}

void UBTD_StateGameplayTag::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    FNodeMemory* Memory = reinterpret_cast<FNodeMemory*>(NodeMemory);
    Memory->~FNodeMemory();
}

void UBTD_StateGameplayTag::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    //UE_LOG(LogTemp, Warning, TEXT("OnBecomeRelevant called for %s"), *StateTag.ToString());

    
    FNodeMemory* Memory = reinterpret_cast<FNodeMemory*>(NodeMemory);
    Memory->CachedBTComp = &OwnerComp;

    UAbilitySystemComponent* ASC = GetASCFromOwnerComp(OwnerComp);
    if (!ASC) return;

    Memory->CachedASC = ASC;

    // タグ変化イベントを購読。以後Tick不要
    TWeakObjectPtr<UBehaviorTreeComponent> BTCompWeak = &OwnerComp;
    Memory->TagDelegateHandle = ASC->RegisterGameplayTagEvent(
        StateTag,
        EGameplayTagEventType::NewOrRemoved
    ).AddUObject(this, &UBTD_StateGameplayTag::HandleTagChanged, BTCompWeak);
    
    //UE_LOG(LogTemp, Warning, TEXT("Tag event registered for %s"), *StateTag.ToString());
}

void UBTD_StateGameplayTag::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FNodeMemory* Memory = reinterpret_cast<FNodeMemory*>(NodeMemory);

    if (UAbilitySystemComponent* ASC = Memory->CachedASC.Get())
    {
        ASC->RegisterGameplayTagEvent(StateTag, EGameplayTagEventType::NewOrRemoved)
            .Remove(Memory->TagDelegateHandle);
    }

    Memory->TagDelegateHandle.Reset();
    Memory->CachedASC.Reset();
    Memory->CachedBTComp.Reset();
}

void UBTD_StateGameplayTag::HandleTagChanged(FGameplayTag Tag, int32 NewCount, TWeakObjectPtr<UBehaviorTreeComponent> BTCompWeak)
{
    
    //SCOPE_CYCLE_COUNTER(STAT_BTD_StateGameplayTag_TagChanged);
    //INC_DWORD_STAT(STAT_BTD_StateGameplayTag_TagChangedCount);
    
    //TagChangedCallCount++;
    //UE_LOG(LogTemp, Warning, TEXT("[%s] TagChanged detected. Total: %d"), *StateTag.ToString(), TagChangedCallCount);
    
    if (UBehaviorTreeComponent* BTComp = BTCompWeak.Get())
    {
        // 変化した瞬間だけ再評価をリクエスト（＝Tickポーリング不要）
        BTComp->RequestExecution(this);
    }
}

bool UBTD_StateGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
   // SCOPE_CYCLE_COUNTER(STAT_BTD_StateGameplayTag_Calculate);
    //INC_DWORD_STAT(STAT_BTD_StateGameplayTag_CalculateCount);
    
    //CalculateCallCount++;
    //UE_LOG(LogTemp, Warning, TEXT("[%s] CalculateRawConditionValue called. Total: %d"), *StateTag.ToString(), CalculateCallCount);

    
    UAbilitySystemComponent* ASC = GetASCFromOwnerComp(OwnerComp);
    if (!ASC) return false;

    return ASC->HasMatchingGameplayTag(StateTag);
}

FString UBTD_StateGameplayTag::GetStaticDescription() const
{
    return FString::Printf(TEXT("State Tag: %s"), *StateTag.ToString());
}
