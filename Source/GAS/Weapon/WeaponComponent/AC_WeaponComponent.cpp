// Fill out your copyright notice in the Description page of Project Settings.


#include "AC_WeaponComponent.h"
#include "GAS/GameplayAbilitySystem/Characters/NexusCharacterBase.h"
#include "GAS/Weapon/WeaponBase/WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"


UAC_WeaponComponent::UAC_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// コンポーネントのレプリケーションを有効にする
	SetIsReplicatedByDefault(true);


}

void UAC_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//EquippedWeapon変数をレプリケーション対象に登録。
	DOREPLIFETIME(UAC_WeaponComponent,EquippedWeapon);
	//PreviouslyEquippedWeapon変数をレプリケーション対象に登録。
	DOREPLIFETIME(UAC_WeaponComponent,PreviouslyEquippedWeapon);
	
}

void UAC_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Ownerを取得し、NexusCharacterBaseにキャストして保存
	OwningCharacter = Cast<ANexusCharacterBase>(GetOwner());
	
	// Authority(サーバー)でのみ実行
	if (OwningCharacter && OwningCharacter -> HasAuthority())
	{
		// StartingWeapons配列をループして武器を付与
		for (TSubclassOf<AWeaponBase> WeaponClass : StartingWeapons)
		{
			if (WeaponClass)
			{
				//GiveWeapon関数を実行
				GiveWeapon(WeaponClass);
			}
		}
	}
}

void UAC_WeaponComponent::OnRep_EquippedWeapon()
{
	// サーバーの武器の見た目をクライアントに反映させる処理
	if (IsValid(EquippedWeapon))
	{
		SetEquippedWeaponProperties();
	}
	else
	{
		SetUnarmedWeaponConfig();
	}
}

void UAC_WeaponComponent::GiveWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	
	UWorld* World = GetWorld();
	if (!OwningCharacter ||	!WeaponClass || !World)
	{
		return;
	}
	
	// Spawn Actor 用のパラメータ設定
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = OwningCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	//スポーンアクター
	AWeaponBase* SpawnWeapon = World->SpawnActor<AWeaponBase>(
		WeaponClass,FVector::ZeroVector,FRotator::ZeroRotator,SpawnParameters);
	
	if (SpawnWeapon)
	{
		// StowedWeapons配列にユニーク追加
		StowedWeapons.AddUnique(SpawnWeapon);
		
		// 武器のコンフィグからアタッチ用のソケット名を取得
		FName SocketName = SpawnWeapon->WeaponConfig.StowedSocketName;
		
		// OwningCharacterのメッシュを取得してアタッチ
		if (USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh())
		{
			// Location/Rotation/Scale は Keep Relative (相対位置を維持) を指定
			SpawnWeapon->AttachToComponent(CharacterMesh,FAttachmentTransformRules::KeepRelativeTransform,SocketName);
		}
	}
}

void UAC_WeaponComponent::EquipWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	// サーバーのみ実行 (Switch Has Authority)
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	//OwningCharacterの有効チェック
	if (!IsValid(OwningCharacter))
	{
		return;
	}
	
	// 現在の武器クラスをチェックし、同じなら終了、違うなら外す
	if (IsValid(EquippedWeapon))
	{
		if (EquippedWeapon->GetClass() == WeaponClass)
		{
			// 等しい場合は武器をすでに持っていて、武器をしまうことを示唆している。
			UnequipWeapon();
			
			return;
		}
		else
		{
			// 等しくない場合はUnequipを実行
			UnequipWeapon();
		}
	}
	
	//取り出す武器が背中にある武器と等しいか調べる。
	AWeaponBase* StowedWeapon = GetStowedWeaponByClass(WeaponClass);
	
	if (IsValid(StowedWeapon))
	{
		// 武器をセットして通知（レプリケート)
		EquippedWeapon = StowedWeapon;
		
		// 装備武器のプロパティの設定関数。
		SetEquippedWeaponProperties();
	}
}

