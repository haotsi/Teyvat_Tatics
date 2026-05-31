#include "Artifact.h"

Artifact::Artifact(ArtifactSlot slot, ArtifactMainStat stat)
    : m_slot(slot), m_mainStat(stat)
{
}

QString Artifact::name() const
{
    if (m_slot == ArtifactSlot::NONE) return QStringLiteral("空");
    return artifactSlotName(m_slot) + QStringLiteral(" - ") + statName(m_mainStat);
}

double Artifact::hpNum() const
{
    if (m_slot == ArtifactSlot::Flower) return ArtifactStatBase::flowerHP();
    return 0;
}

double Artifact::atkNum() const
{
    if (m_slot == ArtifactSlot::Plume) return ArtifactStatBase::plumeATK();
    return 0;
}

double Artifact::atkPer() const
{
    if (m_mainStat != ArtifactMainStat::ATK_PER) return 0;
    switch (m_slot) {
        case ArtifactSlot::Sands:   return ArtifactStatBase::sandsATKPer();
        case ArtifactSlot::Goblet:  return ArtifactStatBase::gobletATKPer();
        case ArtifactSlot::Circlet: return ArtifactStatBase::circletATKPer();
        default: return 0;
    }
}

double Artifact::hpPer() const
{
    if (m_mainStat != ArtifactMainStat::HP_PER) return 0;
    switch (m_slot) {
        case ArtifactSlot::Sands:   return ArtifactStatBase::sandsHPPer();
        case ArtifactSlot::Goblet:  return ArtifactStatBase::gobletHPPer();
        case ArtifactSlot::Circlet: return ArtifactStatBase::circletHPPer();
        default: return 0;
    }
}

double Artifact::energyRc() const
{
    if (m_mainStat == ArtifactMainStat::ENERGY_RC && m_slot == ArtifactSlot::Sands)
        return ArtifactStatBase::sandsER();
    return 0;
}

double Artifact::critDmg() const
{
    if (m_mainStat == ArtifactMainStat::CRIT_DMG && m_slot == ArtifactSlot::Circlet)
        return ArtifactStatBase::circletCritDmg();
    return 0;
}

double Artifact::critRate() const
{
    if (m_mainStat == ArtifactMainStat::CRIT_RATE && m_slot == ArtifactSlot::Circlet)
        return ArtifactStatBase::circletCritRate();
    return 0;
}

double Artifact::eleMastery() const
{
    if (m_mainStat != ArtifactMainStat::ELE_MASTERY) return 0;
    switch (m_slot) {
        case ArtifactSlot::Sands:   return ArtifactStatBase::sandsEM();
        case ArtifactSlot::Goblet:  return ArtifactStatBase::gobletEM();
        case ArtifactSlot::Circlet: return ArtifactStatBase::circletEM();
        default: return 0;
    }
}

double Artifact::dmgBonus() const
{
    if (m_mainStat == ArtifactMainStat::DMG_BONUS && m_slot == ArtifactSlot::Goblet)
        return ArtifactStatBase::gobletDmgBonus();
    return 0;
}
