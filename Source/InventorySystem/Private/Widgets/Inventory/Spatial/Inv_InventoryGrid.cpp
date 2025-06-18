// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "InventorySystem.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Widgets/Inventory/GirdSlots/Inv_GridSlot.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/ItemPopUp/Inv_ItemPopUp.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this,&ThisClass::AddItem);
	InventoryComponent->OnStackChange.AddDynamic(this,&ThisClass::AddStacks);
}

void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector2D CanvasPos = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	if (CursorExitedCanvas(CanvasPos, UInv_WidgetUtils::GetWidgetSize(CanvasPanel), MousePosition))
	{
		return;
	}
	
	UpdateTileParameters(CanvasPos,MousePosition);
}


void UInv_InventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	if (!bMouseWithinCanvas)
	{
		return;
	}
	
	const FIntPoint HoveredCoordinates = CalculateHoveredCoordinates(CanvasPosition,MousePosition);

	LastTileParameters = TileParameters;
	TileParameters.TileCoordinats = HoveredCoordinates;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(HoveredCoordinates,Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition,MousePosition);

	OnTileParametersUpdated(TileParameters);
}

void UInv_InventoryGrid::OnTileParametersUpdated(const FInv_TileParameters& Parameters)
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	// Get Hoer Item's Dimensions
	const FIntPoint Dimensions = HoverItem->GetGridDimensions();

	// Calculate the starting coordinate for highlighting
	const FIntPoint StartingCoordinate = CalculateStartingCoordinate(Parameters.TileCoordinats,Dimensions,Parameters.TileQuadrant);
	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinate,Columns);
	
	CurrentQueryResult = CheckHoverPosition(StartingCoordinate, Dimensions);

	if (CurrentQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex,Dimensions);
		return;
	}
	UnhighlightSlots(LastHighlightedIndex,LastHighlightedDimensions);

	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		
		const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(CurrentQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment)
		{
			return;
		}
		 
		ChangeHoverType(CurrentQueryResult.UpperLeftIndex, GridFragment->GetGridSize() ,EInv_GridSlotState::GrayedOut);
	}
}

FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions)
{
	FInv_SpaceQueryResult Result;
	
	// 1. 주어진 위치 + 크기가 그리드 범위 안에 있는지 확인
	if (!IsInGridBounds(UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions))
	{
		// 범위를 벗어나면 기본값 리턴 (bHasSpace = false)
		return Result;
	}

	// 2. 기본적으로 공간이 있다고 가정
	Result.bHasSpace = true;

	TSet<int32> OccupiedUpperLeftIndices;
	// 3. 해당 위치에 이미 아이템이 있는지 확인
	//    위치 범위 내에 있는 슬롯들을 순회하면서
	//    유효한 아이템이 있으면 그 아이템의 좌측 상단 인덱스를 Set에 저장
	UInv_InventoryStatics::ForEach2D(GridSlots, UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions, Columns,
		[&](const UInv_GridSlot* GridSlot)
	{
		// 슬롯에 아이템이 있는 경우
		if (GridSlot->GetInventoryItem().IsValid())
		{
			// 그 아이템의 시작 인덱스를 Set에 저장
			OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());

			// 이미 아이템이 있으므로 기본적으로는 공간이 없음
			Result.bHasSpace = false;
		}
	});

	// 4. 만약 충돌한 아이템이 단 하나라면, 교환 가능성 있음 (스왑 가능)
	if (OccupiedUpperLeftIndices.Num() == 1)
	{
		// Set에서 유일한 인덱스를 꺼내옴
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();

		// 해당 인덱스의 아이템 정보를 결과에 저장
		Result.ValidItem = GridSlots[Index]->GetInventoryItem().Get();
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}

	// 최종 결과 반환
	return Result;
}

bool UInv_InventoryGrid::CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundarySize, const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	
	bMouseWithinCanvas = UInv_WidgetUtils::IsWithinBounds(BoundaryPos, BoundarySize, Location);
	if (!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		UnhighlightSlots(LastHighlightedIndex,LastHighlightedDimensions);
		return true;
	}
	return false;
}

