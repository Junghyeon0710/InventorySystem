## Unreal5 : Grid Inventory
공부 목적으로 제작된 간단한 네트워크 인벤토리 토이 프로젝트입니다.

| 항목       | 내용                           |
|------------|--------------------------------|
| 엔진       | Unreal Engine 5.6              |
| IDE        | Rider                          |
| 툴         | C++, Blueprint 혼합 사용       |
| 인원       | 개인 개발                      |
| 아키텍처   | Fragment System, Plugin        |
| UI         | Composite Pattern              |

## 영상

# Item Fragments
- 컴포넌트 기반 아키텍처(Fragment 시스템)
- Lyra Fragment방식
- Instanced Struct을 이용한 최적화
- UObject를 위한 컴포넌트 방식



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
- virtual ~Destructor(): 상속받은 구조체를 부모 포인터로 안전하게 삭제하기 위해 가상함수로 만들어 줬습니다.
- Fragment의 정체성을 위한 GameplayTag 사용했습니다. 이를 통해 프래그먼트를 찾거나 구분할 수 있게 설계

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
- FInv_ItemFragment을 상속받아 데이터를 넣어줍니다.
- 앞으로 Fragment를 조합하여 아이템을 만드는 방식

## Fragment 사용하기
> 아이템 컴포넌트가 갖고 있는 ItemManifest
```C++
UPROPERTY(EditAnywhere, Category="Inventory" , meta = (ExcludeBaseStruct))
TArray<TInstancedStruct<FInv_ItemFragment>> Fragments;
```
- 아이템 컴포넌트에 배열로 갖습니다.
- TInstancedStruct 사용으로 UObject클래스 보다 최적화 적용
  
> 아이템 사용 예시







