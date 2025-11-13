#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GroundFollowComponent.generated.h"

UCLASS(ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent))
class STEELSURVIVOR_API UGroundFollowComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGroundFollowComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 지면 추종 사용 여부 (물리 주행과 충돌 방지를 위해 토글 가능) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    bool bEnableGroundFollow = true;

    /** 차 바닥과 지면 간 유지 간격 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    float RideHeight = 20.f;

    /** 위/아래 트레이스 길이 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    float TraceUp = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    float TraceDown = 200.f;

    /** 보정 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    float ZInterpSpeed = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    float RotInterpSpeed = 6.f;

    /** 올라갈 수 있는 최대 경사 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    float MaxWalkableSlopeDeg = 50.f;

    /** 지면 노멀에 회전 정렬할지 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    bool bAlignToGroundNormal = true;

    /** 물리 주행 여부(물리 사용 시 이 컴포넌트는 자동 비활성 권장) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GroundFollow")
    bool bPhysicsDriveMode = false;

private:
    void ApplyGroundFollow(float DeltaTime);

    /** 루트 Primitive 캐싱 */
    TWeakObjectPtr<class UPrimitiveComponent> RootPrim;
};
