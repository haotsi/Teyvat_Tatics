#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include "GameTypes.h"
#include <QVector>

class Board;
class CharacterBase;

class AIController {
public:
    AIController();

    enum Difficulty { Easy, Normal, Hard };
    void setDifficulty(Difficulty d) { m_difficulty = d; }
    Difficulty difficulty() const { return m_difficulty; }

    // Deploy AI team on board
    void deployTeam(Board *board, QVector<CharacterBase*> &team);

    // Choose move for a piece during battle
    GridPos chooseMove(Board *board, CharacterBase *piece);

    // Choose attack target for a piece
    CharacterBase* chooseTarget(Board *board, CharacterBase *piece);

    // Score a target for prioritization (higher = more threatening)
    double scoreTarget(CharacterBase *attacker, CharacterBase *target) const;

private:
    Difficulty m_difficulty = Difficulty::Normal;

    bool shouldAvoidReaction(CharacterBase *attacker, CharacterBase *target,
                             ElementType attackElement) const;
    GridPos findBestPosition(Board *board, CharacterBase *piece) const;
};

#endif // AICONTROLLER_H
