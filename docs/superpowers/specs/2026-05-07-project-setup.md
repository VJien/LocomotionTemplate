# 构建与环境指南

## 环境要求

| 项 | 值 |
|---|---|
| 引擎 | UE 5.7 |
| 引擎路径 | `F:\UnrealEngine\UE_5.7` |
| 项目路径 | `H:\UE_Projects\Locomotion\LocomotionTemplate` |
| IDE | Visual Studio 2022 / Rider |
| 平台 | Win64 |

## 构建

1. 右键 `LocomotionTemplate.uproject` → Generate Visual Studio project files
2. 打开 `LocomotionTemplate.sln`，编译 Development Editor | Win64
3. 或直接通过引擎 BuildRecipe 编译

## 不使用 Git Worktree

本项目不使用worktree，所有开发在主分支进行。技能文档中涉及worktree的步骤跳过。

## LyraAnim 插件

- 路径: `Plugins/LyraAnim/`
- 有独立 `.git`，是子模块
- 提供核心AnimNode: Distance Matching, Orientation Warping, Stride Warping等
- 修改插件代码前确认是否需要同步到上游

## 模块依赖

```
LocomotionTemplate (Runtime)
├── LyraAnim (Runtime, PreLoadingScreen)
├── LyraAnimEditor (Editor)
├── GameplayAbilities (Engine)
├── AnimationWarping (Engine)
├── AnimationLocomotionLibrary (Engine)
└── EnhancedInput (Engine)
```

## 源码结构

```
Source/LocomotionTemplate/
├── Core/
│   ├── LTGameMode          # 默认绑定 Character/Controller/HUD/PlayerState
│   ├── LTGameInstance      # 空壳，后续加系统初始化
│   └── LTPlayerState       # 空壳，后续加状态数据
├── Character/
│   ├── LTCharacter         # 角色基类，预留EnhancedInput
│   ├── LTPlayerController  # 控制器，预留MappingContext注入
│   └── LTHUD               # 空壳，后续加调试面板
└── LocomotionTemplate.Build.cs  # 模块构建配置
```
