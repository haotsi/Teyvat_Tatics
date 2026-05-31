#ifndef KEQING_H
#define KEQING_H

#include "CharacterBase.h"

class KeQing : public CharacterBase {
public:
    KeQing();

    SkillInfo normalAttackInfo() const override;
    SkillInfo burstInfo() const override;

    double constellationNormalAtkBonus() const override;
    double constellationBurstBonus() const override;
    double constellationBaseAtkMultiplier() const override;
    double constellationBaseEnergyRcValMultiplier() const override;
    double constellationBaseDmgBonus() const override;

    std::unique_ptr<CharacterBase> clone() const override;
};

#endif // KEQING_H
