#include "PlatformGridComponent.h"
#include "BuildableBase.h"
#include "DrawDebugHelpers.h"

UPlatformGridComponent::UPlatformGridComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

	MaxWidth = 5;
	Length = 7;
	CellSize = 100.f;
	InitialActiveWidth = 3;
}

void UPlatformGridComponent::BeginPlay()
{
    Super::BeginPlay();

    MaxWidth = FMath::Max(1, MaxWidth);
    Length   = FMath::Max(1, Length);
    const int32 NumCells = MaxWidth * Length;
    Cells.SetNum(NumCells);
    ColumnUnlocked.SetNum(MaxWidth);

    // 일단 전부 true로 (나중에 좌우 잠금/해제 하고 싶으면 여기서만 바꾸면 됨)
    // for (int32 Y = 0; Y < MaxWidth; ++Y)
    // {
    //     ColumnUnlocked[Y] = true;
    // }

    // 필요하면 가운데 3줄만 활성으로 만들고 나머지 false 로 막을 수도 있음:
	
    for (int32 Y = 0; Y < MaxWidth; ++Y)
    {
        ColumnUnlocked[Y] = true;
    }
    const int32 Active = FMath::Clamp(InitialActiveWidth, 1, MaxWidth);
    const int32 Center = MaxWidth / 2;
    const int32 Start  = Center - (Active - 1) / 2;
    for (int32 i = 0; i < Active; ++i)
    {
        const int32 Col = Start + i;
        if (ColumnUnlocked.IsValidIndex(Col))
        {
            ColumnUnlocked[Col] = false;
        }
    }
	//Debug Test
	SetComponentTickEnabled(true);
}

void UPlatformGridComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // **Tick마다 디버그 드로잉 함수 호출**
    TestGridDrawing();
}

void UPlatformGridComponent::TestGridDrawing()
{
    // GetAttachParent()가 없다면 실행 불가
    USceneComponent* PlatformOrigin = GetAttachParent();
    if (!PlatformOrigin || !GetWorld()) 
    {
        return;
    }

    // 그리드의 모든 셀을 순회하며 디버그 박스 그리기
    for (int32 Y = 0; Y < MaxWidth; ++Y)
    {
        // **잠금 해제된 열만 드로잉 (CanBuildAt 로직 반영)**
        if (!ColumnUnlocked.IsValidIndex(Y) || !ColumnUnlocked[Y])
			continue;

        for (int32 X = 0; X < Length; ++X)
        {
            // 1. 그리드 좌표 (X, Y)를 월드 좌표로 변환
            const FVector CenterLocation = GridToWorld(X, Y);
            
            // 2. 그리드 셀 크기 계산 (CellSize는 한 축 길이이므로, 박스 크기는 절반)
            const FVector HalfExtent(CellSize * 0.5f, CellSize * 0.5f, 5.f); 
            
            // 3. 디버그 박스 색상 설정 (점유 여부에 따라 다르게)
            const FPlatformCell& Cell = Cells[Index(X, Y)];
            const FColor DrawColor = Cell.bOccupied ? FColor::Red : FColor::Green;

            // 4. 디버그 박스 그리기
            DrawDebugBox(
                GetWorld(),
                CenterLocation,         // 박스의 중심 위치
                HalfExtent,             // 박스의 절반 크기
                PlatformOrigin->GetComponentQuat(), // 플랫폼의 회전 사용
                DrawColor,              // 색상
                false,                  // 지속 여부 (false = 한 프레임만)
                0.f,                    // 지속 시간 (Tick에서 호출 시 0.f)
                0,                      // 깊이 테스트
                2.f                     // 선 두께
            );

            // 옵션: 중앙에 그리드 좌표 표시
            /*
            DrawDebugString(
                GetWorld(),
                CenterLocation,
                FString::Printf(TEXT("(%d,%d)"), X, Y),
                nullptr,
                FColor::White,
                0.f
            );
            */
        }
    }
}

int32 UPlatformGridComponent::GetActiveWidth() const
{
    int32 Count = 0;
    for (bool b : ColumnUnlocked)
    {
        if (b) ++Count;
    }
    return Count;
}

bool UPlatformGridComponent::WorldToGrid(const FVector& WorldPos, int32& OutX, int32& OutY) const
{
	USceneComponent* PlatformOrigin = GetAttachParent();
    if (!PlatformOrigin)
        return false;

    const FVector Origin = PlatformOrigin->GetComponentLocation();
    const FRotator Rot   = PlatformOrigin->GetComponentRotation();
    const FVector Local  = Rot.UnrotateVector(WorldPos - Origin);

    const float HalfWidth = (MaxWidth - 1) * 0.5f;
    const float HalfLength = (Length - 1) * 0.5f;

    const float Xf = Local.X / CellSize + HalfLength;
    const float Yf = Local.Y / CellSize + HalfWidth;

    OutX = FMath::RoundToInt(Xf);
    OutY = FMath::RoundToInt(Yf);

    return (OutX >= 0 && OutX < Length && OutY >= 0 && OutY < MaxWidth);
}

FVector UPlatformGridComponent::GridToWorld(int32 X, int32 Y) const
{
	USceneComponent* PlatformOrigin = GetAttachParent();
    if (!PlatformOrigin)
        return FVector::ZeroVector;

    const FVector Origin = PlatformOrigin->GetComponentLocation();
    const FRotator Rot   = PlatformOrigin->GetComponentRotation();

    const float HalfWidth = (MaxWidth - 1) * 0.5f;
    const float HalfLength = (Length - 1) * 0.5f;

    const float OffsetX = (X - HalfLength) * CellSize;
    const float OffsetY = (Y - HalfWidth)  * CellSize;

    FVector LocalOffset(OffsetX, OffsetY, 0.f);

    return Origin + Rot.RotateVector(LocalOffset);
}

bool UPlatformGridComponent::CanBuildAt(int32 X, int32 Y) const
{
    if (X < 0 || X >= Length || Y < 0 || Y >= MaxWidth)
        return false;

    if (!ColumnUnlocked.IsValidIndex(Y) || !ColumnUnlocked[Y])
        return false;

    const FPlatformCell& Cell = Cells[Index(X, Y)];
    return !Cell.bOccupied;
}


void UPlatformGridComponent::SetCell(int32 X, int32 Y, ABuildableBase* Actor)
{
    if (X < 0 || X >= Length || Y < 0 || Y >= MaxWidth)
        return;

    FPlatformCell& Cell = Cells[Index(X, Y)];
    Cell.bOccupied     = (Actor != nullptr);
    Cell.OccupiedActor = Actor;
}

bool UPlatformGridComponent::UnlockLeftColumn()
{
    // 왼쪽(작은 인덱스)에서부터 잠긴 열 찾기
    for (int32 Col = 0; Col < MaxWidth; ++Col)
    {
        if (!ColumnUnlocked[Col])
        {
            ColumnUnlocked[Col] = true;
            return true;
        }
    }
    return false;
}

bool UPlatformGridComponent::UnlockRightColumn()
{
    // 오른쪽(큰 인덱스)에서부터 잠긴 열 찾기
    for (int32 Col = MaxWidth - 1; Col >= 0; --Col)
    {
        if (!ColumnUnlocked[Col])
        {
            ColumnUnlocked[Col] = true;
            return true;
        }
    }
    return false;
}

float UPlatformGridComponent::GetTotalWeight() const
{
    float Total = 0.f;
    for (const FPlatformCell& Cell : Cells)
    {
        if (Cell.OccupiedActor)
        {
            Total += Cell.OccupiedActor->Weight;
        }
    }
    return Total;
}
