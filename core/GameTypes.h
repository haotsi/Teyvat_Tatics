#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <QString>
#include <QVector>
#include <QPair>
#include <cmath>

enum class ElementType {
    Hydro, Pyro, Electro, Cryo, Dendro, Anemo, Geo, None
};

inline QString elementName(ElementType e) {
    switch (e) {
        case ElementType::Hydro:  return QStringLiteral("水");
        case ElementType::Pyro:   return QStringLiteral("火");
        case ElementType::Electro: return QStringLiteral("雷");
        case ElementType::Cryo:   return QStringLiteral("冰");
        case ElementType::Dendro: return QStringLiteral("草");
        case ElementType::Anemo:  return QStringLiteral("风");
        case ElementType::Geo:    return QStringLiteral("岩");
        case ElementType::None:   return QStringLiteral("无");
    }
    return {};
}

enum class WeaponType {
    Sword, Claymore, Catalyst, Bow, Polearm
};

inline QString weaponTypeName(WeaponType w) {
    switch (w) {
        case WeaponType::Sword:    return QStringLiteral("单手剑");
        case WeaponType::Claymore: return QStringLiteral("双手剑");
        case WeaponType::Catalyst: return QStringLiteral("法器");
        case WeaponType::Bow:      return QStringLiteral("弓");
        case WeaponType::Polearm:  return QStringLiteral("长柄武器");
    }
    return {};
}

enum class ArtifactSlot {
    Flower, Plume, Sands, Goblet, Circlet, NONE
};

constexpr int MAX_ARTIFACT_SLOTS = 5;

inline QString artifactSlotName(ArtifactSlot s) {
    switch (s) {
        case ArtifactSlot::Flower:  return QStringLiteral("花");
        case ArtifactSlot::Plume:   return QStringLiteral("羽毛");
        case ArtifactSlot::Sands:   return QStringLiteral("沙漏");
        case ArtifactSlot::Goblet:  return QStringLiteral("杯子");
        case ArtifactSlot::Circlet: return QStringLiteral("皇冠");
        case ArtifactSlot::NONE:    return QStringLiteral("无");
    }
    return {};
}

enum class ArtifactMainStat {
    HP_NUM, ATK_NUM, ATK_PER, HP_PER, ENERGY_RC,
    CRIT_DMG, CRIT_RATE, ELE_MASTERY, DMG_BONUS, NONE
};

inline QString statName(ArtifactMainStat s) {
    switch (s) {
        case ArtifactMainStat::HP_NUM:      return QStringLiteral("生命值(数字)");
        case ArtifactMainStat::ATK_NUM:     return QStringLiteral("攻击力(数字)");
        case ArtifactMainStat::ATK_PER:     return QStringLiteral("攻击力(百分比)");
        case ArtifactMainStat::HP_PER:      return QStringLiteral("生命值(百分比)");
        case ArtifactMainStat::ENERGY_RC:   return QStringLiteral("元素充能效率");
        case ArtifactMainStat::CRIT_DMG:    return QStringLiteral("暴击伤害");
        case ArtifactMainStat::CRIT_RATE:   return QStringLiteral("暴击率");
        case ArtifactMainStat::ELE_MASTERY: return QStringLiteral("元素精通");
        case ArtifactMainStat::DMG_BONUS:   return QStringLiteral("元素反应伤害加成");
        case ArtifactMainStat::NONE:        return QStringLiteral("无");
    }
    return {};
}

enum class ReactionType {
    None,
    Vaporize, Melt,
    Overload, ElectroCharged, Superconduct, Frozen,
    Swirl,
    Bloom, Burgeon, Hyperbloom,
    Quicken, Spread, Aggravate,
    Burning,
    Crystallize
};

inline QString reactionName(ReactionType r) {
    switch (r) {
        case ReactionType::None:            return QStringLiteral("无");
        case ReactionType::Vaporize:        return QStringLiteral("蒸发");
        case ReactionType::Melt:            return QStringLiteral("融化");
        case ReactionType::Overload:        return QStringLiteral("超载");
        case ReactionType::ElectroCharged:  return QStringLiteral("感电");
        case ReactionType::Superconduct:    return QStringLiteral("超导");
        case ReactionType::Frozen:          return QStringLiteral("冻结");
        case ReactionType::Swirl:           return QStringLiteral("扩散");
        case ReactionType::Bloom:           return QStringLiteral("原绽放");
        case ReactionType::Burgeon:         return QStringLiteral("烈绽放");
        case ReactionType::Hyperbloom:      return QStringLiteral("超绽放");
        case ReactionType::Quicken:         return QStringLiteral("原激化");
        case ReactionType::Spread:          return QStringLiteral("蔓激化");
        case ReactionType::Aggravate:       return QStringLiteral("超激化");
        case ReactionType::Burning:         return QStringLiteral("燃烧");
        case ReactionType::Crystallize:     return QStringLiteral("结晶");
    }
    return {};
}

