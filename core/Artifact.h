#ifndef ARTIFACT_H
#define ARTIFACT_H

#include "GameTypes.h"

class Artifact {
public:
    Artifact() = default;
    Artifact(ArtifactSlot slot, ArtifactMainStat stat);

    ArtifactSlot slot() const { return m_slot; }
    ArtifactMainStat mainStat() const { return m_mainStat; }

    double hpNum() const;
    double atkNum() const;
    double atkPer() const;
    double hpPer() const;
    double energyRc() const;
    double critDmg() const;
    double critRate() const;
    double eleMastery() const;
    double dmgBonus() const;

    QString name() const;
    int cost() const { return ARTIFACT_COST; }
    int sellPrice() const { return qMax(1, static_cast<int>(ARTIFACT_COST * SELL_RATIO)); }

private:
    ArtifactSlot m_slot = ArtifactSlot::NONE;
    ArtifactMainStat m_mainStat = ArtifactMainStat::NONE;
};

#endif // ARTIFACT_H
