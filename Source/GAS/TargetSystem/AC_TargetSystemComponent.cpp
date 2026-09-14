// Fill out your copyright notice in the Description page of Project Settings.

#include "AC_TargetSystemComponent.h"
#include "EngineUtils.h"
#include "GAS/GAS.h"
#include "GAS/Interface/TargetSystemTargetable.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// コンストラクタ：コンポーネントが生成されたときの初期設定を行います
UAC_TargetSystemComponent::UAC_TargetSystemComponent()
{
    // 毎フレーム TickComponent を呼び出すように設定（カメラ追従や視線判定に必要）
    PrimaryComponentTick.bCanEverTick = true;

    // ロックオン時に表示するウィジェットのデフォルトクラスや検索対象をロード・設定
    TargetableActor = APawn::StaticClass();
    TargetableCollisionChannel = ECollisionChannel::ECC_Pawn;
}

// ゲーム開始時（BeginPlay）の処理
void UAC_TargetSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // このコンポーネントを所持しているアクター（Owner）を取得
    OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogTargetSystem, Error, TEXT("[%s] AC_TargetSystemComponent: Ownerの取得に失敗しました"), *GetName());
        return;
    }

    // Pawnとしてキャスト（Pawn以外にアタッチされている場合はエラー）
    OwnerPawn = Cast<APawn>(OwnerActor);
    if (!ensure(OwnerPawn))
    {
        UE_LOG(LogTargetSystem, Error, TEXT("[%s] AC_TargetSystemComponent: Pawn以外にアタッチされています"), *GetName());
        return;
    }

    // プレイヤーコントローラーのキャッシュ初期化
    SetupLocalPlayerController();
}

// 毎フレーム呼ばれる更新処理
void UAC_TargetSystemComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ロックオンしていない、またはターゲットが存在しない場合は何もしない
    if (!bTargetLocked || !LockedOnTargetActor)
    {
        return;
    }

    // ターゲットがそもそもターゲット可能な状態か（死亡していないか等）を毎フレーム確認
    if (!TargetIsTargetable(LockedOnTargetActor))
    {
        TargetLockOff();
        return;
    }

    // カメラやコントローラーの向きをターゲットに向ける処理
    SetControlRotationOnTarget(LockedOnTargetActor);

    // ターゲットとの距離が離れすぎていたらロック解除
    if (GetDistanceFromCharacter(LockedOnTargetActor) > MinimumDistanceToEnable)
    {
        TargetLockOff();
    }

    // 障害物によって視線が遮られた場合の処理
    if (ShouldBreakLineOfSight() && !bIsBreakingLineOfSight)
    {
        if (BreakLineOfSightDelay <= 0)
        {
            TargetLockOff();
        }
        else
        {
            bIsBreakingLineOfSight = true;
            // 一定時間（Delay）遮られ続けた場合にロックを解除するタイマーをセット
            GetWorld()->GetTimerManager().SetTimer(
                LineOfSightBreakTimerHandle,
                this,
                &UAC_TargetSystemComponent::BreakLineOfSight,
                BreakLineOfSightDelay
            );
        }
    }
}

