# Demo 架构设计规格

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 定义Demo项目的整体C++架构，包括角色系统、动画蓝图交互接口、调试开关面板。

**Architecture:** LTCharacter作为角色核心，持有Locomotion组件管理运动状态。ABP通过C++接口获取状态数据（速度、方向、脚部状态等）。调试面板通过ULTDebugSubsystem控制各技术层开关。

**Tech Stack:** UE5.7 C++, LyraAnim Plugin, EnhancedInput

---

## 1. 角色系统

### LTCharacter

- 继承 `ACharacter`
- 持有 `ULTLocomotionComponent`（运动数据采集与状态管理）
- 通过EnhancedInput接收移动/视角输入
- 为ABP提供数据接口：速度、加速度、朝向、运动方向、脚部状态

### LTPlayerController

- EnhancedInput MappingContext管理
- 输入动作绑定（Move, Look, Jump, Toggle Debug Panel）
- 相机控制

### LTPlayerState

- 存储跨生命周期的玩家数据
- 当前阶段预留，后续扩展

## 2. Locomotion组件

### ULTLocomotionComponent

- 挂在LTCharacter上
- **职责**:
  - 采集角色运动数据：速度、加速度、角速度、运动方向（前/后/左/右）
  - 计算Distance Matching相关数据
  - 管理Locomotion状态机状态（Idle/Start/Loop/Stop/Pivot/Jump等）
  - 脚部着地检测（为Stop左右脚选择服务）
- **不负责**: 动画播放（由ABP负责）

### 数据接口

提供给ABP的关键属性：

```cpp
// 运动数据
float Speed;                    // 当前速度
FVector Velocity;               // 速度向量
FVector Acceleration;           // 加速度
float HeadingAngle;             // 朝向角度
ELTMovementDirection Direction; // 运动8方向

// 状态
ELTLocomotionState LocomotionState; // Idle/Start/Loop/Stop等
bool bIsLeftFootForward;           // 左脚在前

// Distance Matching
float DistanceMatchingDistance;     // DM目标距离

// 调试
bool bDebugDrawEnabled;            // 调试绘制开关
```

## 3. 调试系统

### ULTProjectSettings

- 项目设置（Project Settings → Game → Locomotion Template）
- 配置Debug面板滑动条的值范围

```cpp
UCLASS(Config=Game, defaultconfig)
class ULTProjectSettings : public UDeveloperSettings
{
    // CMC参数范围
    FLinearColor MaxAccelerationRange;  // X=min, Y=max, Z=default
    FLinearColor BrakingDecelerationRange;
    FLinearColor MaxWalkSpeedRange;
};
```

### ULTDebugSubsystem

- GameInstance级别子系统
- 管理技术层开关 + CMC参数值
- 持久化开关状态（运行时，不存盘）

### FLTToggleSettings 开关结构体

```
bUseBlendSpaceLoop   = true    ← 特殊模式：ON时锁定除速度/加速度/制动力外所有控件
bEnableStart         = false
bEnableStartDM       = false   ← 仅bEnableStart=true时可见
bEnableStop          = false
bEnableStopDM        = false
bEnablePivot         = false
bEnablePivotDM       = false
bEnableAimOffset     = false
bEnableTurnInPlace   = false   ← 依赖bEnableAimOffset，AO关时灰色不可点
bEnableFootIK        = false
bEnableLean          = false
```

### 滑动条参数

| 参数 | 读取/写入 | 范围来源 |
|------|----------|---------|
| MaxAcceleration | CMC → Slider | ULTProjectSettings |
| BrakingDeceleration | CMC → Slider | ULTProjectSettings |
| MaxWalkSpeed | CMC → Slider | ULTProjectSettings |

### 面板UI层级关系

```
[Slider] MaxAcceleration
[Slider] BrakingDeceleration
[Slider] MaxWalkSpeed

[Toggle] BlendSpace Loop ←──────────────── ON时下面全部锁定
  [Toggle] Start
    [Toggle] Start DM                    ← Start ON才显示
  [Toggle] Stop
    [Toggle] Stop DM
  [Toggle] Pivot
    [Toggle] Pivot DM
  [Toggle] Aim Offset
  [Toggle] Turn In Place                 ← AO ON才可交互
  [Toggle] Foot IK
  [Toggle] Lean
```

### LTHUD

- 渲染调试面板UI（Widget）
- Toggle开关（通过快捷键呼出）
- 实时显示当前运动数据

## 4. 输入系统

### MappingContext

- `IMC_Default`: 基础移动+视角
- 后续按需添加

### Input Actions

| IA | 类型 | 用途 |
|----|------|------|
| IA_Move | Value(Vector2D) | WASD/左摇杆移动 |
| IA_Look | Value(Vector2D) | 鼠标/右摇杆视角 |
| IA_Jump | Digital | 跳跃 |
| IA_DebugMenu | Digital | 切换调试面板 |

## 5. 文件规划

```
Source/LocomotionTemplate/
├── Core/
│   ├── LTGameMode.h/cpp
│   ├── LTGameInstance.h/cpp
│   ├── LTPlayerState.h/cpp
│   ├── LTDebugSubsystem.h/cpp
│   └── LTProjectSettings.h/cpp          ← 新增（项目设置）
├── Character/
│   ├── LTCharacter.h/cpp
│   ├── LTPlayerController.h/cpp
│   ├── LTHUD.h/cpp
│   └── Locomotion/
│       ├── LTLocomotionComponent.h/cpp
│       └── LTLocomotionTypes.h
└── UI/
    └── SLTDebugPanel.h/cpp              ← Slate调试面板
```

## 6. 阶段对应关系

| 阶段 | 需要实现 | C++类 | ABP改动 |
|------|---------|-------|---------|
| B-Loop | 基础移动+OW | LTLocomotionComponent | Blend Space + OW节点 |
| C-Start | DM启动 | 组件加DM数据 | Start状态+DM节点 |
| D-Stop | DM停止+脚部检测 | 组件加脚部状态 | Stop状态+截断逻辑 |
| E-AO | 瞄准偏移 | 组件加瞄准数据 | AO节点 |
| F-TIP | 原地转向 | 组件加转向状态 | TIP逻辑 |
| G-Pivot | 变向 | 组件加Pivot检测 | Pivot状态 |
| H-Jump | 跳跃 | 组件加跳跃状态 | 跳跃状态机 |
| I-Lean | 身体倾斜 | 组件加角速度 | Lean曲线 |
| J-FootIK | 脚部IK | ULTFootIKComponent | IK节点 |
| K-MM | Motion Matching | 无（纯ABP） | MM节点替换 |
