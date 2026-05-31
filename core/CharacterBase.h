#ifndef CHARACTERBASE_H
#define CHARACTERBASE_H

#include "GameTypes.h"
#include "Weapon.h"
#include "Artifact.h"
#include <array>
#include <memory>

class CharacterBase {
public:
    CharacterBase(const QString &name, ElementType element, WeaponType weaponType);
    virtual ~CharacterBase() = default;

    // --- Identity ---
    QString name() const { return m_name; }
    ElementType element() const { return m_element; }
    WeaponType weaponType() const { return m_weaponType; }
    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
    int persistentId() const { return m_persistentId; }
    void setPersistentId(int pid) { m_persistentId = pid; }
    static int nextPersistentId();
    TeamSide side() const { return m_side; }
    void setSide(TeamSide s) { m_side = s; }

    // --- Base stats (can be modified by constellation) ---
    double baseAtk() const;
    double baseHp() const { return m_baseHp; }
    double baseEleMastery() const { return m_baseEleMastery; }
    double baseCritRate() const { return m_baseCritRate; }
    double baseCritDmg() const { return m_baseCritDmg; }
    double baseDmgBonus() const;
    double baseEnergyRcVal() const;

    int energyMax() const { return m_energyMax; }
    int currentEnergy() const { return m_currentEnergy; }
    void setCurrentEnergy(int e) { m_currentEnergy = qBound(0, e, m_energyMax); }
    void addEnergy(int e) { m_currentEnergy = qBound(0, m_currentEnergy + e, m_energyMax); }
    bool energyFull() const { return m_currentEnergy >= m_energyMax; }

    // --- Calculated stats ---
    double atk() const;
    double hp() const;
    double maxHp() const;
    double energyRcVal() const;
    double eleMastery() const;
    double critRate() const;
    double critDmg() const;
    double dmgBonus() const;

    // --- Constellation ---
    int constellation() const { return m_constellation; }
    void setConstellation(int c) { m_constellation = qBound(0, c, 6); }
    virtual double constellationNormalAtkBonus() const { return 0; }
    virtual double constellationBurstBonus() const { return 0; }
    virtual double constellationBaseAtkMultiplier() const { return 1.0; }
    virtual double constellationBaseEnergyRcValMultiplier() const { return 1.0; }
    virtual double constellationBaseDmgBonus() const { return 0; }

    // --- Equipment ---
    Weapon weapon() const { return m_weapon; }
    void setWeapon(const Weapon &w);
    bool hasWeapon() const { return m_hasWeapon; }
    void clearWeapon();

    const std::array<Artifact, MAX_ARTIFACT_SLOTS> &artifacts() const { return m_artifacts; }
    Artifact artifact(ArtifactSlot slot) const;
    void setArtifact(ArtifactSlot slot, const Artifact &a);
    void clearArtifact(ArtifactSlot slot);
    bool hasArtifact(ArtifactSlot slot) const;

    // Sum all artifact stats
    double artifactHpNum() const;
    double artifactAtkNum() const;
    double artifactAtkPer() const;
    double artifactHpPer() const;
    double artifactEnergyRc() const;
    double artifactCritDmg() const;
    double artifactCritRate() const;
    double artifactEleMastery() const;
    double artifactDmgBonus() const;

    // --- Skills ---
    struct SkillInfo {
        QString name;
        double multiplier;
        ElementType elementAttachment;
        int energyRecovery;
    };

    virtual SkillInfo normalAttackInfo() const = 0;
    virtual SkillInfo burstInfo() const = 0;

    double normalAttackMultiplier() const;
    double burstMultiplier() const;

    // --- Damage calculation ---
    double calcDamage(double skillMultiplier, bool crit, double dmgBonusOverride = -1) const;
    double calcAmplifyingDamage(double skillMultiplier, double reactionMult, double masteryZone, bool crit) const;

    // --- Combat state ---
    double currentHp() const { return m_currentHp; }
    void setCurrentHp(double hp) { m_currentHp = qBound(0.0, hp, maxHp()); }
    void takeDamage(double dmg) { m_currentHp = qMax(0.0, m_currentHp - dmg); }
    bool isAlive() const { return m_currentHp > 0; }
    bool isFrozen() const { return m_frozenTurns > 0; }
    void setFrozen(int turns) { m_frozenTurns = turns; }
    void tickFrozen() { if (m_frozenTurns > 0) --m_frozenTurns; }

    // Shield from Crystallize
    double shieldStrength() const { return m_shieldStrength; }
    void addShield(double pct) { m_shieldStrength = 1.0 - (1.0 - m_shieldStrength) * (1.0 - pct); }
    void clearShield() { m_shieldStrength = 0; }
    double absorbDamage(double dmg);

    // --- Elemental Aura ---
    QVector<ElementalAura> &auras() { return m_auras; }
    const QVector<ElementalAura> &auras() const { return m_auras; }
    void applyAura(ElementType elem, int units = 1);
    void tickAuras();
    bool hasAura(ElementType elem) const;

    // Quicken state
    bool inQuicken() const { return m_quickenTurns > 0; }
    void setQuicken(int turns) { m_quickenTurns = turns; }
    void tickQuicken() { if (m_quickenTurns > 0) --m_quickenTurns; }

    // Burning DoT
    int burningTurns() const { return m_burningTurns; }
    void setBurning(int turns) { m_burningTurns = turns; }
    void tickBurning() { if (m_burningTurns > 0) { --m_burningTurns; applyAura(ElementType::Pyro, 1); } }

    // Grid position during battle
    GridPos gridPos() const { return m_gridPos; }
    void setGridPos(GridPos p) { m_gridPos = p; }

    int deployIndex() const { return m_deployIndex; }
    void setDeployIndex(int i) { m_deployIndex = i; }

    // Clone
    virtual std::unique_ptr<CharacterBase> clone() const = 0;

protected:
    QString m_name;
    ElementType m_element;
    WeaponType m_weaponType;

    // Base stats
    double m_baseAtk = 322;
    double m_baseHp = 18000;
    double m_baseEleMastery = 0;
    double m_baseCritRate = 0.22;
    double m_baseCritDmg = 0.884;
    double m_baseDmgBonus = 0;
    double m_baseEnergyRcVal = 8;
    int m_energyMax = 40;

    int m_constellation = 0;
    int m_currentEnergy = 0;

    Weapon m_weapon;
    bool m_hasWeapon = false;
    std::array<Artifact, MAX_ARTIFACT_SLOTS> m_artifacts;
    bool m_hasArtifactSlot[MAX_ARTIFACT_SLOTS] = {false};

    double m_currentHp = 18000;
    int m_frozenTurns = 0;
    double m_shieldStrength = 0;

    QVector<ElementalAura> m_auras;
    int m_quickenTurns = 0;
    int m_burningTurns = 0;

    GridPos m_gridPos;
    int m_deployIndex = -1;
    int m_id = -1;
    int m_persistentId = -1;
    TeamSide m_side = TeamSide::Player;
};

#endif // CHARACTERBASE_H