// .cpp
void UAC_TargetSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (TargetLockedOnWidgetComponent)
    {
        TargetLockedOnWidgetComponent->DestroyComponent();
        TargetLockedOnWidgetComponent = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

// ターゲットボタンが押されたときのメイン処理（ロックオン切り替え）
void UAC_TargetSystemComponent::TargetActor()
{
    ClosestTargetDistance = MinimumDistanceToEnable;

    if (bTargetLocked)
    {
        // すでにロック中なら解除
        TargetLockOff();
    }
    else
    {
        // 非ロック中なら周辺のターゲット可能なアクターを全取得して一番近いものを狙う
        const TArray<AActor*> Actors = GetAllActorsOfClass(TargetableActor);
        LockedOnTargetActor = FindNearestTarget(Actors);
        TargetLockOn(LockedOnTargetActor);
    }
}

// スティック入力やマウス移動によるターゲット切り替え処理
void UAC_TargetSystemComponent::TargetActorWithAxisInput(const float AxisValue)
{
    if (!bTargetLocked || !LockedOnTargetActor)
    {
        return;
    }

    // 切り替えの入力閾値に達しているかチェック
    if (!ShouldSwitchTargetActor(AxisValue))
    {
        return;
    }

    // 連続切り替えを防ぐクールダウン中なら何もしない
    if (bIsSwitchingTarget)
    {
        return;
    }

    AActor* CurrentTarget = LockedOnTargetActor;

    // 入力値がマイナスなら左側、プラスなら右側の範囲（角度）で次のターゲットを探す
    const float RangeMin = AxisValue < 0 ? 0 : 180;
    const float RangeMax = AxisValue < 0 ? 180 : 360;

    ClosestTargetDistance = MinimumDistanceToEnable;

    TArray<AActor*> Actors = GetAllActorsOfClass(TargetableActor);
    TArray<AActor*> ActorsToLook;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(CurrentTarget);

    // 視線が通るかつ画面内にいるアクターを候補リストに追加
    for (AActor* Actor : Actors)
    {
        const bool bHit = LineTraceForActor(Actor, ActorsToIgnore);
        if (bHit && IsInViewport(Actor))
        {
            ActorsToLook.Add(Actor);
        }
    }

    // 指定された左右の角度範囲内にいるターゲットを抽出
    TArray<AActor*> TargetsInRange = FindTargetsInRange(ActorsToLook, RangeMin, RangeMax);

    // 範囲内の候補から、現在のターゲットにより近いものを探す
    AActor* ActorToTarget = nullptr;
    for (AActor* Actor : TargetsInRange)
    {
        const float Distance = GetDistanceFromCharacter(Actor);
        if (Distance < MinimumDistanceToEnable)
        {
            const float RelativeActorsDistance = CurrentTarget->GetDistanceTo(Actor);
            if (RelativeActorsDistance < ClosestTargetDistance)
            {
                ClosestTargetDistance = RelativeActorsDistance;
                ActorToTarget = Actor;
            }
        }
    }

    // 切り替え先のターゲットが見つかった場合
    if (ActorToTarget)
    {
        if (SwitchingTargetTimerHandle.IsValid())
        {
            SwitchingTargetTimerHandle.Invalidate();
        }

        TargetLockOff();
        LockedOnTargetActor = ActorToTarget;
        TargetLockOn(ActorToTarget);

        // 連続切り替え防止用のタイマーを設定
        GetWorld()->GetTimerManager().SetTimer(
            SwitchingTargetTimerHandle,
            this,
            &UAC_TargetSystemComponent::ResetIsSwitchingTarget,
            bIsSwitchingTarget ? 0.25f : 0.5f
        );

        bIsSwitchingTarget = true;
    }
}

bool UAC_TargetSystemComponent::GetTargetLockedStatus()
{
    return bTargetLocked;
}

bool UAC_TargetSystemComponent::IsLocked() const
{
    return bTargetLocked && LockedOnTargetActor;
}

AActor* UAC_TargetSystemComponent::GetLockedOnTargetActor() const
{
    return LockedOnTargetActor;
}

// 視野角（カメラ基準）をもとに、指定された角度範囲内にいるアクターをフィルタリングする
TArray<AActor*> UAC_TargetSystemComponent::FindTargetsInRange(TArray<AActor*> ActorsToLook, const float RangeMin, const float RangeMax) const
{
    TArray<AActor*> ActorsInRange;

    for (AActor* Actor : ActorsToLook)
    {
        const float Angle = GetAngleUsingCameraRotation(Actor);
        if (Angle > RangeMin && Angle < RangeMax)
        {
            ActorsInRange.Add(Actor);
        }
    }

    return ActorsInRange;
}

// カメラの向きを基準にして、ターゲットがプレイヤーから見てどの角度にあるかを計算する
float UAC_TargetSystemComponent::GetAngleUsingCameraRotation(const AActor* ActorToLook) const
{
    UCameraComponent* CameraComponent = OwnerActor->FindComponentByClass<UCameraComponent>();
    if (!CameraComponent)
    {
        return GetAngleUsingCharacterRotation(ActorToLook);
    }

    const FRotator CameraWorldRotation = CameraComponent->GetComponentRotation();
    const FRotator LookAtRotation = FindLookAtRotation(CameraComponent->GetComponentLocation(), ActorToLook->GetActorLocation());

    float YawAngle = CameraWorldRotation.Yaw - LookAtRotation.Yaw;
    if (YawAngle < 0)
    {
        YawAngle = YawAngle + 360;
    }

    return YawAngle;
}

// キャラクターの向きを基準にした角度計算（カメラがない場合のフォールバック）
float UAC_TargetSystemComponent::GetAngleUsingCharacterRotation(const AActor* ActorToLook) const
{
    const FRotator CharacterRotation = OwnerActor->GetActorRotation();
    const FRotator LookAtRotation = FindLookAtRotation(OwnerActor->GetActorLocation(), ActorToLook->GetActorLocation());

    float YawAngle = CharacterRotation.Yaw - LookAtRotation.Yaw;
    if (YawAngle < 0)
    {
        YawAngle = YawAngle + 360;
    }

    return YawAngle;
}

// 始点から終点を向く回転（Rotator）を算出する数学的ヘルパー
FRotator UAC_TargetSystemComponent::FindLookAtRotation(const FVector Start, const FVector Target)
{
    return FRotationMatrix::MakeFromX(Target - Start).Rotator();
}

void UAC_TargetSystemComponent::ResetIsSwitchingTarget()
{
    bIsSwitchingTarget = false;
    bDesireToSwitch = false;
}

// スティック入力によるターゲット切り替えの遊び（スレッショルド）を計算する
bool UAC_TargetSystemComponent::ShouldSwitchTargetActor(const float AxisValue)
{
    if (bEnableStickyTarget)
    {
        StartRotatingStack += (AxisValue != 0) ? AxisValue * AxisMultiplier : (StartRotatingStack > 0 ? -AxisMultiplier : AxisMultiplier);

        if (AxisValue == 0 && FMath::Abs(StartRotatingStack) <= AxisMultiplier)
        {
            StartRotatingStack = 0.0f;
        }

        if (FMath::Abs(StartRotatingStack) < StickyRotationThreshold)
        {
            bDesireToSwitch = false;
            return false;
        }

        if (StartRotatingStack * AxisValue > 0)
        {
            StartRotatingStack = StartRotatingStack > 0 ? StickyRotationThreshold : -StickyRotationThreshold;
        }
        else if (StartRotatingStack * AxisValue < 0)
        {
            StartRotatingStack = StartRotatingStack * -1.0f;
        }

        bDesireToSwitch = true;
        return true;
    }

    return FMath::Abs(AxisValue) > StartRotatingThreshold;
}

// ロックオンを実際に有効化する内部処理
void UAC_TargetSystemComponent::TargetLockOn(AActor* TargetToLockOn)
{
    if (!IsValid(TargetToLockOn))
    {
        return;
    }

    SetupLocalPlayerController();

    bTargetLocked = true;
    if (bShouldDrawLockedOnWidget)
    {
        CreateAndAttachTargetLockedOnWidgetComponent(TargetToLockOn);
    }

    if (bShouldControlRotation)
    {
        ControlRotation(true);
    }

    if (bAdjustPitchBaseOnDistanceToTarget || bIgnoreLookInput)
    {
        if (IsValid(OwnerPlayerController))
        {
            OwnerPlayerController->SetIgnoreLookInput(true);
        }
    }

    if (OnTargetLockOn.IsBound())
    {
        OnTargetLockOn.Broadcast(TargetToLockOn);
    }
}

// ロックオンを解除する内部処理
void UAC_TargetSystemComponent::TargetLockOff()
{
    SetupLocalPlayerController();

    bTargetLocked = false;

    if (IsValid(TargetLockedOnWidgetComponent))
    {
        // Destroyせず、非表示にしてデタッチするだけ
        TargetLockedOnWidgetComponent->SetVisibility(false);
        TargetLockedOnWidgetComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
    }

    if (LockedOnTargetActor)
    {
        if (bShouldControlRotation)
        {
            ControlRotation(false);
        }

        if (IsValid(OwnerPlayerController))
        {
            OwnerPlayerController->ResetIgnoreLookInput();
        }

        if (OnTargetLockOff.IsBound())
        {
            OnTargetLockOff.Broadcast(LockedOnTargetActor);
        }
    }

    LockedOnTargetActor = nullptr;
}

// ターゲットの頭上にロックオンカーソル（UIウィジェット）を生成してアタッチする
void UAC_TargetSystemComponent::CreateAndAttachTargetLockedOnWidgetComponent(AActor* TargetActor)
{
    if (!LockedOnWidgetClass)
    {
        UE_LOG(LogTargetSystem, Error, TEXT("AC_TargetSystemComponent: LockedOnWidgetClassが設定されていません。"));
        return;
    }

    UMeshComponent* MeshComponent = TargetActor->FindComponentByClass<UMeshComponent>();
    USceneComponent* ParentComponent = MeshComponent && LockedOnWidgetParentSocket != NAME_None ? MeshComponent : TargetActor->GetRootComponent();

    // 初回のみ生成。Outerはプレイヤー(OwnerActor)にして、敵のDestroyに巻き込まれないようにする
    if (!IsValid(TargetLockedOnWidgetComponent))
    {
        TargetLockedOnWidgetComponent = NewObject<UWidgetComponent>(OwnerActor, MakeUniqueObjectName(OwnerActor, UWidgetComponent::StaticClass(), FName("TargetLockOn")));
        TargetLockedOnWidgetComponent->SetWidgetClass(LockedOnWidgetClass);
        TargetLockedOnWidgetComponent->ComponentTags.Add(FName("TargetSystem.LockOnWidget"));
        TargetLockedOnWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
        TargetLockedOnWidgetComponent->SetDrawSize(FVector2D(LockedOnWidgetDrawSize, LockedOnWidgetDrawSize));

        if (IsValid(OwnerPlayerController))
        {
            TargetLockedOnWidgetComponent->SetOwnerPlayer(OwnerPlayerController->GetLocalPlayer());
        }

        TargetLockedOnWidgetComponent->RegisterComponent();
    }

    // 2回目以降はアタッチ先を新しいターゲットに付け替えるだけ
    TargetLockedOnWidgetComponent->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepRelativeTransform, LockedOnWidgetParentSocket);
    TargetLockedOnWidgetComponent->SetRelativeLocation(LockedOnWidgetLocation);
    TargetLockedOnWidgetComponent->SetVisibility(true);
}

