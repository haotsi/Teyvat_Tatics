#include "KeQing.h"

KeQing::KeQing()
    : CharacterBase(QStringLiteral("刻晴"), ElementType::Electro, WeaponType::Sword)
{
    m_baseAtk = 322;
    m_baseHp = 18000;
    m_baseEleMastery = 0;
    m_baseCritRate = 0.22;
    m_baseCritDmg = 0.884;
    m_baseDmgBonus = 0;
    m_baseEnergyRcVal = 8;
    m_energyMax = 40;
    m_currentHp = maxHp();
}

CharacterBase::SkillInfo KeQing::normalAttackInfo() const
{
    return {QStringLiteral("普通攻击"), 1.0, ElementType::Electro, 8};
}

CharacterBase::SkillInfo KeQing::burstInfo() const
{
    return {QStringLiteral("大招"), 2.0, ElementType::Electro, 0};
}

double KeQing::constellationNormalAtkBonus() const
{
    return (m_constellation >= 5) ? 0.20 : 0;
}

double KeQing::constellationBurstBonus() const
{
    double bonus = 0;
    if (m_constellation >= 1) bonus += 0.20;
    if (m_constellation >= 3) bonus += 0.30;
    return bonus;
}

double KeQing::constellationBaseAtkMultiplier() const
{
    return (m_constellation >= 4) ? 1.5 : 1.0;
}

double KeQing::constellationBaseEnergyRcValMultiplier() const
{
    return (m_constellation >= 2) ? 1.25 : 1.0;
}

double KeQing::constellationBaseDmgBonus() const
{
    return (m_constellation >= 6) ? 0.24 : 0;
}

std::unique_ptr<CharacterBase> KeQing::clone() const
{
    auto c = std::make_unique<KeQing>(*this);
    return c;
}
