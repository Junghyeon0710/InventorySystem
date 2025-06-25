## Unreal5 : Grid Inventory
공부 목적으로 제작된 간단한 네트워크 인벤토리 토이 프로젝트입니다.


- 엔진 : Unreal Engine 5.6              
- IDE : Rider                          
- 툴 : C++, Blueprint 혼합 사용       
- 인원 : 개인 개발                      
- 아키텍처 : Fragment System, Plugin        
- UI : Composite Pattern              

## 영상

# Item Fragments
- 컴포넌트 기반 아키텍처(Fragment 시스템)
- Lyra Fragment방식
- Instanced Struct을 이용한 최적화
- UObject를 위한 컴포넌트 방식

<br>

## Fragment 설계
```C++
USTRUCT(BlueprintType)
struct FInv_ItemFragment
{
	GENERATED_BODY()

	FInv_ItemFragment() {};
	//가상 소멸자 호출하면 위에 5가지 필수 (가상 소멸자 호출시 자동으로 생성 안해주기 때문, 복사생성자, 이동생성자, 대입)
	FInv_ItemFragment(const FInv_ItemFragment&) = default;
	FInv_ItemFragment& operator=(const FInv_ItemFragment&) = default;
	FInv_ItemFragment(FInv_ItemFragment&&) = default;
	FInv_ItemFragment& operator=(FInv_ItemFragment&&) = default;
	virtual ~FInv_ItemFragment() {};


	FGameplayTag GetFragmentTag() const {return FragmentTag;}
	void SetFragmentTag(FGameplayTag Tag) {FragmentTag = Tag;}
	virtual void Manifest() {}
private:

	UPROPERTY(EditAnywhere, Category="Inventory" , meta = (Categories="FragmentTags"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
	
};
```
- virtual ~Destructor(): 상속받은 구조체를 부모 포인터로 안전하게 삭제하기 위해 가상함수로 설계
- Fragment의 정체성을 위한 GameplayTag 사용했습니다. 이를 통해 프래그먼트를 찾거나 구분할 수 있게 설계

<br>

## Child Fragment 예시
```C++
USTRUCT(BlueprintType)
struct FInv_GridFragment : public FInv_ItemFragment
{
	GENERATED_BODY()

	FIntPoint GetGridSize() const {return GridSize;}
	void SetGridSize(FIntPoint& Size) {GridSize = Size;}
	float GetGridPadding() const {return GridPadding;}
	void SetGridPadding(float Padding) {GridPadding = Padding;}
	
private:

	UPROPERTY(EditAnywhere, Category="Inventory")
	FIntPoint GridSize{1,1};

	UPROPERTY(EditAnywhere, Category="Inventory")
	float GridPadding{0.f};
};

```
- FInv_ItemFragment을 상속받아 데이터를 넣어줌
- 아이템이 인벤토리에서 차지하는 공간과 패딩 정보를 담당하는 프래그먼트
- 다양한 속성은 각각의 Fragment로 확장할 수 있음


<br>
<br>


## Fragment 사용하기
> 아이템 컴포넌트가 갖고 있는 ItemManifest
```C++
UPROPERTY(EditAnywhere, Category="Inventory" , meta = (ExcludeBaseStruct))
TArray<TInstancedStruct<FInv_ItemFragment>> Fragments;
```
- 아이템 컴포넌트에 배열로 갖습니다.
- TInstancedStruct 사용으로 UObject클래스 보다 최적화 적용
-  대신 UObject가 지원하는 UFCUTION매크로, 타이머 등등 사용하지 못하는 제약이 있음
  
> 아이템 사용 예시