void UInv_InventoryGrid::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas)
	{
		return;
	}
	UnhighlightSlots(LastHighlightedIndex,LastHighlightedDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,[&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetOccupiedTexture();
	});
	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}

void UInv_InventoryGrid::UnhighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		if (GridSlot->IsAvailable())
		{
			GridSlot->SetUnoccupiedTexture();
		}
		else
		{
			GridSlot->SetOccupiedTexture();
		}
	});
}

void UInv_InventoryGrid::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EInv_GridSlotState GridSlotState)
{
	UnhighlightSlots(LastHighlightedIndex,LastHighlightedDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [State = GridSlotState](UInv_GridSlot* GridSlot)
	{
		switch (State)
		{
			case EInv_GridSlotState::Occupied:
				GridSlot->SetOccupiedTexture();
				break;
			case EInv_GridSlotState::Unoccupied:
				GridSlot->SetUnoccupiedTexture();
				break;
			case EInv_GridSlotState::GrayedOut:
				GridSlot->SetGrayedOutTexture();
				break;
			case EInv_GridSlotState::Selected:
				GridSlot->SetSelectedTexture();
				break;
		}
	});

	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}

FIntPoint UInv_InventoryGrid::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions, const EInv_TileQuadrant& Quadrant) const
{
	// 아이템 가로, 세로가 짝수인지 여부를 1 or 0으로 저장
	// 짝수면 1, 홀수면 0
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;
    
	FIntPoint StartingCoordinate;

	// 마우스가 칸 내 어느 사분면인지에 따라 시작 좌표 계산 방법이 달라짐
	switch (Quadrant)
	{
	case EInv_TileQuadrant::TopLeft:
		// 사분면이 좌측 상단이면,
		// 아이템의 시작 좌표는 현재 좌표에서 아이템 크기의 절반(내림)을 뺀 위치
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;

	case EInv_TileQuadrant::TopRight:
		// 사분면이 우측 상단이면,
		// 가로 방향으로는 절반 내림 위치에 짝수면 +1을 더해줌 (정렬 보정)
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;

	case EInv_TileQuadrant::BottomLeft:
		// 사분면이 좌측 하단이면,
		// 세로 방향에 짝수일 경우 +1 보정
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;

	case EInv_TileQuadrant::BottomRight:
		// 사분면이 우측 하단이면,
		// 가로, 세로 둘 다 짝수일 경우 +1씩 보정
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;

	default:
		UE_LOG(LogInventory, Error , TEXT("Invalid Quadrant."))
		return FIntPoint(-1,-1);
	}
	return StartingCoordinate;
}

FIntPoint UInv_InventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPosition,const FVector2D& MousePosition) const
{
	return FIntPoint{
		static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X) / TileSize)),
		static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y) / TileSize))
	};
}

EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const
{
	// 마우스 위치에서 캔버스(격자 전체)의 좌상단 위치를 빼면
	// 현재 마우스가 캔버스 기준으로 얼마나 떨어졌는지 알 수 있음
	// 그걸 타일 크기로 나눈 나머지를 구하면 → 타일 안에서의 상대 좌표가 됨
	const float TileLocalX = FMath::Fmod((MousePosition.X - CanvasPosition.X), TileSize);
	const float TileLocalY = FMath::Fmod((MousePosition.Y - CanvasPosition.Y), TileSize);

	// 타일의 세로 중앙보다 위면 → 위쪽 절반
	const bool bIsTop = TileLocalY < TileSize / 2;

	// 타일의 가로 중앙보다 왼쪽이면 → 왼쪽 절반
	const bool bIsLeft = TileLocalX < TileSize / 2;
	
	EInv_TileQuadrant HoveredTileQuadrant{EInv_TileQuadrant::None};

	// 위쪽 + 왼쪽 → 좌상단 사분면
	if (bIsTop && bIsLeft)
	{
		HoveredTileQuadrant = EInv_TileQuadrant::TopLeft;
	}
	// 위쪽 + 오른쪽 → 우상단 사분면
	else if (bIsTop && !bIsLeft)
	{
		HoveredTileQuadrant = EInv_TileQuadrant::TopRight;
	}
	// 아래쪽 + 왼쪽 → 좌하단 사분면
	else if (!bIsTop && bIsLeft)
	{
		HoveredTileQuadrant = EInv_TileQuadrant::BottomLeft;
	}
	// 아래쪽 + 오른쪽 → 우하단 사분면
	else
	{
		HoveredTileQuadrant = EInv_TileQuadrant::BottomRight;
	}
	
	// 계산된 사분면 결과 반환
	return HoveredTileQuadrant;
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	return HasRoomForItem(Item->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest)
{
	FInv_SlotAvailabilityResult Result;

	//스택으로 쌓을 수 있는 아이템인지
	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;

	// 스택을 얼마나 더 쌓을 수 있는지
	const int32 MaxStackSize = Result.bStackable ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = Result.bStackable ? StackableFragment->GetStackCount() : 1;

	//중복 방지 TSet사용 
	TSet<int32> CheckIndices;
 	for (const auto& GridSlot : GridSlots)
	{
		// 스택 쌓을게 없으면 종료
		if (AmountToFill == 0)
		{
			break;
		}

 		// 현재 인덱스가 이미 체크된 인덱스 목록에 포함되어 있는지
 		if (IsIndexClaimed(CheckIndices,GridSlot->GetIndex()))
 		{
 			continue;
 		}

 		// 아이템이 그리드 영역에 있는지
 		if (!IsInGridBounds(GridSlot->GetIndex(), GetItemDimensions(Manifest)))
 		{
 			continue;
 		}
 		
		// 인덱스에 공간이 있는지
 		TSet<int32> TentativelyClaimed;
 		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(Manifest),CheckIndices,TentativelyClaimed, Manifest.GetItemType(), MaxStackSize))
 		{
 			continue;
 		}
 		
 		// 슬롯을 얼마나 채워야 되는지
 		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill, GridSlot);
 		if (AmountToFillInSlot == 0)
 		{
 			continue;
 		}

 		CheckIndices.Append(TentativelyClaimed);

 		Result.TotalRoomToFill += AmountToFillInSlot;
 		Result.SlotAvailabilities.Emplace(
			FInv_SlotAvailability{
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetIndex(),
				Result.bStackable? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
 		);

 		AmountToFill -= AmountToFillInSlot;
 		Result.Remainder = AmountToFill;
 		if (AmountToFill == 0)
 		{
 			return Result;
 		}
	}
	
	return Result;
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32& MaxStackSize)
{
	bool bHasRoomAtIndex = true;

	UInv_InventoryStatics::ForEach2D(GridSlots, GridSlot->GetIndex(), Dimensions, Columns, [&](const UInv_GridSlot* SubGridSlot)
	{
		if (CheckSlotConstraints(GridSlot,SubGridSlot, CheckedIndices, OutTentativelyClaimed, ItemType, MaxStackSize))
		{
			OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		}
		else
		{
			bHasRoomAtIndex = false;
		}
	});
	
	return bHasRoomAtIndex;
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot,  const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32& MaxStackSize) const
{
	//인덱스가 이미 사용 됐는지
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetIndex()))
	{
		return false;
	}

	//이 슬롯에 이미 아이템이 들어있는지
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimed.Add(SubGridSlot->GetIndex());
		return true;
	}

	//슬롯에 아이템이 있으면 (즉, 스택 가능한 경우인지 확인해야 함)
	//만약 SubGridSlot이 해당 아이템의 좌측 상단 칸이 아니라면
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot))
	{
		return false;
	}

	//스택 쌓을 수 있는 아이템인가?
	UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
	if (!SubItem->IsStackable())
	{
		return false;
	}

	// 아이템 타입이 맞는지
	if (!DoesItemTypeMatch(SubItem, ItemType))
	{
		return false;
	}

	//최대 사이즈보다 많은지
	if (GridSlot->GetStackCount() >= MaxStackSize)
	{
		return false;
	}
	
	return true;
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlot* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::IsUpperLeftSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetIndex();
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInv_InventoryGrid::IsInGridBounds(const int32& StartIndex, const FIntPoint& ItemDimensions) const
{
	//시작 인덱스가 유효한 범위(0 이상, 전체 그리드 칸 수 이하)가 아니면 false 반환.
	if (StartIndex < 0 || StartIndex >= GridSlots.Num())
	{
		return false;
	}
	// EndColumn: 시작 인덱스의 열(column) 위치 + 아이템의 가로 크기
	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	// EndRow: 시작 인덱스의 행(row) 위치 + 아이템의 세로 크기
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;
	return EndColumn <= Columns && EndRow <= Rows;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountFill, const UInv_GridSlot* GridSlot) const
{
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(RoomInSlot,AmountFill) : 1;
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlot* GridSlot) const
{
	int32 CurrentSLotStackCount = GridSlot->GetStackCount();
	if ( const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex() ; UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSLotStackCount = UpperLeftGridSlot->GetStackCount();
	}
	return CurrentSLotStackCount;
}

