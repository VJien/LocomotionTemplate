# LyraAnim Plugin / 动画系统插件

数据驱动的 Locomotion 动画系统，替代原版 Lyra 的 Linked Layer Child AnimBP 模式。

## Overview / 概述

将原版 Lyra 动画系统中每个武器需要单独子 AnimBP 的模式，替换为数据驱动模式。
每个武器/姿态只需创建一个 `ULAAnimConfigDataAsset` 实例即可配置全部动画。

依赖插件: GameplayAbilities, AnimationWarping, AnimationLocomotionLibrary

## When to Use / 何时使用

- 新增武器类型（手枪/步枪/霰弹枪/空手等）
- 修改动画行为参数（步幅扭曲、Root Yaw、播放速率等）
- 配置新的动画资产集合
- 添加新的移动姿态（蹲伏、冲刺等）

## Architecture / 架构

```
                    ┌─────────────────┐
                    │   AnimBP (ABP)  │  ← 动画蓝图框架，不含动画资产
                    │  ABP_LA_Template │
                    └────────┬────────┘
                             │ 读取 AnimConfig
                    ┌────────▼────────┐
                    │  LAAnimInstance  │  ← C++ 基类
                    │  (GAS/Ground/Hurt)│
                    └────────┬────────┘
                             │ 引用
              ┌──────────────▼──────────────┐
              │   LAAnimConfigDataAsset     │  ← 数据资产（每个武器一份）
              │  DA_AnimConfig_ue5_pistol   │
              │  DA_AnimConfig_ue5_unarmed  │
              └─────────────────────────────┘
```

核心文件:
- `Data/LAAnimConfigDataAsset.h` — 动画配置数据资产（所有动画引用 + 行为参数）
- `Data/LATypes.h` — 枚举和结构体定义
- `Core/LAAnimInstance.h` — 动画实例基类（GAS集成、地面检测、受击）
- `Core/LAAnimLinkComponent.h` — 动画层动态切换组件
- `Core/LAAdditiveHurtInterface.h` — 受击叠加动画接口

## How to Add a New Weapon / 如何添加新武器

### Step 1: 创建 DataAsset

1. 在 Content Browser 右键 → Miscellaneous → Data Asset → 选择 `LAAnimConfigDataAsset`
2. 命名为 `DA_AnimConfig_ue5_[weapon_name]`
3. 在 Details 面板中设置 `ValidationFlags`（勾选该武器支持的动画类型）

### Step 2: 配置 Animation 资产

按以下类别填入动画资产（AnimSequence）：

| Category | 说明 | 必填 |
|----------|------|------|
| Idle | HipFire/ADS待机、IdleBreak、蹲伏待机 | ✅ |
| Start | Jog/Walk/Crouch 启动（4方向） | ✅ |
| Stop | Jog/Walk/Crouch 停止（4方向） | ✅ |
| Cycle | Jog/Walk/Crouch 循环（4方向 或 BlendSpace） | ✅ |
| Pivot | Jog/Walk/Crouch 转向（4方向） | 推荐 |
| Sprint | Entry/Exit/Loop/Additive | 可选 |
| TurnInPlace | 站/蹲 左/右 | bEnableRootYawOffset时必填 |
| Jump | Start/StartLoop/Apex/FallLoop/Land/RecoveryAdditive | ✅ |
| Aim | HipFirePose(站/蹲)、AimOffset BlendSpace | ✅ |
| Lean | BS_Jog_Lean BlendSpace1D | 推荐 |
| AdditiveHurt | 四方向受击叠加动画 | 可选 |

### Step 3: 配置 Settings 参数

关键参数（按需调整，有默认值）：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| bEnableRootYawOffset | true | TurnInPlace 基础开关 |
| RootYawOffsetAngleClamp | (-120, 100) | 站立偏移角度限制 |
| CardinalDirectionDeadZone | 10.0 | 方向死区 |
| bOnlyForwardAnimWhenMoveOrientation | true | 速度方向模式仅用前向动画 |
| StrideWarpingBlendInDurationScaled | 0.2 | 步幅扭曲混合时间 |
| PlayRateClampCycle | (0.8, 1.2) | 循环动画播放速率限制 |

### Step 4: 在 AnimBP 中引用

在角色 AnimBP 的 `AnimConfig` 属性上设置新创建的 DataAsset。

## DataAsset Category Structure / DataAsset 分类结构

```
Settings
├── RootYaw     — Root Yaw 偏移参数 (TurnInPlace 角度阈值、钳制范围)
├── Locomotion  — 移动参数 (死区、FootPlacement、IdleBreak 计时)
├── Aiming      — 瞄准参数 (HipFire覆盖、蹲伏射击恢复、ADS步态)
├── IK          — IK 参数 (手部IK、左手姿势覆盖)
├── Warping     — 扭曲参数 (StrideWarping混合、OrientationWarping速度)
├── PlayRate    — 播放速率钳制
├── Jump        — 跳跃参数 (着陆距离阈值、到顶点时间阈值)
└── Blend       — 混合配置 (上半身BlendProfile)

Curve
└── 所有动画曲线名称映射 (可自定义曲线命名约定)

Animation
├── Idle / Pose / Start / Stop / Cycle / Pivot / Sprint
├── TurnInPlace / Jump / Aim / Lean / AdditiveHurt
└── 每个分类都有启用开关和 EditCondition
```

## Animation Curve Requirements / 动画曲线要求

以下动画资产必须包含对应的曲线：

| 动画 | 必需曲线 |
|------|---------|
| TurnInPlace 系列 | `TurnYawWeight`, `RemainingTurnYaw` |
| Start/Stop/Pivot 系列 | `Distance` |
| Jump_Land | `GroundDistance` |
| 任意动画 | `DisableRightHandIK`, `DisableLeftHandIK`, `DisableLegIK` (可选) |

曲线名称可在 DataAsset 的 `Curve` 分类中自定义。

## Troubleshooting / 常见问题

**验证错误**: 在 DataAsset 上右键 → Validate Asset，检查缺失的动画引用和曲线。
可通过 `ValidationFlags` 位掩码控制哪些类别需要验证（不支持的动画类型不要勾选）。
