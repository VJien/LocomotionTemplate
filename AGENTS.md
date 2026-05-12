# LocomotionTemplate - 项目总说明

新对话开始时必须先读本文件。这里记录项目路径、构建方式、文档规范、代码边界和常用约定，保证上下文不中断。

## 项目性质

UE5 Locomotion教学演示项目。面向游戏开发者的公开课配套Demo，逐步从单Loop动画搭建到完整角色运动系统。

## 引擎与构建

- **引擎版本**: UE 5.7
- **引擎路径**: `F:\UnrealEngine\UE_5.7`
- **项目路径**: `H:\UE_Projects\Locomotion\LocomotionTemplate`
- **Workspace根目录**: `H:\UE_Projects\Locomotion`
- **构建方式**: 优先通过IDE/引擎Build.bat编译，不直接调用裸UBT dotnet命令
- **Worktree**: 不需要，本项目不使用git worktree

### 编译命令

推荐命令（PowerShell）：

```powershell
& "F:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat" LocomotionTemplateEditor Win64 Development -Project="H:\UE_Projects\Locomotion\LocomotionTemplate\LocomotionTemplate.uproject" -WaitMutex -architecture=x64
```

预期成功输出：

```text
Result: Succeeded
```

注意：

- 不要用 `Intermediate\ProjectFiles\LocomotionTemplate.uproject`，该路径不存在，会导致MSBuild失败。
- `.sln` 可以用IDE打开，但命令行MSBuild可能指向Intermediate里的临时uproject。若MSBuild失败，使用上面的Build.bat真实uproject命令验证。
- 不使用git worktree。

## 目录结构

```text
H:/UE_Projects/Locomotion/
├── AGENTS.md                  # 本文件，新对话入口
├── CLAUDE.md                  # Claude Code兼容入口，指向AGENTS.md
├── docs/                      # 所有文档都写外层docs，不写项目内docs
│   ├── superpowers/specs/     # 设计规格文档
│   ├── superpowers/plans/     # 实现计划
│   └── *.md / *.pptx          # 课程材料、PPT草稿等
LocomotionTemplate/
├── Config/                    # 引擎配置
├── Content/                   # 蓝图/资产（二进制，不可文本diff）
├── Plugins/LyraAnim/          # 动画插件 - 核心依赖
│   ├── Content/               # 动画资产
│   └── Source/
│       ├── LyraAnim/          # Runtime模块
│       └── LyraAnimEditor/    # Editor模块
├── Source/LocomotionTemplate/
│   ├── Core/                  # GameMode、GameInstance、ProjectSettings、DebugSubsystem
│   ├── Character/             # Character、PlayerController、HUD
│   ├── Data/                  # 项目DataAsset类型
│   └── UI/                    # Slate DebugPanel
└── LocomotionTemplate.uproject
```

## 类命名规范

| 前缀 | 基类 | 用途 |
|------|------|------|
| ALT | AActor派生 | GameMode、Character、Controller、HUD、PlayerState |
| ULT | UObject派生 | GameInstance、DataAsset、组件等 |

## 文档规范

### 文档目录

所有项目文档写到外层 `H:\UE_Projects\Locomotion\docs`，不要写到 `LocomotionTemplate/docs`。

| 路径 | 用途 |
|------|------|
| `docs/superpowers/specs/` | 功能设计规格，先设计后实现 |
| `docs/superpowers/plans/` | 实现计划，拆成可执行任务 |
| `docs/stage-*.md` | 课程章节/PPT草稿 |
| `docs/*.pptx` | PPT文件 |

### 文档同步（Obsidian备份）

docs目录需阶段性同步到Obsidian作为备份：

```powershell
robocopy "H:\UE_Projects\Locomotion\docs" "D:\BaiduSyncdisk\Obsidian\work\locomotion" /E /MIR /DCOPY:T /R:1 /W:1
```

- 时机：docs下文档有新增或修改后，阶段性执行同步。
- `/MIR` 增量镜像，仅复制变更。

