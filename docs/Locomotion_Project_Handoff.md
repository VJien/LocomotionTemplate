# Locomotion PPT 项目交接说明

## 项目目标

本项目是在 Unreal Engine / UE5 语境下制作一套游戏角色 Locomotion 技术分享 PPT。

定位不是简单填充文字，而是做成偏技术分享 / GDC 风格的课程型演示稿：以问题驱动章节，以图示、视频、Demo 占位和少量关键文字承载信息，详细解释尽量放到讲稿备注或口播中。

推荐主标题方向：

```text
游戏角色运动系统设计
```

推荐副标题方向：

```text
从基础 Loop 到完整 Locomotion 的深入拆解
```

“行走的艺术”可以作为副标题、封面小字或宣传语，但不建议单独作为主标题，因为它偏文艺，技术辨识度弱于“游戏角色运动系统设计”。

## 当前目录

主要文档目录：

```text
H:\UE_Projects\Locomotion\docs
```

资源目录：

```text
H:\UE_Projects\Locomotion\docs\resouce
```

注意：目录名当前拼写是 `resouce`，不是 `resource`。后续脚本和 PPT 资源引用都按这个路径处理。

## 已有章节文档

当前章节已经从 A 到 L 基本完整：

```text
stage-A-opening.md
stage-B-loop.md
stage-C-start.md
stage-D-stop.md
stage-E-aim-offset.md
stage-F-turn-in-place.md
stage-G-pivot.md
stage-H-jump.md
stage-I-lean.md
stage-J-foot-ik.md
stage-K-frontier.md
stage-L-summary.md
```

整体结构：

```text
A 开场
B Loop
C Start
D Stop
E Aim Offset
F Turn In Place
G Pivot
H Jump
I Lean
J Foot IK
K 前沿技术
L 总结
```

整体主线：

```text
让角色动起来
→ 解决滑步
→ 解决启动和停止
→ 解决朝向、瞄准、原地转向
→ 解决运动中急转、跳跃、身体倾斜、地形适配
→ 引到 Motion Matching 等前沿方向
→ 总结完整 Locomotion 搭建路径
```

## 重要设计判断

1. PPT 不应逐字照搬 md。
   md 是讲稿草稿，PPT 应该压缩成“每页一个观点 + 少量关键词 + 图示/视频/Demo 承载信息”。

2. 页面文字建议控制在：
   - 标题：一个明确问题或结论
   - 正文：3 到 5 个要点
   - 复杂解释：放备注或口播

3. 视频页和 Demo 页应尽量留大画面。
   不要在视频页堆太多解释文字。

4. 技术图应尽量用可编辑图形。
   例如 Blend Space、状态机、Distance Matching、Foot IK 射线、Land + Recovery 分层等，适合用 PPT 形状画，不必全部依赖位图。

5. AI 图片适合用在：
   - 封面图
   - 通用背景图
   - 章节过渡页背景
   - 概念氛围图

6. AI 图片不适合替代精确技术图。
   技术图最好使用可编辑形状、UE 截图或真实 Demo 截图。

## 已生成 PPT

最早的 ABC 草稿：

```text
H:\UE_Projects\Locomotion\docs\Locomotion_ABC_Draft.pptx
```

说明：

- 用户已经手动修改过一部分。
- 后续不建议覆盖它。

基于 ABC 草稿复制并追加 D-L 后的完整结构稿：

```text
H:\UE_Projects\Locomotion\docs\Locomotion_ABCL_Draft.pptx
```

说明：

- 总页数：89 页。
- 前 27 页保留 ABC。
- 第 28 页开始追加 D-L。
- 追加顺序已验证：
  - D Stop
  - E Aim Offset
  - F Turn In Place
  - G Pivot
  - H Jump
  - I Lean
  - J Foot IK
  - K 前沿技术
  - L 总结
- 这版是结构完整稿，图和视频位置多为占位，后续需要继续视觉精修。

## 已生成/使用的资源

资源目录中已有图片：

```text
bg_common_1.png
bg_common_2.png
bg_title.png
bg_trans_data.png
bg_trans_idle.png
bg_trans_stop.png
bg_trans_tip.png
```

用途理解：