bool UInv_InventoryGrid::IsRightClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInv_InventoryGrid::IsLeftClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	
}

void UInv_InventoryGrid::PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 Index)
{
	AssignHoverItem(ClickedInventoryItem, Index,  Index);
	RemoveItemFromGrid(ClickedInventoryItem, Index);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	HoverItem->SetPreviousGridIndex(PreviousGridIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

void UInv_InventoryGrid::RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex)
{
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem,FragmentTags::GridFragment);
	if (!GridFragment)
	{
		return;
	}
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(nullptr);
		GridSlot->SetUpperLeftIndex(INDEX_NONE);
		GridSlot->SetStackCount(0);
		GridSlot->SetAvailable(true);
		GridSlot->SetUnoccupiedTexture();
	});

	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInv_HoverItem>(GetOwningPlayer(),HoverItemClass);
	}

	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(InventoryItem,FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(InventoryItem,FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment)
	{
		return;
	}

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}


void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get()))
	{
		return;
	}

	for ( const auto& Availability : Result.SlotAvailabilities )
	{
		if (Availability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
		}
		else
		{
			AddItemAtIndex(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
			UpdateGridSlots(Result.Item.Get(),Availability.Index, Result.bStackable, Availability.AmountToFill);
		}
	}
}

