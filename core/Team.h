#ifndef TEAM_H
#define TEAM_H

#include "GameTypes.h"
#include <QVector>
#include <QString>

class CharacterBase;

class Team {
public:
    Team(TeamSide side);
    ~Team();

    TeamSide side() const { return m_side; }
    QVector<CharacterBase*> &pieces() { return m_pieces; }
    const QVector<CharacterBase*> &pieces() const { return m_pieces; }

    void addPiece(CharacterBase *piece);
    void removePiece(CharacterBase *piece);
    void clear();

    int pieceCount() const { return m_pieces.size(); }

    // Team bonuses from elemental resonance
    struct TeamBonuses {
        bool voltage = false;   // 2+ Electro: energy_rc_val +5
        bool flames = false;    // 2+ Pyro: atk +25%
        bool water = false;     // 2+ Hydro: hp +25%
        bool greenery = false;  // 2+ Dendro: ele_mastery +100
        bool ice = false;       // 2+ Cryo: crit_rate +15%
    };

    TeamBonuses calculateBonuses() const;
    void applyBonuses();
    QString activeBonusesText() const;

    int countElement(ElementType e) const;

private:
    TeamSide m_side;
    QVector<CharacterBase*> m_pieces;
};

#endif // TEAM_H