### 文档语言

- 文档默认使用中文。
- 技术术语保留英文原文，如 `DataAsset`、`AnimBlueprint`、`LinkedAnimLayer`、`SkeletalMesh`。
- 标题中文为主，可保留必要英文类名。
- 新增设计文档命名：`YYYY-MM-DD-<feature-name>-design.md`。
- 新增计划文档命名：`YYYY-MM-DD-<feature-name>.md`。

### 文档流程

1. 新功能先写 `docs/superpowers/specs/` 设计文档。
2. 用户确认设计后，再写 `docs/superpowers/plans/` 实现计划。
3. 实现计划确认后再改代码。
4. 文档和代码都以最小可用范围为准，不写无关扩展。

## 代码工作规则

- C++代码为主，蓝图仅用于资产引用和UI配置。
- `Content/` 下资产为二进制，不能用文本diff审查。
- 不主动改 `Content/` 资产，除非用户明确要求。
- 不改LyraAnim插件，除非需求明确必须改；该插件有独立git历史。
- 最小正确改动优先，不做无关重构。
- 遇到已有未提交改动，不回滚、不覆盖；只改当前任务相关文件。
- 手写代码编辑使用patch，不用脚本批量改写源码。

## 依赖插件

| 插件 | 来源 | 说明 |
|------|------|------|
| LyraAnim | Plugins/ | 项目自带，提供Distance Matching、Orientation Warping等AnimNode |
| GameplayAbilities | Engine | LyraAnim依赖 |
| AnimationWarping | Engine | 骨骼扭曲 |
| AnimationLocomotionLibrary | Engine | ALL节点库 |
| EnhancedInput | Engine | 输入系统 |

## 当前系统概览

### 核心C++模块

| 路径 | 说明 |
|------|------|
| `Source/LocomotionTemplate/Core/` | GameMode、GameInstance、PlayerState、ProjectSettings、DebugSubsystem |
| `Source/LocomotionTemplate/Character/` | Character、PlayerController、HUD |
| `Source/LocomotionTemplate/UI/` | Slate DebugPanel |
| `Source/LocomotionTemplate/Data/` | 项目DataAsset类型 |
| `Source/LocomotionTemplate/Character/Locomotion/` | Locomotion相关类型 |

### DebugPanel

- 角色默认使用Strafe模式：`bStrafeMovement=true`、`bUseControllerRotationYaw=true`、`bOrientRotationToMovement=false`。
- HUD顶部提示条动态显示当前模式：`Mode: Strafe` 或 `Mode: Orient To Movement`。
- `ALTCharacter::DebugPanelAction` 绑定输入。
- `ALTHUD` 负责显示/隐藏 `SLTDebugPanel`。
- `SLTDebugPanel` 是Slate面板，显示运动参数、Camera Arm长度、动画开关和角色切换按钮。
- CMC/Camera参数范围来自 `ULTProjectSettings`。

### 角色Mesh切换

- `ULTCharacterSet` 是每个角色一份的DataAsset。
- Project Settings -> `Locomotion Template` -> `CharacterSets` 配置角色DA数组。
- DebugPanel按数组生成Button。
- 点击Button调用 `ALTCharacter::ApplyCharacterSet(Index)`。
- 切换内容：`SkeletalMesh`、`AnimClass`、`LinkedAnimClasses`。

## 技术路线（对应教学阶段）

```text
Loop → Start → Stop → AimOffset → TurnInPlace → Pivot → Jump → Lean → FootIK → MotionMatching
```

每个阶段对应课程一个章节，Demo中提供开关面板逐步启用。

## 注意事项

- 新对话优先读 `AGENTS.md`；Claude Code环境也可从 `CLAUDE.md` 跳转到本文件。
- 需要了解已实现功能时，先看相关 `docs/superpowers/specs/` 和 `docs/superpowers/plans/`，再看代码。
- 编译验证使用上方Build.bat命令。
- 所有新增说明文档写中文，放外层 `docs/`。