enum class GamePhase {
    Preparation,
    Battle,
    GameOver
};

enum class TeamSide {
    Player,
    Enemy
};

// --- Board Constants ---
constexpr int BOARD_ROWS = 8;
constexpr int BOARD_COLS = 8;
constexpr int PLAYER_START_ROW = 5;  // rows 5-7 (0-indexed)
constexpr int ENEMY_START_ROW = 0;   // rows 0-3
constexpr int PLAYER_DEPLOY_ROWS = 4; // rows 5-8 in 1-indexed = rows 4-7 in 0-indexed

// --- Shop Constants ---
constexpr int CHARACTER_COST = 160;
constexpr int ARTIFACT_COST = 100;
constexpr int WEAPON_BASE_COST = 50; // +50 per star level above 2

constexpr int INITIAL_PRIMOGEMS = 6400;//***{6400 for demo;in fact,it should be 320}***
constexpr int INITIAL_MORA = 10000;//***{10000 for demo;in fact,it should be 1000}***
constexpr int WIN_PRIMOGEMS = 320;
constexpr int WIN_MORA = 1000;
constexpr int LOSE_PRIMOGEMS = 160;
constexpr int LOSE_MORA = 500;

constexpr int MAX_POPULATION = 4;
constexpr int INITIAL_POPULATION = 2;
constexpr int POPULATION_UPGRADE_COST = 160;
constexpr int STORAGE_CAPACITY = 10;

constexpr int TOTAL_ROUNDS = 11;
constexpr int WINS_NEEDED = 6;

constexpr int REFRESH_COST_BASE = 50;
constexpr int REFRESH_COST_INCREMENT = 25;

constexpr double INTEREST_RATE = 0.10;
constexpr double SELL_RATIO = 0.25;

// --- Reaction Constants ---
constexpr double REACTION_CONSTANT = 800.0;
constexpr double VAPORIZE_MULT = 1.8;
constexpr double MELT_MULT = 1.8;
constexpr double OVERLOAD_MULT = 3.0;
constexpr double ELECTRO_CHARGED_MULT = 2.0;
constexpr double ELECTRO_CHARGED_SPLASH_MULT = 0.5;
constexpr double SUPERCONDUCT_MULT = 1.5;
constexpr double SUPERCONDUCT_SPLASH_MULT = 0.6;
constexpr double SWIRL_MULT = 0.8;
constexpr double SWIRL_SPREAD_MULT = 0.4;
constexpr double BLOOM_MULT = 1.0;
constexpr double BLOOM_SELF_MULT = 0.6;
constexpr double BURGEON_MULT = 1.5;
constexpr double BURGEON_SELF_MULT = 0.6;
constexpr double HYPERBLOOM_MULT = 1.0;
constexpr double SPREAD_AGGRAVATE_MULT = 2.5;
constexpr double BURNING_MULT = 0.8;
constexpr int MAX_AURA_UNITS = 3;
constexpr int AURA_DURATION = 3; // turns

// --- Weapon ATK by star and type ---
inline int weaponBaseAtk(int stars, WeaponType type) {
    static const int table[4][5] = {
        // Sword, Claymore, Bow, Catalyst, Polearm
        {243, 243, 243, 243, 243}, // 2-star
        {448, 448, 402, 402, 433}, // 3-star
        {565, 565, 454, 454, 510}, // 4-star
        {674, 741, 542, 542, 608}  // 5-star
    };
    int row = qBound(0, stars - 2, 3);
    int col = static_cast<int>(type);
    return table[row][col];
}

