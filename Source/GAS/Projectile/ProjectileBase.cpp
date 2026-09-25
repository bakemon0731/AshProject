// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "GAS/WorldSubsystem/ActorPoolSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayCueFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"


// Sets default values
AProjectileBase::AProjectileBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(16.f);
	SphereCollision->SetCollisionProfileName(TEXT("Projectile"));
	SphereCollision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnSphereHit);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnSphereBeginOverlap);
	
	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(SphereCollision);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	
	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectileBase::OnSphereHit(UPrimitiveComponent* HitComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, 
	const FHitResult& Hit
	)
{
	// PawnをBlockにした場合、敵との衝突はOverlapではなくここ(Hit)で通知される。
	// 壁などASCを持たないアクターの場合はResolveImpact内のApplyEffectsToTargetが安全に何もしない。
	ResolveImpact(OtherActor);
}

void AProjectileBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// コリジョンレスポンスをOverlapにしているターゲット(将来的な仕様変更等)向けに残しておく
	ResolveImpact(OtherActor);
}

void AProjectileBase::ResolveImpact(AActor* OtherActor)
{
	//サーバーの権威でのみ実行する。
	if (!HasAuthority())
	{
		return;
	}
	
	// Hit/Overlap両方から呼ばれる可能性があるため、一度衝突処理をしたら以降は無視する
	if (bHasImpacted)
	{
		return;
	}
 
	// IgnoreActorWhenMovingで基本的には弾かれるはずだが、念のための保険
	if (!IsValid(OtherActor) || OtherActor == ProjectileInstigator)
	{
		return;
	}
 
	bHasImpacted = true;
 
	ApplyEffectToActor(OtherActor);
	ExecuteImpactCue();
	ReturnToPoolOrDestroy();
}

void AProjectileBase::ApplyEffectToActor(AActor* TargetActor)
{
	//TargetActorからAbilitySystemComponentを取得。
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}
	
	//GAから渡されたEffectSpecHandle(ダメージGE)を適用
	if (EffectSpecHandle.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
	
	// クラスデフォルトで設定されたデバフを別途適用
	if (DebuffToApply)
	{
		//Contextを作成。
		FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
		ContextHandle.AddInstigator(ProjectileInstigator,this);
		
		//デバフを適用。
		const FGameplayEffectSpecHandle DebuffSpec = TargetASC->MakeOutgoingSpec(DebuffToApply,1.f,ContextHandle);
		if (DebuffSpec.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DebuffSpec.Data.Get());
		}
	}
}

void AProjectileBase::ExecuteImpactCue_Implementation()
{
	if (GameplayCueImpact.IsValid())
	{
		//Cueのパラメーターを設定。（MakeGameplayCueParameterノード）
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		CueParams.Instigator = ProjectileInstigator;
		
		//ExecuteGameplayCueOnActor（ノード）を実行
		UGameplayCueFunctionLibrary::ExecuteGameplayCueOnActor(this, GameplayCueImpact, CueParams);
	}
}

void AProjectileBase::ExecuteSpawnCue_Implementation()
{
	//アクティブ時（スポーン時）のゲームプレイキューを実行。
	if (GameplayCueSpawn.IsValid())
	{
		//Cueのパラメーターを設定。（MakeGameplayCueParameterノード）
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		CueParams.Instigator = ProjectileInstigator;
		
		//ExecuteGameplayCueOnActor（ノード）を実行
		UGameplayCueFunctionLibrary::ExecuteGameplayCueOnActor(this, GameplayCueSpawn, CueParams);
	}
}

void AProjectileBase::ReturnToPoolOrDestroy()
{
	// Hit/Overlapで呼ばれた場合と寿命タイマーの両方から呼ばれうるため、ここで確実にタイマーを止めて二重返却を防ぐ
	GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
	
	if (UWorld* World = GetWorld())
	{
		if (UActorPoolSubsystem* Pool = World->GetSubsystem<UActorPoolSubsystem>())
		{
			Pool->ReleaseActor(this);
			return;
		}
	}
	// Subsystemが取得できない特殊なワールド(エディタユーティリティ等)向けのフォールバック
	Destroy();
}

void AProjectileBase::OnActivatedFromPool_Implementation()
{
	// 前回使用時の状態が残らないようリセット
	EffectSpecHandle = FGameplayEffectSpecHandle();
	ProjectileInstigator = nullptr;
	Speed = 0.0f;
	TargetLocation = FVector::ZeroVector;
	bHasImpacted = false;
	
	// ここから先(見た目の有効化)はBP側の実装に委ねる
	OnPoolActivated();
}

//発射物の移動を止める。
void AProjectileBase::OnReturnToPool_Implementation()
{
	// ReturnToPoolOrDestroy経由以外でReleaseActorが呼ばれた場合の保険
	GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
	
	// 前回のInstigator無視設定を残さない(次に取り出された時に別の発射者になるため)
	SphereCollision->MoveIgnoreActors.Reset();
	
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
	
	
	// ここから先(見た目の無効化)はBP側の実装に委ねる
	OnPoolDeactivated();
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileBase::InitializeAndFire(
	float InSpeed, 
	FVector InTargetLocation,
	const FGameplayEffectSpecHandle& InEffectSpecHandle, 
	AActor* InInstigator
	)
{
	Speed = InSpeed;
	TargetLocation = InTargetLocation;
	EffectSpecHandle = InEffectSpecHandle;
	ProjectileInstigator = InInstigator;
	
	//スポーン・発射処理もサーバーの権威でのみ実行されるべきのためチェック。
	if (!HasAuthority())
	{
		return;
	}
	
	// 発射者自身には(PawnをBlockにしていても)当たらないようにする。コリジョンレスポンスは変えず、「このアクターとの衝突だけ無視する」設定を使う。
	if (ProjectileInstigator)
	{
		SphereCollision->IgnoreActorWhenMoving(ProjectileInstigator, true);
	}
	
	//Targetの位置とこのアクタの位置を引いて正規化し、まっすぐな方向を計算方向を取得。（constなので、値の書き換え不可）
	const FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
	
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = Direction * Speed;
	ProjectileMovement->Activate(true);
	
	// 何にも当たらないまま飛び続けるケース(真上に撃つ等)に備え、MaxLifetime秒後に自動でプールへ返却するタイマーをセットする
	GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
	if (MaxLifetime > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			LifeTimeTimerHandle, 
			this, 
			&AProjectileBase::ReturnToPoolOrDestroy, 
			MaxLifetime, 
			false);
	}
	// ゲームプレイキューを実行。
	ExecuteSpawnCue();
}



