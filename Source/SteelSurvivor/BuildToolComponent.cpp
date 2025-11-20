#include "BuildToolComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include "PlatformGridComponent.h"
#include "BuildableBase.h"
#include "PlayerVehicle.h"
#include "VehiclePawn.h"

UBuildToolComponent::UBuildToolComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UBuildToolComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBuildToolComponent::ToggleBuildMode()
{
    SetBuildMode(!bBuildMode);
}

void UBuildToolComponent::SetBuildMode(bool bEnable)
{
    bBuildMode = bEnable;

    if (!bBuildMode)
    {
        if (PreviewActor)
        {
            PreviewActor->Destroy();
            PreviewActor = nullptr;
        }
        CurrentGrid = nullptr;
        PreviewX = PreviewY = -1;
    }
}

void UBuildToolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bBuildMode)
        return;

    UpdatePreview();
}

void UBuildToolComponent::UpdatePreview()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    APawn* PawnOwner = Cast<APawn>(Owner);
    if (!PawnOwner) return;

    APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
    if (!PC) return;

    FVector ViewLoc;
    FRotator ViewRot;
    PC->GetPlayerViewPoint(ViewLoc, ViewRot);

    const FVector Start = ViewLoc;
    const FVector End   = Start + ViewRot.Vector() * TraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BuildTrace), true);
    Params.AddIgnoredActor(Owner);

    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
		//디버그
		if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("BuildTrace: no hit"));

        // 히트 안 되면 프리뷰 숨기기
        // if (PreviewActor)
        //     PreviewActor->SetActorHiddenInGame(true);
        // CurrentGrid = nullptr;
        return;
    }

    APlayerVehicle* Vehicle = Cast<APlayerVehicle>(Hit.GetActor());
    if (!Vehicle)
    {
		//디버그
		if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow,
            FString::Printf(TEXT("Hit %s but not APlayerVehicle"), *Hit.GetActor()->GetName()));

        // if (PreviewActor)
        //     PreviewActor->SetActorHiddenInGame(true);
        // CurrentGrid = nullptr;
        return;
    }

    UPlatformGridComponent* Grid = Vehicle->PlatformGrid;
    if (!Grid)
        return;

    int32 GX, GY;
    if (!Grid->WorldToGrid(Hit.ImpactPoint, GX, GY))
    {
        if (PreviewActor)
            PreviewActor->SetActorHiddenInGame(true);
        CurrentGrid = nullptr;
        return;
    }

    if (!Grid->CanBuildAt(GX, GY))
    {
        if (PreviewActor)
            PreviewActor->SetActorHiddenInGame(true);
        CurrentGrid = Grid;
        PreviewX = PreviewY = -1;
        return;
    }

    const FVector CellPos = Grid->GridToWorld(GX, GY);

    CurrentGrid = Grid;
    PreviewX = GX;
    PreviewY = GY;

    FRotator PlatformRotator = CurrentGrid->GetComponentRotation();

    if (!PreviewActor)
    {
        if (!FloorClass) return;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        PreviewActor = GetWorld()->SpawnActor<ABuildableBase>(
            FloorClass,
            CellPos,
            PlatformRotator,
            SpawnParams);

        if (PreviewActor)
        {
            PreviewActor->SetActorEnableCollision(false);
            PreviewActor->SetActorHiddenInGame(false);
        }
    }
    else
    {
        PreviewActor->SetActorLocation(CellPos);
        PreviewActor->SetActorHiddenInGame(false);
    }

    // 디버그 보기 원하면
    // DrawDebugBox(GetWorld(), CellPos, FVector(Grid->CellSize*0.5f), FColor::Green, false, 0.f, 0, 1.f);
}

void UBuildToolComponent::ConfirmBuild()
{
    if (!bBuildMode || !CurrentGrid || !FloorClass)
        return;

    if (PreviewX < 0 || PreviewY < 0)
        return;

    if (!CurrentGrid->CanBuildAt(PreviewX, PreviewY))
        return;

    const FVector SpawnLoc = CurrentGrid->GridToWorld(PreviewX, PreviewY);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FRotator PlatformRotator = CurrentGrid->GetComponentRotation();

    ABuildableBase* NewFloor = GetWorld()->SpawnActor<ABuildableBase>(
        FloorClass,
        SpawnLoc,
        PlatformRotator,
        SpawnParams);

    AVehiclePawn* Pawn = Cast<AVehiclePawn>(CurrentGrid->GetOwner());
    USkeletalMeshComponent* BodyMesh = Pawn->VehicleBodyMesh;
    NewFloor->AttachToComponent(BodyMesh, FAttachmentTransformRules::KeepWorldTransform);
    
    

    if (NewFloor)
    {
        CurrentGrid->SetCell(PreviewX, PreviewY, NewFloor);
    }
}
