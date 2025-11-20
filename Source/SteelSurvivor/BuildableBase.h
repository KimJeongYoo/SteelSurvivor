#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildableBase.generated.h"

UCLASS()
class STEELSURVIVOR_API ABuildableBase : public AActor
{
    GENERATED_BODY()

public:
    ABuildableBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Build")
    float Durability = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Build")
    float Weight = 10.f;
};
