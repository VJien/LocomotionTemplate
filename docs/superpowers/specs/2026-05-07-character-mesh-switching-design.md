# 角色Mesh切换系统设计

## 概述

运行时角色Mesh切换系统。每个角色配置一个DataAsset（包含mesh + ABP + linked layers），所有DA在Project Settings中配置为数组，通过DebugPanel中的Button进行切换。

## 需求

- 运行时切换 SkeletalMesh、AnimClass、LinkedAnimClassLayers
- DataAsset驱动，编辑器可配置
- 集成到现有DebugPanel
- 直接切换，无过渡效果
- Button支持文字和可选Icon

## 架构

### 数据层 — `ULTCharacterSet`

路径：`Source/LocomotionTemplate/Data/LTCharacterSet.h/.cpp`

```cpp
UCLASS()
class ULTCharacterSet : public UDataAsset
{
    UPROPERTY(EditAnywhere)
    FText DisplayName;

    UPROPERTY(EditAnywhere)
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere)
    USkeletalMesh* SkeletalMesh;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UAnimInstance> AnimClass;

    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<UAnimInstance>> LinkedAnimClasses;
};
```

每个角色一个DA实例（如 `DA_Shinbi`、`DA_Kwang`）。

### 配置层 — `ULTProjectSettings` 扩展

路径：`Source/LocomotionTemplate/Core/LTProjectSettings.h`

在现有DeveloperSettings类中新增字段：

```cpp
UPROPERTY(Config=Game, EditAnywhere)
TArray<ULTCharacterSet*> CharacterSets;
```

在Project Settings编辑器中配置。数组长度 = Button数量。

### 逻辑层 — `ALTCharacter` 扩展

路径：`Source/LocomotionTemplate/Character/LTCharacter.h/.cpp`

新增方法：

- `ApplyCharacterSet(int32 Index)` — 读取 `ULTProjectSettings::CharacterSets[Index]`，应用mesh/anim/linked layers
- `GetCurrentCharacterSetIndex()` — 返回当前选中索引
- 委托 `OnCharacterSetChanged` — 广播索引变化，通知UI刷新

切换流程：
1. 读取 `ULTProjectSettings::CharacterSets` 数组
2. `GetMesh()->SetSkeletalMesh(Set->SkeletalMesh)`
3. `GetMesh()->SetAnimClass(Set->AnimClass)`
4. 遍历 `Set->LinkedAnimClasses`，逐个调用 `GetMesh()->LinkAnimClassLayers(Class)`
5. 广播 `OnCharacterSetChanged`

### UI层 — DebugPanel 扩展

在现有DebugPanel Widget中新增Section：
- 构造时读取 `ULTProjectSettings::CharacterSets` 数组长度
- 每个元素生成一个Button
- Button内容：有Icon显示Icon，否则显示DisplayName
- 点击回调 → `Character->ApplyCharacterSet(Index)`
- 当前选中Button高亮（通过 `OnCharacterSetChanged` 委托跟踪）

DebugPanel同时提供调试用Slider：
- `Acceleration`、`Braking Decel`、`Walk Speed` 读取当前 `CharacterMovement` 初始值
- `Camera Arm` 读取当前 `CameraBoom->TargetArmLength` 初始值
- Slider范围来自 `ULTProjectSettings`

## 文件变更

### 新增文件
| 文件 | 用途 |
|------|------|
| `Source/LocomotionTemplate/Data/LTCharacterSet.h` | DA类声明 |
| `Source/LocomotionTemplate/Data/LTCharacterSet.cpp` | DA类实现 |

### 修改文件
| 文件 | 变更 |
|------|------|
| `Source/LocomotionTemplate/Core/LTProjectSettings.h` | 新增 `CharacterSets` 数组 |
| `Source/LocomotionTemplate/Core/LTProjectSettings.h` | 新增 `CameraArmLengthRange` |
| `Source/LocomotionTemplate/Character/LTCharacter.h` | 新增 `ApplyCharacterSet`、委托、索引追踪 |
| `Source/LocomotionTemplate/Character/LTCharacter.cpp` | 实现切换逻辑 |

### 蓝图变更
| 资产 | 变更 |
|------|------|
| DebugPanel Widget | 新增CharacterSet区域，动态生成Button |
| DA实例 | 每个角色创建一个DA（DA_Shinbi等） |

## 命名规范

- 类前缀遵循项目约定：`ULT` 用于UObject派生类
- DA资产命名：`DA_<角色名>`，存放于 `Content/LT/Data/`

## 约束

- 不修改LyraAnim插件
- 无过渡效果，直接切换
- 当前功能不需要在Build.cs中添加 `"LyraAnim"` 依赖
