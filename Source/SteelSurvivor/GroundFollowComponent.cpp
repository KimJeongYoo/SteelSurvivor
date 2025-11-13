#include "GroundFollowComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
// #include "DrawDebugHelpers.h"

UGroundFollowComponent::UGroundFollowComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGroundFollowComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
        // 루트는 충돌 가능해야 스윕이 의미 있음
        if (RootPrim.IsValid())
        {
            // 여기서 충돌 프로필/시뮬레이트 설정을 강제하지는 않음(차마다 다를 수 있음)
        }
    }
}

void UGroundFollowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableGroundFollow || bPhysicsDriveMode) return;

    // 물리 시뮬 중이면 위치/회전 보정을 하면 안 된다
    if (RootPrim.IsValid() && RootPrim->IsSimulatingPhysics()) return;

    ApplyGroundFollow(DeltaTime);
}

void UGroundFollowComponent::ApplyGroundFollow(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Up = FVector::UpVector;
    const FVector ActorLoc = Owner->GetActorLocation();
    const FVector TraceStart = ActorLoc + Up * TraceUp;
    const FVector TraceEnd   = ActorLoc - Up * TraceDown;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(GroundFollow), false, Owner);

    const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
    // DrawDebugLine(World, TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 0, 0, 1);

    if (!bHit)
    {
        // 지면 못 찾으면 천천히 하강(낙하 느낌)
        if (RootPrim.IsValid())
        {
            Owner->AddActorWorldOffset(-Up * 300.f * DeltaTime, /*bSweep=*/true);
        }
        return;
    }

    const FVector GroundNormal = Hit.ImpactNormal.GetSafeNormal();
    const float SlopeDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GroundNormal, Up)));
    if (SlopeDeg > MaxWalkableSlopeDeg)
    {
        // 너무 가파른 곳은 붙지 않음
        Owner->AddActorWorldOffset(Up * 2.f, true);
        return;
    }

    // Z 위치 보정
    const float TargetZ = Hit.ImpactPoint.Z + RideHeight;
    FVector TargetLoc = ActorLoc;
    TargetLoc.Z = FMath::FInterpTo(ActorLoc.Z, TargetZ, DeltaTime, ZInterpSpeed);

    Owner->SetActorLocation(TargetLoc, /*bSweep=*/true);

    // Pitch/Roll 정렬
    if (bAlignToGroundNormal)
    {
        const FVector Fwd = Owner->GetActorForwardVector();
        FVector FwdOnPlane = (Fwd - FVector::DotProduct(Fwd, GroundNormal) * GroundNormal).GetSafeNormal();
        if (FwdOnPlane.IsNearlyZero())
        {
            FwdOnPlane = Fwd;
        }

        const FRotator Desired = FRotationMatrix::MakeFromXZ(FwdOnPlane, GroundNormal).Rotator();
        FRotator Current = Owner->GetActorRotation();
        FRotator Target  = FRotator(Desired.Pitch, Current.Yaw, Desired.Roll);

        const FRotator NewRot = FMath::RInterpTo(Current, Target, DeltaTime, RotInterpSpeed);
        Owner->SetActorRotation(NewRot);
    }
}