![Image](https://github.com/user-attachments/assets/c7f4047d-a974-4e70-aa64-23079480e6b1)

- 아이템마다 필요한 Fragment를 조합해 기능을 부여
- 유연하고 재사용성 높은 설계를 가능하게 함

>유틸 함수 제공
#### 아이템에 포함된 Fragment들을 간단하게 조회할 수 있도록 다음과 같은 템플릿 기반 유틸리티 함수들을 제공합니다.
```C++
template<typename T> requires std::derived_from<T, FInv_ItemFragment> //  requires std::derived_from<T, FInv_ItemFragment> 해당 파생된게 아니면 컴파일 단에서 에러
const T* FInv_ItemManifest::GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const
{
	for (const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			if (FragmentPtr->GetFragmentTag().IsValid() || FragmentPtr->GetFragmentTag().MatchesTagExact(FragmentTag))
			{
				return FragmentPtr;
			}
		}
	}
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
const T* FInv_ItemManifest::GetFragmentOfType() const
{
	for (const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
T* FInv_ItemManifest::GetFragmentOfTypeMutable()
{
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInv_ItemFragment>
TArray<const T*> FInv_ItemManifest::GetAllFragmentOfType() const
{
	TArray<const T*> Result;
	for (const TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			Result.Add(FragmentPtr);
		}
	}
	return Result;
}
```
- 유틸 함수로 아이템의 Fragment를 쉽게 찾아 해당 Fragment의 맞는 기능을 구현하계 설계
- std::derived_from<T, FInv_ItemFragment> 제약 조건을 통해 컴파일 타임 타입 안전성 보장

>사용 예시 
```C++
const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item,FragmentTags::GridFragment);
Widget->SetGridDimensions(GridFragment->GetGridSize())	
```
- 특정 Item에서 GridFragment를 찾아 UI 위젯에 설정
- Fragment 기반 설계 덕분에 각 기능은 개별 프래그먼트로 분리되고, 필요시 해당 프래그먼트를 찾아 기능을 호출할 수 있도록 설계됨
  

<br>
<br>

# Composite Pattern
- Composite 패턴을 활용한 Item Description 위젯 구성
- 해당 속성의 프래그먼트(Fragment)**가 없으면 보여주지 않도록 설계
- ItemDescriptionWidget(부모) 하나에 단일 함수만 호출
- 내부의 여러 하위 위젯들이 자동으로 자신의 프래그먼트 정보를 받아서 UI를 구성

<br>

![Image](https://github.com/user-attachments/assets/b79f7ee9-9cc3-4746-b5e1-2b44b615a2c2)
- 해당 위젯은 아이템 설명 위젯
- 위젯에 맞는 설명을 아이템에 맞는 Fragment를 이용하여 알아서 채워지도록 설계

## Composite Pattern 설계

>기본 클래스: UINVCompositeBase
```C++
UCLASS()
class INVENTORYSYSTEM_API UInv_CompositeBase : public UUserWidget
{
	GENERATED_BODY()
public:

	FGameplayTag GetFragmentTag() const {return FragmentTag;}
	void SetFragmentTag(const FGameplayTag& Tag) {FragmentTag = Tag;}
	virtual void Collapse() {};
	void Expand();

	using FuncType = TFunction<void(UInv_CompositeBase*)>;
	virtual void ApplayFunction(FuncType Function) {}
	
private:

	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag FragmentTag;
};
```
- Collapse() / Expand() 함수 제공: 위젯 가시성 제어하도록 설계
- 함수를 위임할 수 있는 ApplyFunction() 가상 함수 정의
- TFunction 템플릿을 사용하여 UInv_CompositeBase* 파라미터를 갖고 void를 반환하는 함수 정의
- Gameplay Tag (FGameplayTag FragmentTag) 보유: 어떤 아이템 fragment와 연결될지 식별용

###Composite 클래스: INV_Composite
#### UINVCompositeBase에서 파생
> INV_Composite.h
```C++
UCLASS()
class INVENTORYSYSTEM_API UInv_Composite : public UInv_CompositeBase
{
	GENERATED_BODY()
public:

	virtual void NativeOnInitialized() override;
	virtual void ApplayFunction(FuncType Function) override;
	virtual void Collapse() override;
	TArray<UInv_CompositeBase*> GetChildren() {return Children;}
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UInv_CompositeBase>> Children;
};
```
- 자식 위젯들을 저장하는 TArray<UINVCompositeBase*> Children 을 가지고 있음
- ApplyFunction() 오버라이드 : 자식들에게 재귀적으로 ApplyFunction 호출

>INV_Composite.cpp
```C++
void UInv_Composite::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UInv_CompositeBase* Composite = Cast<UInv_CompositeBase>(Widget);
		if (IsValid(Composite))
		{
			Children.Add(Composite);
			Composite->Collapse();
		}
	});
}

void UInv_Composite::ApplayFunction(FuncType Function)
{
	for (auto& Child : Children)
	{
		Child->ApplayFunction(Function);
	}
}

void UInv_Composite::Collapse()
{
	for (auto& Child : Children)
	{
		Child->Collapse();
	}
}
```
- UInv_Composite위젯이 초기화 될 때 UInv_CompositeBase을 상속 있는 모든 자식 위젯들을 Children 배열에 넣어줌
- ApplyFunction() 오버라이드 : 자식들에게 재귀적으로 ApplyFunction 호출하도록 설계

###  Leaf 클래스: UINVLeaf
>UInv_Leaf.h
```C++
UCLASS()
class INVENTORYSYSTEM_API UInv_Leaf : public UInv_CompositeBase
{
	GENERATED_BODY()
public:
	virtual void ApplayFunction(FuncType Function) override;
};

```
- 실제로 
- 자식은 없음 단일 노드
- ApplyFunction() 오버라이드: 받은 함수에 자기 자신을 인자로 넘겨 실행.

<br>
> UInv_Leaf.cpp
```C++
void UInv_Leaf::ApplayFunction(FuncType Function)
{
	Function(this);
}
```
- 자기 자신을 인자로 넘겨서 호출
- 이렇게 하면 실제로 ApplayFunction호출 하는곳에서 매개변수로 Leaf인 자기 자신이 매개변수로 들어옴

<br>

##Composite Pattern 사용