// --- Artifact main stat base values ---
struct ArtifactStatBase {
    static double flowerHP()  { return 4780.0; }
    static double plumeATK()  { return 311.0; }
    static double sandsATKPer()    { return 0.466; }
    static double sandsHPPer()     { return 0.466; }
    static double sandsER()        { return 0.518; }
    static double sandsEM()        { return 187.0; }
    static double gobletATKPer()   { return 0.466; }
    static double gobletHPPer()    { return 0.466; }
    static double gobletEM()       { return 187.0; }
    static double gobletDmgBonus() { return 0.466; }
    static double circletATKPer()  { return 0.466; }
    static double circletHPPer()   { return 0.466; }
    static double circletCritDmg() { return 0.622; }
    static double circletCritRate(){ return 0.311; }
    static double circletEM()      { return 187.0; }
};

// Position on the board
struct GridPos {
    int row = 0;
    int col = 0;

    bool isValid() const {
        return row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS;
    }

    bool operator==(const GridPos &o) const { return row == o.row && col == o.col; }
    bool operator!=(const GridPos &o) const { return !(*this == o); }

    int manhattanDist(const GridPos &o) const {
        return std::abs(row - o.row) + std::abs(col - o.col);
    }

    int chebyshevDist(const GridPos &o) const {
        return std::max(std::abs(row - o.row), std::abs(col - o.col));
    }

    bool isAdjacent8(const GridPos &o) const {
        return chebyshevDist(o) == 1 && *this != o;
    }

    bool isAdjacent4(const GridPos &o) const {
        return manhattanDist(o) == 1;
    }

    QVector<GridPos> neighbors8() const {
        QVector<GridPos> result;
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                GridPos p{row + dr, col + dc};
                if (p.isValid()) result.append(p);
            }
        return result;
    }

    QVector<GridPos> neighbors4() const {
        QVector<GridPos> result;
        static const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
        for (auto &d : dirs) {
            GridPos p{row + d[0], col + d[1]};
            if (p.isValid()) result.append(p);
        }
        return result;
    }
};

// Elemental aura on a character
struct ElementalAura {
    ElementType element = ElementType::None;
    int remainingTurns = 0;
    int units = 1;
};

// Battle action result for logging
struct BattleAction {
    int attackerId = -1;
    int targetId = -1;
    GridPos fromPos;
    GridPos toPos;
    double damage = 0;
    bool crit = false;
    ReactionType reaction = ReactionType::None;
    QString skillName;
    bool moved = false;
    ElementType attackElement = ElementType::None;
    ElementType reactionElement = ElementType::None;
    TeamSide attackerSide = TeamSide::Player;
};

// --- Color helpers ---
inline QString constellationColor(int c) {
    switch (c) {
        case 6: return QStringLiteral("#FF2020"); // red
        case 5: return QStringLiteral("#FF8800"); // orange
        case 4: return QStringLiteral("#FFD700"); // gold
        case 3: return QStringLiteral("#A335EE"); // purple
        case 2: return QStringLiteral("#0070DD"); // blue
        case 1: return QStringLiteral("#1EFF00"); // green
        default: return QStringLiteral("#FFFFFF"); // white
    }
}

inline QString elementColorHex(ElementType e) {
    switch (e) {
        case ElementType::Hydro:  return QStringLiteral("#0080FF");
        case ElementType::Pyro:   return QStringLiteral("#FF5028");
        case ElementType::Electro: return QStringLiteral("#A050FF");
        case ElementType::Cryo:   return QStringLiteral("#8CDCFF");
        case ElementType::Dendro: return QStringLiteral("#50C83C");
        case ElementType::Anemo:  return QStringLiteral("#78C8B4");
        case ElementType::Geo:    return QStringLiteral("#C8A03C");
        default: return QStringLiteral("#CCCCCC");
    }
}

inline ElementType reactionElementColor(ReactionType r) {
    switch (r) {
        case ReactionType::Vaporize:
        case ReactionType::Melt:
        case ReactionType::Burning:       return ElementType::Pyro;
        case ReactionType::Overload:
        case ReactionType::ElectroCharged:
        case ReactionType::Superconduct:  return ElementType::Electro;
        case ReactionType::Frozen:        return ElementType::Cryo;
        case ReactionType::Swirl:         return ElementType::Anemo;
        case ReactionType::Bloom:
        case ReactionType::Burgeon:
        case ReactionType::Hyperbloom:
        case ReactionType::Quicken:
        case ReactionType::Spread:
        case ReactionType::Aggravate:     return ElementType::Dendro;
        case ReactionType::Crystallize:   return ElementType::Geo;
        default: return ElementType::None;
    }
}

#endif // GAMETYPES_H
