// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "GAS/GamePlayAbilitySystem/Characters/NexusCharacterBase.h"// キャラクタークラスのヘッダ
#include "GAS/WeaponBase/WeaponBase.h"// 武器クラスのヘッダ

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

//GiveWeaponイベント
void UWeaponComponent::GiveWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	// 必要な参照が揃っているか安全チェック
	if (!WeaponClass || !GetWorld() || !OwnerCharacter)
	{
		return;
	}
	
	// 1. SpawnActor (生成パラメータの設定)
	FActorSpawnParameters SpawnParames;
	SpawnParames.Instigator = OwnerCharacter;// ダメージ発生源として設定
	AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(
		WeaponClass, 
		FVector::ZeroVector,
		FRotator::ZeroRotator
		);// 武器アクタをスポーン
	
	
	//NewWeapon（AWeaponBase）が有効か確認
	if (NewWeapon)
	{
		// 2. Add Unique (StowedWeapons配列へ追加)
		StowedWeapons.AddUnique(NewWeapon);
		
		// 3. Attach Actor To Component (背中ソケットへの取り付け)
		if (USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh())
		{
			// BPの Keep Relative, Weld Simulated Bodies に相当する設定
			FAttachmentTransformRules AttachmentRules(
				EAttachmentRule::KeepRelative,
				EAttachmentRule::KeepRelative,
				EAttachmentRule::KeepRelative,
				true // bWeldSimulatedBodies = true
				);
			//FName SocketName = NewWeapon->WeaponConfig.StowedSocketName;
			//NewWeapon->AttachToComponent(CharacterMesh, AttachmentRules, SocketName);
		}
	}
		
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 1. Get Owner & Cast To NexusCharacterBase
	OwnerCharacter = Cast<ANexusCharacterBase>(GetOwner());

	// 2. Switch Has Authority (サーバー権限がある場合のみ実行)
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		// 3. For Each Loop (StartingWeapons の要素を1つずつ処理)
		for (TSubclassOf<AWeaponBase> WeaponClass : StartingWeapons)
		{
			// 配列の中にちゃんとクラスが設定されている場合のみ処理する
			if (WeaponClass)
			{   
				//GiveWeapon関数呼び出し。
				GiveWeapon(WeaponClass);
			}
		}
	}	
	
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

