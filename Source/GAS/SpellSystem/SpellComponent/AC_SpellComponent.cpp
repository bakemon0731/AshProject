// Fill out your copyright notice in the Description page of Project Settings.


#include "AC_SpellComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UAC_SpellComponent::UAC_SpellComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
}

void UAC_SpellComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UAC_SpellComponent, EquippedSpellSlots);
}

void UAC_SpellComponent::InitializeKnownSpells(UAbilitySystemComponent* InASC)
{
	if (!InASC || !InASC->GetOwner() || !InASC->GetOwner()->HasAuthority())
	{
		return;
	}
	
	CachedASC = InASC;
	
	for (USpellDataAsset* Spell : KnownSpells)
	{
		if (!Spell || !Spell->AbilityClass)
		{
			continue;
		}
		
		FGameplayAbilitySpecHandle Handle = CachedASC->GiveAbility(
			FGameplayAbilitySpec(Spell->AbilityClass,1,INDEX_NONE,GetOwner())
			);
			SpellHandleMap.Add(Spell->AbilityClass,Handle);
	}
	EquippedSpellSlots.SetNum(NumSpellSlots);
}

bool UAC_SpellComponent::ActivateSpellIndex(int32 SpellIndex)
{
	if (!CachedASC || !EquippedSpellSlots.IsValidIndex(SpellIndex))
	{
		return false;
	}
	
	USpellDataAsset* Spell = EquippedSpellSlots[SpellIndex];
	if (!Spell || !Spell->AbilityClass)
	{
		// 空スロット
		return false;
	}
	
	if (const FGameplayAbilitySpecHandle* Handle = SpellHandleMap.Find(Spell->AbilityClass))
	{
		return CachedASC->TryActivateAbility(*Handle);
	}
	
	return false;
}

void UAC_SpellComponent::EquipSpellToSlot(int32 SlotIndex, USpellDataAsset* Spell)
{
	Server_EquipSpellToSlot(SlotIndex, Spell);
}

void UAC_SpellComponent::Server_EquipSpellToSlot_Implementation(int32 SlotIndex, USpellDataAsset* Spell)
{
	if (!EquippedSpellSlots.IsValidIndex(SlotIndex))
	{
		return;
	}
	if (Spell && !KnownSpells.Contains(Spell))
	{
		// 未習得の魔法は装備不可
		return;
	}
	
	// 同じ魔法が既に別スロットに入っていたら、そちらを空にする（一意装備の強制）
	if (Spell)
	{
		for (int32 i = 0; i < EquippedSpellSlots.Num(); i++)
		{
			if (i != SlotIndex && EquippedSpellSlots[i] == Spell)
			{
				EquippedSpellSlots[i] = nullptr;
			}
		}
	}
	EquippedSpellSlots[SlotIndex] = Spell;
	// サーバー自身のUIにも反映
	OnRep_EquippedSpellSlots();
}

void UAC_SpellComponent::OnRep_EquippedSpellSlots()
{
	OnEquippedSpellsChanged.Broadcast();
}

USpellDataAsset* UAC_SpellComponent::GetEquippedSpellAtSlot(int32 SlotIndex) const
{
	if (!EquippedSpellSlots.IsValidIndex(SlotIndex))
	{
		return nullptr;
	}
	return EquippedSpellSlots[SlotIndex];
}

bool UAC_SpellComponent::IsSpellEquippedInAnySlot(USpellDataAsset* Spell) const
{
	return Spell && EquippedSpellSlots.Contains(Spell);
}

void UAC_SpellComponent::Server_SetSelectedSpellIndex_Implementation(int32 NewIndex)
{
	SelectedSpellIndex = NewIndex;
}

// Called when the game starts
void UAC_SpellComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void UAC_SpellComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