void UAC_WeaponComponent::UnequipWeapon()
{
	// EquippedWeaponがValidならPreviouslyEquippedWeaponに保存。
	if (IsValid(EquippedWeapon))
	{
		PreviouslyEquippedWeapon = EquippedWeapon;
	}
	// EquippedWeaponを空にする。
	EquippedWeapon = nullptr;
	
	// 解除武器のプロパティの設定関数を実行。
	SetUnarmedWeaponConfig();
}

AWeaponBase* UAC_WeaponComponent::GetStowedWeaponByClass(TSubclassOf<AWeaponBase> WeaponClass)
{
	// 配列をループしてクラスが一致するものを探す
	for (AWeaponBase* Weapon : StowedWeapons)
	{
		if (IsValid(Weapon) && Weapon->GetClass() == WeaponClass)
		{
			// 一致したら値を返す
			return Weapon;
		}
	}
	// 見つからなかった場合はnullptrを返す
	return nullptr;
}

void UAC_WeaponComponent::SetEquippedWeaponProperties()
{
	// EquippedWeaponとOwningCharacterのValidチェック
	if (!IsValid(EquippedWeapon) || !IsValid(OwningCharacter))
	{
		return;
	}
	
	// 武器の親アクターから継承した構造体を取り出す (Break SWeaponConfig)
	FSWeaponConfig WeaponConfig=EquippedWeapon->WeaponConfig;
	
	// メッシュの取得
	USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		return;
	}
	
	// AnimInstanceClassを設定
	if (WeaponConfig.AnimClass)
	{
		CharacterMesh->SetAnimInstanceClass(WeaponConfig.AnimClass);
	}
	
	// 移動速度/カメラを設定 (Break SMovementProperties)
	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	if (IsValid(MovementComp))
	{
		FSMovementProperties MoveProps = WeaponConfig.MovementProperties;
		
		MovementComp->MaxWalkSpeed = MoveProps.MaxWalkSpeed;
		MovementComp->bOrientRotationToMovement = MoveProps.OrientRotationToMovement;
		MovementComp->bUseControllerDesiredRotation = MoveProps.UseControllerDesiredRotation;
	}
	
	//武器を変更したことをコール (イベントディスパッチャー)
	OnWeaponChanged.Broadcast(EquippedWeapon);
	
	//BPでPlayMontageは任せる
	if (IsValid(EquippedWeapon))
	{
		PlayEquipMontage(EquippedWeapon->WeaponConfig);
	}
}

void UAC_WeaponComponent::SetUnarmedWeaponConfig()
{
	if (!IsValid(OwningCharacter))
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh();
	
	if (!IsValid(CharacterMesh))
	{
		return;
	}
	
	// AnimInstanceClassを設定
	if (UnarmedWeaponConfig.AnimClass)
	{
		CharacterMesh->SetAnimInstanceClass(UnarmedWeaponConfig.AnimClass);
	}
	
	//BPでPlayMontageは任せる
	if (IsValid(PreviouslyEquippedWeapon))
	{
		PlayUnequipMontage(PreviouslyEquippedWeapon->WeaponConfig);
	}
}

void UAC_WeaponComponent::SetUnarmedProperties()
{
	// UnarmedWeaponConfig変数から構造体を取り出す (Break SWeaponConfig)
	FSWeaponConfig WeaponConfig = UnarmedWeaponConfig;
	
	UCharacterMovementComponent* MovementComp = OwningCharacter->GetCharacterMovement();
	if (IsValid(MovementComp))
	{
		FSMovementProperties MoveProps = WeaponConfig.MovementProperties;
		
		MovementComp->MaxWalkSpeed = MoveProps.MaxWalkSpeed;
		MovementComp->bOrientRotationToMovement = MoveProps.OrientRotationToMovement;
		MovementComp->bUseControllerDesiredRotation = MoveProps.UseControllerDesiredRotation;
	}
	
	//武器を変更したことをコール (イベントディスパッチャー)
	OnWeaponChanged.Broadcast(nullptr);
	
	IsWieldingWeapon = false;
}


void UAC_WeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



