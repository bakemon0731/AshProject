// Fill out your copyright notice in the Description page of Project Settings.


#include "BTD_StateGameplayTag.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"

UBTD_StateGameplayTag::UBTD_StateGameplayTag()
{
	NodeName = "StateGameplayTag_CPP";
	
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

//バインド実装処理
void UBTD_StateGameplayTag::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    
    FNodeMemory* Memory = reinterpret_cast<FNodeMemory*>(NodeMemory);
    Memory->CachedBTComp = &OwnerComp;

    UAbilitySystemComponent* ASC = GetASCFromOwnerComp(OwnerComp);
    if (!ASC) return;

    Memory->CachedASC = ASC;
    
    TWeakObjectPtr<UBehaviorTreeComponent> BTCompWeak = &OwnerComp;
    //StateTagがついたり外れたりしたらASCに教える。（予約）
    Memory->TagDelegateHandle = ASC->RegisterGameplayTagEvent(
        StateTag,
        EGameplayTagEventType::NewOrRemoved
      //予約が発火したとき、HandleTagChanged関数を呼び出す。（コールバック関数を登録）  
    ).AddUObject(this, &UBTD_StateGameplayTag::HandleTagChanged, BTCompWeak);
    
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
    
    if (UBehaviorTreeComponent* BTComp = BTCompWeak.Get())
    {
        // 変化した瞬間だけ再評価をリクエスト（＝Tickポーリング不要）
        BTComp->RequestExecution(this);
    }
}

bool UBTD_StateGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    
    UAbilitySystemComponent* ASC = GetASCFromOwnerComp(OwnerComp);
    if (!ASC) return false;

    return ASC->HasMatchingGameplayTag(StateTag);
}

FString UBTD_StateGameplayTag::GetStaticDescription() const
{
    return FString::Printf(TEXT("State Tag: %s"), *StateTag.ToString());
}
