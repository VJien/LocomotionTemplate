# LyraAnim 插件参考

## 插件概览

LyraAnim是项目自带动画插件，提供Locomotion系统核心AnimNode。源自Epic Lyra Starter Game，本项目裁剪使用。

## 模块结构

```
Plugins/LyraAnim/Source/
├── LyraAnim/              # Runtime模块 (PreLoadingScreen加载)
│   ├── AnimNodes/         # 自定义AnimNode实现
│   └── ...
└── LyraAnimEditor/        # Editor模块
    └── ...
```

## 核心AnimNode（预期）

| 节点 | 用途 | 对应阶段 |
|------|------|---------|
| Distance Matching | 按距离驱动动画播放位置 | Start(C), Stop(D) |
| Stride Warping | 步幅调整匹配移动速度 | Loop(B) |
| Orientation Warping | 角色朝向扭曲，减少方向动画数 | Loop(B) |

> 注：具体节点名待阅读Source确认

## 依赖链

```
LyraAnim
├── GameplayAbilities      # GAS系统，AnimNode可能依赖其类型
├── AnimationWarping       # 骨骼扭曲基础
└── AnimationLocomotionLibrary  # ALL节点库（UE5.4+内置）
```

## 注意

- 插件有独立 `.git`，改前确认是否影响上游
- `Content/` 下有动画资产（二进制）
- `PreLoadingScreen` 加载阶段确保AnimNode在早期可用