// 指定されたクラスのアクターをワールド内からすべて収集し、ターゲット可能か判定して返す
TArray<AActor*> UAC_TargetSystemComponent::GetAllActorsOfClass(const TSubclassOf<AActor> ActorClass) const
{
    TArray<AActor*> Actors;
    for (TActorIterator<AActor> ActorIterator(GetWorld(), ActorClass); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (TargetIsTargetable(Actor))
        {
            Actors.Add(Actor);
        }
    }

    return Actors;
}

// 対象のアクターがインターフェースを実装しており、かつ IsTargetable が true を返すか確認する
bool UAC_TargetSystemComponent::TargetIsTargetable(const AActor* Actor)
{
    const bool IsTargetable = Actor->GetClass()->ImplementsInterface(UTargetSystemTargetable::StaticClass());
    if (IsTargetable)
    {
        return ITargetSystemTargetable::Execute_IsTargetable(Actor);
    }

    return true;
}

void UAC_TargetSystemComponent::SetupLocalPlayerController()
{
    if (!IsValid(OwnerPawn))
    {
        return;
    }

    OwnerPlayerController = Cast<APlayerController>(OwnerPawn->GetController());
}

// 候補のアクターの中から、障害物に遮られておらず、プレイヤーに最も近い敵を探す
AActor* UAC_TargetSystemComponent::FindNearestTarget(TArray<AActor*> Actors) const
{
    TArray<AActor*> ActorsHit;

    for (AActor* Actor : Actors)
    {
        TArray<AActor*> ActorsToIgnore;
        const bool bHit = LineTraceForActor(Actor, ActorsToIgnore);
        if (bHit && IsInViewport(Actor))
        {
            ActorsHit.Add(Actor);
        }
    }

    if (ActorsHit.Num() == 0)
    {
        return nullptr;
    }

    float ClosestDistance = ClosestTargetDistance;
    AActor* Target = nullptr;
    for (AActor* Actor : ActorsHit)
    {
        const float Distance = GetDistanceFromCharacter(Actor);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            Target = Actor;
        }
    }

    return Target;
}

