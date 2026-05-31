# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 构建与运行

Qt 6.10.2 安装在 `C:/Qt/6.10.2/`，包含 `mingw_64` 和 `msvc2022_64` 两套工具链。项目要求 C++17（`CMAKE_CXX_STANDARD 17`）。

```bash
# 配置 & 编译 (MinGW)
cmake -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/mingw_64"
cmake --build build

# 配置 & 编译 (MSVC)
cmake -B build/msvc -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
cmake --build build/msvc

# 运行 (MinGW — 需要将 Qt DLL 加入 PATH)
export PATH="/c/Qt/6.10.2/mingw_64/bin:$PATH"
./build/TeyvatTatics.exe
```

项目当前主要使用 MinGW。build 目录下已有 `build/Desktop_Qt_6_10_2_MinGW_64_bit-Debug/` 和 `build/Desktop_Qt_6_10_2_MSVC2022_64bit-Debug/` 两个 Qt Creator 生成的构建目录。

## 架构概览

这是一个原神题材的单人 PvE 自走棋游戏，使用 Qt6 Widgets 构建，双层架构。

### `core/` — 游戏引擎层（除 QtCore 信号/槽外不依赖 Qt UI）

**中心控制器：** `GameEngine`（`QObject`）拥有所有子系统并暴露 `Q_PROPERTY`。所有游戏状态变更都经过它。在 `startNewGame()` 中通过 `createAllCharacters()`（定义于 `TestCharacter.h`）创建角色池。

**子系统：**
- `Board`（`QObject`）— 8×8 的原始 `CharacterBase*` 指针网格。敌方区域：0–3 行，己方部署区域：4–7 行。移动/放置时发出 `boardChanged()`。
- `Team` — 追踪某一方棋子并计算元素共鸣加成（voltage/flames/water/greenery/ice，各需 2 名同元素角色）。
- `ElementSystem` — 通过每个角色身上的元素附着队列（最多 3 单位 FIFO，持续 3 回合）处理 12 种元素反应。使用回调（`DamageCallback`/`SplashCallback`）解耦伤害应用。
- `Shop`（`QObject`）— 生成随机 `ShopItem`（角色/武器/圣遗物）。刷新费用递增。支持锁定槽位。
- `AIController` — `Difficulty` 枚举（Easy/Normal/Hard）影响走位和攻击目标评分。战斗中双方均由 AI 控制。
- `SaveManager` — 纯静态工具类，通过 `QJsonObject` 进行游戏状态的序列化/反序列化。`GameEngine` 将其声明为 `friend`。

**数据类：**
- `CharacterBase` — 角色抽象基类。所有属性计算（ATK、HP、暴击、伤害公式、圣遗物求和）均在此处。命之座通过虚方法（`constellationNormalAtkBonus()` 等）由子类覆写。具体子类：`KeQing`、`TestCharacter`。
- `Weapon` — 值类型。星级 + 职业 → ATK 查找表在 `GameTypes.h::weaponBaseAtk()`。
- `Artifact` — 值类型。槽位 + 主词条 → 属性值查找表在 `GameTypes.h::ArtifactStatBase`。
- `GridPos` — 棋盘坐标，含曼哈顿距离、切比雪夫距离、邻居遍历等工具方法。
- `BattleAction` / `ReactionResult` / `ElementalAura` — 战斗相关结构体。

**角色池工厂：** `createAllCharacters()`（`TestCharacter.cpp:62`）创建 1 个刻晴 + 6 个 test_N 角色（test_1 风单手剑 ~ test_6 水单手剑）。所有 test 角色基础属性与刻晴一致，仅元素与武器类型不同。

### `ui/` — Qt Widgets 界面层

- `MainWindow` — 顶层窗口，组合所有 UI 组件并连接信号。左侧面板：难度下拉 → InfoPanel → ShopWidget → BattleLogWidget。中央：BoardWidget + StorageBar。背包/合成按钮可弹出对应面板。
- `BoardWidget` — `QGraphicsView`/`QGraphicsScene`。拖放使用自定义 `DragDropMimeData`，MIME 类型为 `"teyvat/piece-drag"`，携带 `sourceType`（`"storage"`/`"board"`/`"backpack"`）、索引、背包索引等元数据。
- `PieceItem` — 每个棋盘角色的 `QGraphicsObject`。绘制顺序：己方绿色/敌方红色底座环 → 元素渐变色圆 → 命之座颜色名称 → 血条 → 能量条。
- `ShopWidget` / `StorageBar` / `InfoPanel` — 商店、储存栏、角色详情面板。
- `BattleLogWidget` — 使用 `insertHtml()` 输出带颜色的战斗日志。`BattleAction` 携带 `attackElement` 和 `reactionElement` 用于颜色编码。
- `BackpackWidget` — 10 格背包，存放武器和圣遗物。支持点击查看、右键出售、拖拽到角色装备槽。
- `MergePanel` — 命之座合成面板：两个输入槽（接受拖放）+ 一个输出槽 + 合并按钮。同角色合成后命之座 = a + b + 1（上限 6）。