void UInv_InventoryGrid::OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	check(GridSlots.IsValidIndex(GridIndex));
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();

	if (!IsValid(HoverItem) && IsLeftClick(MouseEvent))
	{
		PickUp(ClickedInventoryItem,GridIndex);
		return;
	}

	if (IsRightClick(MouseEvent))
	{
		CreateItemPopUp(GridIndex);
		return;
	}

	// Do the hovered item and the cliced inventory itme share a type, and are they stackable,
	if (IsSameStackable(ClickedInventoryItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoryItem->GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = HoverItem->GetStackCount();
		
		if (ShouldSwapStackCounts(RoomInClickedSlot,HoveredStackCount, MaxStackSize))
		{
			SwapStackCounts(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}

		if (ShouldConsumeHoverItemStacks(HoveredStackCount, RoomInClickedSlot))
		{
			ConsumeHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}

		if (ShouldFillInStack(RoomInClickedSlot, HoveredStackCount))
		{
			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return;
		}

		if (RoomInClickedSlot == 0)
		{
			return;
		}
		return;
	}

	//호버 아이템 스왑
	SwapWithHoverItem(ClickedInventoryItem,GridIndex);
}

void UInv_InventoryGrid::CreateItemPopUp(const int32 GridIndex)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem))
	{
		return;
	}
	if (IsValid(GridSlots[GridIndex]->GetItemPopUp()))
	{
		return;
	}
	
	ItemPopUp = CreateWidget<UInv_ItemPopUp>(this,ItemPopUpClass);
	GridSlots[GridIndex]->SetItemPopUp(ItemPopUp);
	
	OwningCanvasPanel->AddChild(ItemPopUp);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasSlot->SetPosition(MousePosition - ItemPopupOffset);
	CanvasSlot->SetSize(ItemPopUp->GetBoxSize());

	const int32 SliderMax = GridSlots[GridIndex]->GetStackCount() - 1;
	if (RightClickedItem->IsStackable() && SliderMax > 0)
	{
		ItemPopUp->OnSplit.BindDynamic(this, &ThisClass::OnPopUpMenuSplit);
		ItemPopUp->SetSliderParms(SliderMax, FMath::Max(1, GridSlots[GridIndex]->GetStackCount() / 2));
	}
	else
	{
		ItemPopUp->CollapseSplitButton();
	}

	ItemPopUp->OnDrop.BindDynamic(this, &ThisClass::OnPopUpMenuDrop);

	if (RightClickedItem->IsConsumable())
	{
		ItemPopUp->OnConsume.BindDynamic(this, &ThisClass::OnPopUpMenuConsume);
	}
	else
	{
		ItemPopUp->CollapseConsumeButton();
	}
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& Manifest) const
{
	const FInv_GridFragment* GridFragment = Manifest.GetFragmentOfType<FInv_GridFragment>();
	return  GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!MatchesCategory(Item))
	{
		return;
	}

	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);
	AddItemToIndices(Result,Item);
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem,Availability.Index, Result.bStackable, Availability.AmountToFill);
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable,const int32 StackAmount)
{
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item,FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item,FragmentTags::IconFragment);
	const FInv_StackableFragment* StackableFragment = GetFragment<FInv_StackableFragment>(Item,FragmentTags::IconFragment);
	
	if (!GridFragment || !ImageFragment)
	{
		return;
	}

	UInv_SlottedItem* SlottedItem = CreateSlottedItem(Item,bStackable,StackAmount,GridFragment,ImageFragment,Index);
	AddSlottedItemToCanvas(Index,GridFragment,SlottedItem);

	SlottedItems.Add(Index,SlottedItem);
	
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable,const int32 StackAmount, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,const int32 Index)
{
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(),SlottedItemClass);
	SlottedItem->SetInventoryItem(Item);
	SetSlottedItemImage(SlottedItem,GridFragment,ImageFragment);
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackUpdateAmount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClicked);

	return SlottedItem;
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index,Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));

	if (bStackableItem)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewItem,FragmentTags::GridFragment);
	if (!GridFragment)
	{
		return;
	}
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);

	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions,Columns,[&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(NewItem);
		GridSlot->SetUpperLeftIndex(Index);
		GridSlot->SetOccupiedTexture();
		GridSlot->SetAvailable(false);
	});
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

void UInv_InventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Rows * Columns);

	for (int32 j = 0; j < Rows; ++j)
	{
		for (int32 i = 0; i < Columns; i++)
		{
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this,GridSlotClass);
			CanvasPanel->AddChildToCanvas(GridSlot);

			const FIntPoint TitlePosition = FIntPoint(i,j);
			int32 Index = UInv_WidgetUtils::GetIndexFromPosition(TitlePosition,Columns);
			GridSlot->SetTitleIndex(Index);

			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCPS->SetSize(FVector2D(TileSize,TileSize));
			GridCPS->SetPosition(TitlePosition * TileSize);

			GridSlots.Add(GridSlot);
			GridSlot->GridSlotClicked.AddDynamic(this,&ThisClass::OnGridSlotClicked);
			GridSlot->GridSlotHovered.AddDynamic(this, &ThisClass::OnGridSlotHovered);
			GridSlot->GridSlotUnhovered.AddDynamic(this, &ThisClass::OnGridSlotUnhovered);
		}
	}
}

void UInv_InventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem))
	{
		return;
	}
	if (!GridSlots.IsValidIndex(ItemDropIndex))
	{
		return;
	}

	//이미 그 위치에 아이템이 하나 있고, 스왑 가능한 상황이면
	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		OnSlottedItemClicked(CurrentQueryResult.UpperLeftIndex, MouseEvent);
		return;
	}

	auto GridSlot = GridSlots[ItemDropIndex];
	//그 자리에 아이템이 없는 경우 (빈 슬롯)
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		PutDownOnIndex(ItemDropIndex);
	}
}