bool UAC_TargetSystemComponent::LineTraceForActor(const AActor* OtherActor, const TArray<AActor*>& ActorsToIgnore) const
{
    FHitResult HitResult;
    const bool bHit = LineTrace(HitResult, OtherActor, ActorsToIgnore);
    if (bHit)
    {
        const AActor* HitActor = HitResult.GetActor();
        if (HitActor == OtherActor)
        {
            return true;
        }
    }

    return false;
}

// プレイヤーからターゲットへ向けてライントレース（レイキャスト）を実行する
bool UAC_TargetSystemComponent::LineTrace(FHitResult& OutHitResult, const AActor* OtherActor, const TArray<AActor*>& ActorsToIgnore) const
{
    if (!IsValid(OwnerActor) || !IsValid(OtherActor))
    {
        return false;
    }

    TArray<AActor*> IgnoredActors;
    IgnoredActors.Reserve(ActorsToIgnore.Num() + 1);
    IgnoredActors.Add(OwnerActor);
    IgnoredActors.Append(ActorsToIgnore);

    FCollisionQueryParams Params = FCollisionQueryParams(FName("LineTraceSingle"));
    Params.AddIgnoredActors(IgnoredActors);

    if (const UWorld* World = GetWorld(); IsValid(World))
    {
        return World->LineTraceSingleByChannel(
            OutHitResult,
            OwnerActor->GetActorLocation(),
            OtherActor->GetActorLocation(),
            TargetableCollisionChannel,
            Params
        );
    }

    return false;
}

