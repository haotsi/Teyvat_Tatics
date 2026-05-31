#include "CharacterBase.h"
#include <QtMath>
#include <QRandomGenerator>

CharacterBase::CharacterBase(const QString &name, ElementType element, WeaponType weaponType)
    : m_name(name), m_element(element), m_weaponType(weaponType)
{
    // Default weapon matching character's weapon type
    m_weapon = Weapon(m_weaponType, 2);
    m_hasWeapon = true;
    m_currentHp = maxHp();
}

void CharacterBase::setWeapon(const Weapon &w)
{
    m_weapon = w;
    m_hasWeapon = true;
}

Artifact CharacterBase::artifact(ArtifactSlot slot) const
{
    int idx = static_cast<int>(slot);
    if (idx < 0 || idx >= MAX_ARTIFACT_SLOTS) return Artifact();
    return m_artifacts[idx];
}

void CharacterBase::setArtifact(ArtifactSlot slot, const Artifact &a)
{
    int idx = static_cast<int>(slot);
    if (idx < 0 || idx >= MAX_ARTIFACT_SLOTS) return;
    m_artifacts[idx] = a;
    m_hasArtifactSlot[idx] = (a.slot() != ArtifactSlot::NONE);
}

void CharacterBase::clearArtifact(ArtifactSlot slot)
{
    int idx = static_cast<int>(slot);
    if (idx < 0 || idx >= MAX_ARTIFACT_SLOTS) return;
    m_artifacts[idx] = Artifact();
    m_hasArtifactSlot[idx] = false;
}

bool CharacterBase::hasArtifact(ArtifactSlot slot) const
{
    int idx = static_cast<int>(slot);
    if (idx < 0 || idx >= MAX_ARTIFACT_SLOTS) return false;
    return m_hasArtifactSlot[idx];
}

double CharacterBase::baseAtk() const { return m_baseAtk * constellationBaseAtkMultiplier(); }
double CharacterBase::baseDmgBonus() const { return m_baseDmgBonus + constellationBaseDmgBonus(); }
double CharacterBase::baseEnergyRcVal() const { return m_baseEnergyRcVal * constellationBaseEnergyRcValMultiplier(); }

double CharacterBase::atk() const
{
    return (baseAtk() + m_weapon.atk()) * (1.0 + artifactAtkPer()) + artifactAtkNum();
}

double CharacterBase::hp() const
{
    return m_currentHp;
}

double CharacterBase::maxHp() const
{
    return m_baseHp * (1.0 + artifactHpPer()) + artifactHpNum();
}

double CharacterBase::energyRcVal() const
{
    return baseEnergyRcVal() * (1.0 + artifactEnergyRc());
}

double CharacterBase::eleMastery() const
{
    return m_baseEleMastery + artifactEleMastery();
}

double CharacterBase::critRate() const
{
    return m_baseCritRate + artifactCritRate();
}

double CharacterBase::critDmg() const
{
    return m_baseCritDmg + artifactCritDmg();
}

double CharacterBase::dmgBonus() const
{
    return baseDmgBonus() + artifactDmgBonus();
}

double CharacterBase::artifactHpNum() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].hpNum(); return s;
}
double CharacterBase::artifactAtkNum() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].atkNum(); return s;
}
double CharacterBase::artifactAtkPer() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].atkPer(); return s;
}
double CharacterBase::artifactHpPer() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].hpPer(); return s;
}
double CharacterBase::artifactEnergyRc() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].energyRc(); return s;
}
double CharacterBase::artifactCritDmg() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].critDmg(); return s;
}
double CharacterBase::artifactCritRate() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].critRate(); return s;
}
double CharacterBase::artifactEleMastery() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].eleMastery(); return s;
}
double CharacterBase::artifactDmgBonus() const {
    double s = 0; for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) if (m_hasArtifactSlot[i]) s += m_artifacts[i].dmgBonus(); return s;
}

double CharacterBase::normalAttackMultiplier() const
{
    return normalAttackInfo().multiplier + constellationNormalAtkBonus();
}

double CharacterBase::burstMultiplier() const
{
    return burstInfo().multiplier + constellationBurstBonus();
}

double CharacterBase::calcDamage(double skillMultiplier, bool crit, double dmgBonusOverride) const
{
    double bonus = (dmgBonusOverride >= 0) ? dmgBonusOverride : dmgBonus();
    double base = atk() * (1.0 + bonus) * skillMultiplier;
    if (crit)
        base *= (1.0 + critDmg());
    return base;
}

double CharacterBase::calcAmplifyingDamage(double skillMultiplier, double reactionMult, double masteryZone, bool crit) const
{
    double raw = atk() * (reactionMult + masteryZone) * skillMultiplier;
    if (crit)
        raw *= (1.0 + critDmg());
    raw *= (1.0 + dmgBonus());
    return raw;
}

double CharacterBase::absorbDamage(double dmg)
{
    if (m_shieldStrength <= 0) return dmg;
    double absorbed = dmg * m_shieldStrength;
    m_shieldStrength = qMax(0.0, m_shieldStrength - 0.4); // shield degrades
    return dmg - absorbed;
}

void CharacterBase::applyAura(ElementType elem, int units)
{
    if (elem == ElementType::None || elem == ElementType::Anemo) return;
    // Remove oldest if at capacity
    while (m_auras.size() >= MAX_AURA_UNITS)
        m_auras.removeFirst();
    m_auras.append({elem, AURA_DURATION, units});
}

void CharacterBase::tickAuras()
{
    for (int i = m_auras.size() - 1; i >= 0; --i) {
        m_auras[i].remainingTurns--;
        if (m_auras[i].remainingTurns <= 0)
            m_auras.removeAt(i);
    }
}

bool CharacterBase::hasAura(ElementType elem) const
{
    for (auto &a : m_auras)
        if (a.element == elem) return true;
    return false;
}
