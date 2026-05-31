#ifndef TESTCHARACTER_H
#define TESTCHARACTER_H

#include "CharacterBase.h"

class TestCharacter : public CharacterBase {
public:
    TestCharacter(const QString &displayName, ElementType element, WeaponType weaponType);

    SkillInfo normalAttackInfo() const override;
    SkillInfo burstInfo() const override;

    double constellationNormalAtkBonus() const override;
    double constellationBurstBonus() const override;
    double constellationBaseAtkMultiplier() const override;
    double constellationBaseEnergyRcValMultiplier() const override;
    double constellationBaseDmgBonus() const override;

    std::unique_ptr<CharacterBase> clone() const override;
};

// Factory to create all test characters
std::vector<std::unique_ptr<CharacterBase>> createAllCharacters();

#endif // TESTCHARACTER_H