// ロックオン中のカメラ回転角度（Pitch/Yaw）を滑らかに計算する
FRotator UAC_TargetSystemComponent::GetControlRotationOnTarget(const AActor* OtherActor) const
{
    if (!IsValid(OwnerPlayerController))
    {
        return FRotator::ZeroRotator;
    }

    const FRotator ControlRotation = OwnerPlayerController->GetControlRotation();
    const FVector CharacterLocation = OwnerActor->GetActorLocation();
    const FVector OtherActorLocation = OtherActor->GetActorLocation();

    const FRotator LookRotation = FRotationMatrix::MakeFromX(OtherActorLocation - CharacterLocation).Rotator();
    float Pitch = LookRotation.Pitch;
    FRotator TargetRotation;

    // 距離に応じてカメラの上下角度（Pitch）を自動調整する補正
    if (bAdjustPitchBaseOnDistanceToTarget)
    {
        const float DistanceToTarget = GetDistanceFromCharacter(OtherActor);
        const float PitchInRange = (DistanceToTarget * MonitorDistanceCoefficient + MonitorDistanceOffSet) * -1.0f;
        const float PitchOffset = FMath::Clamp(PitchInRange, MonitorMin, MonitorMax);

        Pitch = Pitch + PitchOffset;
        TargetRotation = FRotator(Pitch, LookRotation.Yaw, ControlRotation.Roll);
    }
    else
    {
        if (bIgnoreLookInput)
        {
            TargetRotation = FRotator(Pitch, LookRotation.Yaw, ControlRotation.Roll);
        }
        else
        {
            TargetRotation = FRotator(ControlRotation.Pitch, LookRotation.Yaw, ControlRotation.Roll);
        }
    }

    // 現在の回転からターゲットの回転へ滑らかに補間する（RInterpTo）
    return FMath::RInterpTo(ControlRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 9.0f);
}

void UAC_TargetSystemComponent::SetControlRotationOnTarget(AActor* TargetActor) const
{
    if (!IsValid(OwnerPlayerController))
    {
        return;
    }

    const FRotator ControlRotation = GetControlRotationOnTarget(TargetActor);
    if (OnTargetSetRotation.IsBound())
    {
        OnTargetSetRotation.Broadcast(TargetActor, ControlRotation);
    }
    else
    {
        OwnerPlayerController->SetControlRotation(ControlRotation);
    }
}

float UAC_TargetSystemComponent::GetDistanceFromCharacter(const AActor* OtherActor) const
{
    return OwnerActor->GetDistanceTo(OtherActor);
}

// 視線（ライントレース）が遮られているかどうかを判定する
bool UAC_TargetSystemComponent::ShouldBreakLineOfSight() const
{
    if (!LockedOnTargetActor)
    {
        return true;
    }

    // 全アクター走査は不要。ターゲット自身への視線だけ。
    TArray<AActor*> ActorsToIgnore;

    FHitResult HitResult;
    const bool bHit = LineTrace(HitResult, LockedOnTargetActor, ActorsToIgnore);
    if (bHit && HitResult.GetActor() != LockedOnTargetActor)
    {
        return true;
    }

    return false;
}

void UAC_TargetSystemComponent::BreakLineOfSight()
{
    bIsBreakingLineOfSight = false;
    if (ShouldBreakLineOfSight())
    {
        TargetLockOff();
    }
}

// キャラクターの移動方向への自動回転を切り替える
void UAC_TargetSystemComponent::ControlRotation(const bool ShouldControlRotation) const
{
    if (!IsValid(OwnerPawn))
    {
        return;
    }

    OwnerPawn->bUseControllerRotationYaw = ShouldControlRotation;

    UCharacterMovementComponent* CharacterMovementComponent = OwnerPawn->FindComponentByClass<UCharacterMovementComponent>();
    if (CharacterMovementComponent)
    {
        CharacterMovementComponent->bOrientRotationToMovement = !ShouldControlRotation;
    }
}

// 対象のアクターが画面（ビューポート）の内側に映っているかを判定する
bool UAC_TargetSystemComponent::IsInViewport(const AActor* TargetActor) const
{
    if (!IsValid(OwnerPlayerController))
    {
        return true;
    }

    FVector2D ScreenLocation;
    OwnerPlayerController->ProjectWorldLocationToScreen(TargetActor->GetActorLocation(), ScreenLocation);

    FVector2D ViewportSize;
    GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);

    return ScreenLocation.X > 0 && ScreenLocation.Y > 0 && ScreenLocation.X < ViewportSize.X && ScreenLocation.Y < ViewportSize.Y;
}