void UInv_InventoryGrid::PutDownOnIndex(const int32 Index)
{
	AddItemAtIndex(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	UpdateGridSlots(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount());
	ClearHoverItem();
}

void UInv_InventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetPreviousGridIndex(INDEX_NONE);
	HoverItem->UpdateStackCount(0);
	HoverItem->SetImageBrush(FSlateNoResource());
	
	HoverItem->RemoveFromParent();
	HoverItem = nullptr;

	ShowCursor();
}

UUserWidget* UInv_InventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer()))
	{
		return nullptr;
	}

	if (!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(),VisibleCursorWidgetClass);
	}
	return VisibleCursorWidget;
}

UUserWidget* UInv_InventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer()))
	{
		return nullptr;
	}

	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(),VisibleCursorWidgetClass);
	}
	return HiddenCursorWidget;
}

bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	const bool bIsSameItem = ClickedInventoryItem == HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable && HoverItem->GetItemTag().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

void UInv_InventoryGrid::SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if ( !IsValid(HoverItem))
	{
		return;
	}

	UInv_InventoryItem* TempInventoryItem = HoverItem->GetInventoryItem();
	const int32 TempStackCount = HoverItem->GetStackCount();
	const bool bTempIsStackable = HoverItem->IsStackable();

	AssignHoverItem(ClickedInventoryItem, GridIndex, HoverItem->GetPreviousGridIndex());
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
	AddItemAtIndex(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
	UpdateGridSlots(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
}

bool UInv_InventoryGrid::ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount, const int32 MaxStackSize) const
{
	return RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize;
}

void UInv_InventoryGrid::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index)
{
	UInv_GridSlot* GridSlot = GridSlots[Index];
	GridSlot->SetStackCount(HoveredStackCount);
	

	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(HoveredStackCount);

	HoverItem->UpdateStackCount(ClickedStackCount);
}

bool UInv_InventoryGrid::ShouldConsumeHoverItemStacks(const int32 HoveredStackCount, const int32 RoomInClickedSlot) const
{
	return RoomInClickedSlot >= HoveredStackCount;
}

void UInv_InventoryGrid::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index)
{
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount + AmountToTransfer;

	GridSlots[Index]->SetStackCount(NewClickedStackCount);
	SlottedItems[Index]->UpdateStackCount(NewClickedStackCount);
	ClearHoverItem();
	ShowCursor();

	const FInv_GridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInv_GridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);
	HighlightSlots(Index,Dimensions);
}

bool UInv_InventoryGrid::ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const
{
	return RoomInClickedSlot < HoveredStackCount;
}

void UInv_InventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	UInv_GridSlot* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);

	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

	HoverItem->UpdateStackCount(Remainder);
}

void UInv_InventoryGrid::ShowCursor()
{
	if (!IsValid(GetOwningPlayer()))
	{
		return;
	}
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetVisibleCursorWidget());
}

void UInv_InventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer()))
	{
		return;
	}
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetHiddenCursorWidget());
}

void UInv_InventoryGrid::SetOwningCanvas(UCanvasPanel* InOwningCanvas)
{
	OwningCanvasPanel = InOwningCanvas;
}

void UInv_InventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem))
	{
		return;
	}

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetOccupiedTexture();;
	}
}

void UInv_InventoryGrid::OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem))
	{
		return;
	}

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetUnoccupiedTexture();;
	}
}

void UInv_InventoryGrid::OnPopUpMenuSplit(int32 SplitAmount, int32 Index)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem) || !RightClickedItem->IsStackable())
	{
		return;
	}

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 StackCount = UpperLeftGridSlot->GetStackCount();
	const int32 NewStackCount = StackCount - SplitAmount;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	AssignHoverItem(RightClickedItem, UpperLeftIndex, UpperLeftIndex);
	HoverItem->UpdateStackCount(SplitAmount);
}

void UInv_InventoryGrid::OnPopUpMenuDrop(int32 Index)
{
}

void UInv_InventoryGrid::OnPopUpMenuConsume(int32 Index)
{
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}