- `bg_title.png`：封面背景。
- `bg_common_1.png` / `bg_common_2.png`：普通内容页背景。
- `bg_trans_idle.png`：Loop / Idle / Start 类过渡页可用。
- `bg_trans_stop.png`：Stop / Pivot / 急刹车类过渡页可用。
- `bg_trans_data.png`：数据、前沿、Motion Matching 类页面可用。
- `bg_trans_tip.png`：Turn In Place / Aim Offset 类页面可用。

用户提到：背景图后续可能自己再调整，所以 PPT 当前背景不是最终定稿。

## 已创建脚本

工作区：

```text
C:\Users\zld\Documents\New project
```

相关脚本：

```text
C:\Users\zld\Documents\New project\tools\create-locomotion-abc-ppt.ps1
C:\Users\zld\Documents\New project\tools\append-locomotion-d-to-l.ps1
```

其中：

- `create-locomotion-abc-ppt.ps1`：早期生成 ABC 草稿。
- `append-locomotion-d-to-l.ps1`：复制 `Locomotion_ABC_Draft.pptx`，生成 `Locomotion_ABCL_Draft.pptx`，并从第 28 页追加 D-L。

注意：

- Windows PowerShell 5 对 UTF-8 脚本编码敏感。
- 脚本需要保存为 UTF-8 BOM，否则中文字符串可能乱码。
- PowerPoint COM 可用，曾用于生成和验证 PPT。

## 当前 PPT 优化建议

后续如果继续精修，优先处理这些方向：

1. 封面
   - 主标题换成更明确的技术标题。
   - 推荐：`游戏角色运动系统设计`。
   - 副标题建议：`从基础 Loop 到完整 Locomotion 的深入拆解`。

2. 章节过渡页
   - 保持单标题为主。
   - 背景可以用主题图，不要堆文字。

3. A4 总览脑图
   - 建议做成完整课程地图。
   - 结尾 L1 再呼应这张图。

4. B Loop
   - Blend Space、方向覆盖、滑步、Stride Warping 要多用图。
   - 文字进一步压缩。

5. C Start / D Stop
   - Distance Matching 是核心，建议用统一视觉语言解释：

```text
代码预测距离
→ 查 Distance Curve
→ 定位动画播放时间
→ 脚步和位移对齐
```

6. H Jump
   - `Land + Recovery Additive` 是高价值页面。
   - 建议做成分层图，而不是普通 bullet 页。

7. J Foot IK
   - 射线检测、IK 目标、骨盆补偿适合做成一张清晰流程图。

8. K 前沿技术
   - Motion Matching 不要喧宾夺主。
   - 作为“传统方案天花板之后的进阶方向”即可。

## 后续制作原则

如果在新对话或新项目中继续，请遵守：

1. 不要覆盖用户已经手动修改过的 PPT。
2. 修改现有 PPT 时，优先另存新版本。
3. 不要重新生成 A-C，除非用户明确要求。
4. 新增或精修页面时，尽量从当前完整稿 `Locomotion_ABCL_Draft.pptx` 继续。
5. 背景图可以先保守使用，用户后续可能会自己替换。
6. 代码或脚本改动应尽量小范围，避免破坏已有生成逻辑。
7. 如果需要读取 H 盘目录，通常要申请权限。

## 当前最合理的下一步

建议下一步不是继续增加内容，而是做“精修第一轮”：

```text
目标：把结构稿变成可看的技术演示稿。
范围：优先精修 A-D 或 A-F。
方式：每页减少文字，增加图示，统一标题语言和视觉层级。
输出：另存为 Locomotion_ABCL_Refined_v1.pptx。
```

优先精修页面类型：

- 封面
- 课程总览脑图
- 阶段过渡页
- Distance Matching 解释页
- 方案对比页
- Demo / 视频占位页

## 给后续 Codex 的一句话上下文

这是一个 UE5 游戏角色 Locomotion 技术分享 PPT 项目，用户已经写好了 A-L 的 md 章节草稿，并手动改过 ABC 草稿。不要重做前面内容；应基于 `Locomotion_ABCL_Draft.pptx` 继续精修，保持技术分享/GDC 风格，把 md 内容压缩成适合 PPT 的视觉表达。
