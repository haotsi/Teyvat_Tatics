#include "TestCharacter.h"
#include "KeQing.h"

TestCharacter::TestCharacter(const QString &displayName, ElementType element, WeaponType weaponType)
    : CharacterBase(displayName, element, weaponType)
{
    // Same base stats as KeQing per design doc
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

CharacterBase::SkillInfo TestCharacter::normalAttackInfo() const
{
    return {QStringLiteral("普通攻击"), 1.0, m_element, 8};
}

CharacterBase::SkillInfo TestCharacter::burstInfo() const
{
    return {QStringLiteral("大招"), 2.0, m_element, 0};
}

double TestCharacter::constellationNormalAtkBonus() const
{
    return (m_constellation >= 5) ? 0.20 : 0;
}

double TestCharacter::constellationBurstBonus() const
{
    double bonus = 0;
    if (m_constellation >= 1) bonus += 0.20;
    if (m_constellation >= 3) bonus += 0.30;
    return bonus;
}

double TestCharacter::constellationBaseAtkMultiplier() const
{
    return (m_constellation >= 4) ? 1.5 : 1.0;
}

double TestCharacter::constellationBaseEnergyRcValMultiplier() const
{
    return (m_constellation >= 2) ? 1.25 : 1.0;
}

double TestCharacter::constellationBaseDmgBonus() const
{
    return (m_constellation >= 6) ? 0.24 : 0;
}

std::unique_ptr<CharacterBase> TestCharacter::clone() const
{
    return std::make_unique<TestCharacter>(*this);
}

std::vector<std::unique_ptr<CharacterBase>> createAllCharacters()
{
    std::vector<std::unique_ptr<CharacterBase>> chars;
    chars.push_back(std::make_unique<KeQing>());
    chars.push_back(std::make_unique<TestCharacter>(QStringLiteral("test_1"), ElementType::Anemo, WeaponType::Sword));
    chars.push_back(std::make_unique<TestCharacter>(QStringLiteral("test_2"), ElementType::Pyro, WeaponType::Claymore));
    chars.push_back(std::make_unique<TestCharacter>(QStringLiteral("test_3"), ElementType::Cryo, WeaponType::Catalyst));
    chars.push_back(std::make_unique<TestCharacter>(QStringLiteral("test_4"), ElementType::Dendro, WeaponType::Bow));
    chars.push_back(std::make_unique<TestCharacter>(QStringLiteral("test_5"), ElementType::Geo, WeaponType::Polearm));
    chars.push_back(std::make_unique<TestCharacter>(QStringLiteral("test_6"), ElementType::Hydro, WeaponType::Sword));
    return chars;
}