### 关键设计决策

- **角色所有权**：玩家角色以原始指针存储在 `m_storage`（`QVector<CharacterBase*>`）或 Board `m_grid` 中。敌方角色以 `std::unique_ptr` 存储在 `m_enemyCharacters`（`std::vector`，非 `QVector`，因为 `QVector` 对仅移动类型支持不可靠）中，每轮清除。
- **ShopItem 移动语义**：包含原始 `CharacterBase*` 指针，拷贝构造函数已删除，自定义移动构造函数将源指针置空。使用 `std::vector<ShopItem>`，**绝不**使用 `QVector<ShopItem>`。
- **圣遗物槽位守卫**：`ArtifactSlot` 枚举以 `NONE` 作为最后一个值（=5）。`m_artifacts` 为 `std::array<Artifact, MAX_ARTIFACT_SLOTS>`（大小 5）。`m_hasArtifactSlot[i]` 守卫圣遗物属性求和——默认构造的 Artifact 槽位为 NONE，贡献零属性。遍历时使用 `for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i)` 并转换为 `ArtifactSlot`，不要假设 Flower=0。
- **武器默认行为**：角色构造时自动装备对应职业的 2 星武器（`CharacterBase` 构造函数）。`unequipWeapon()` 将旧武器放入背包并装备默认 2 星武器——角色始终持有武器。
- **购买退款**：`buyShopItem()` 若储存栏已满会自动退款原石。
- **战后重置**：`setupBattle()` 在重置 HP 前将玩家位置保存到 `m_savedPlayerPositions`。`startPreparationPhase()` 通过移除所有棋子、回满 HP、再放回保存坐标的方式恢复 HP 和位置。
- **战斗日志颜色编码**：`BattleLogWidget` 使用 `insertHtml()`。`BattleAction` 携带 `attackElement` 和 `reactionElement`。反应颜色映射见 `GameTypes.h::reactionElementColor()`。
- **命之座合成**：`mergeCharactersDirect(a, b)` 创建一个新的合并角色（命之座 = a.命座 + b.命座 + 1，上限 6），不删除原始角色——调用者负责管理生命周期。`findDuplicates()` 扫描储存栏和棋盘，找出同名角色配对以驱动高亮 UI。
- **存档系统**：`SaveManager` 序列化所有游戏状态（资源、回合、比分、棋盘布局、储存栏、背包、商店物品）为 JSON，通过 `GameEngine` 的 friend 访问权限直接读写私有成员。
- **特效接口**：`effectPlay(type, pos)` 在测试阶段仅记录日志，可后续接入实际特效系统。

### 游戏流程

1. `startPreparationPhase()` → 清除敌方、恢复玩家 HP/位置、计算利息、刷新商店（免费）
2. 玩家购买物品，从储存栏拖拽到棋盘（4–7 行），可选进行合成/装备/出售
3. 玩家点击"开始战斗" → `startBattlePhase()` → `generateEnemyTeam()`（随机角色 + 随机星级武器）→ `setupBattle()`（保存位置、重置 HP/能量/附着）
4. 自动战斗通过 `QTimer` → `executeBattleRound()`：每个棋子 AI 移动 → 获得能量 → AI 选择目标攻击 → 触发元素反应 → 结算溅射/控制
5. `onBattleFinished()` → 发放资源，检查 6 胜/11 轮条件，回到步骤 1

两种战斗执行模式：
- `executeBattleRound()` — 整轮执行（己方全部行动 → 敌方全部行动），用于 QTimer 驱动
- `executeBattleStep()` — 逐棋子交错执行（己方 1 个 → 敌方 1 个），各方向棋子全部行动后切换回合

### 已知问题跟踪

`problem/` 目录记录已知问题和待实现功能：
- `problems.md` — 初始问题列表（HP 初始值错误、拖拽闪退、功能缺失等）
- `problem_1.md` — 后续反馈（商店刷新扣费异常、合成面板在窗口外等问题）
- `problem_2.md` — 空文件，待填充

### 常见注意事项

- 在 UI 头文件中添加 include 时，`GameTypes.h` **不**包含 `Artifact.h` 或 `Weapon.h`——如果头文件使用了这些类型，需要显式 include。
- `ShopItem` 是仅移动类型。使用 `std::vector<ShopItem>`，**绝不**使用 `QVector<ShopItem>`。
- Qt6 中 `QGraphicsItem::setCursor()` 接受 `const QCursor&` 而非 `Qt::CursorShape`。需用 `QCursor(Qt::OpenHandCursor)` 包装。
- Qt6 中 `QDrag` 构造函数接受 `QObject*`——需要显式传入 `QWidget*`。
- 所有使用信号/槽的 QObject 子类必须在头文件中声明 `Q_OBJECT` 宏，且头文件需被 MOC 处理（已通过 `CMAKE_AUTOMOC ON` 自动处理）。
- 回复时需要使用简体中文。
