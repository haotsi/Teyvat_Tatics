#include "AIController.h"
#include "Board.h"
#include "CharacterBase.h"
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

AIController::AIController() = default;

void AIController::deployTeam(Board *board, QVector<CharacterBase*> &team)
{
    if (!board || team.isEmpty()) return;

    // Deploy in enemy zone (rows 0-3)
    int deployRow = 0;
    int deployCol = 0;
    for (int i = 0; i < team.size() && i < MAX_POPULATION; ++i) {
        deployRow = ENEMY_START_ROW + (i / 2);
        deployCol = (i % 2 == 0) ? 2 : 5; // stagger positions
        if (!board->isOccupied(deployRow, deployCol)) {
            team[i]->setSide(TeamSide::Enemy);
            team[i]->setDeployIndex(i);
            board->placePiece(deployRow, deployCol, team[i]);
        }
    }
}

GridPos AIController::chooseMove(Board *board, CharacterBase *piece)
{
    if (!board || !piece) return GridPos{};

    // If piece can already attack a target, stay in place
    auto currentTargets = board->getValidAttackTargets(piece);
    if (!currentTargets.isEmpty())
        return piece->gridPos();

    auto validMoves = board->getValidMoves(piece);
    if (validMoves.isEmpty()) return piece->gridPos();

    GridPos best = findBestPosition(board, piece);
    if (best != piece->gridPos()) return best;

    // No position gives attack targets: move toward nearest enemy
    auto enemies = board->enemyPieces();
    if (!enemies.isEmpty()) {
        GridPos current = piece->gridPos();
        // Find nearest enemy
        int minDist = 999;
        for (auto *enemy : enemies) {
            int d = current.manhattanDist(enemy->gridPos());
            if (d < minDist) minDist = d;
        }
        // Find move that reduces distance to nearest enemy
        GridPos bestMove = current;
        int bestDist = minDist;
        for (auto &move : validMoves) {
            for (auto *enemy : enemies) {
                int d = move.manhattanDist(enemy->gridPos());
                if (d < bestDist) {
                    bestDist = d;
                    bestMove = move;
                }
            }
        }
        if (bestMove != current) return bestMove;
    }

    // Stay in place rather than moving randomly
    return piece->gridPos();
}

CharacterBase* AIController::chooseTarget(Board *board, CharacterBase *piece)
{
    if (!board || !piece) return nullptr;

    auto targets = board->getValidAttackTargets(piece);
    if (targets.isEmpty()) return nullptr;

    // Score all targets and pick the best
    CharacterBase *bestTarget = nullptr;
    double bestScore = -1;

    for (auto &pos : targets) {
        CharacterBase *target = board->pieceAt(pos);
        if (!target || !target->isAlive()) continue;

        double score = scoreTarget(piece, target);
        if (score > bestScore) {
            bestScore = score;
            bestTarget = target;
        }
    }

    return bestTarget;
}

double AIController::scoreTarget(CharacterBase *attacker, CharacterBase *target) const
{
    if (!attacker || !target) return 0;

    double score = 0;

    // Prioritize low HP targets (execution bonus)
    double hpRatio = target->currentHp() / target->maxHp();
    score += (1.0 - hpRatio) * 50;

    // Execution bonus: low HP targets are high priority
    if (hpRatio < 0.3)
        score += 30;

    // Prioritize high ATK targets (threat assessment)
    score += target->atk() / 100.0;

    // Avoid attacking targets with shield
    if (target->shieldStrength() > 0)
        score *= 0.5;

    // Avoid triggering bad reactions for us (reduced penalty)
    if (shouldAvoidReaction(attacker, target, attacker->element()))
        score *= 0.8;

    // Prefer targets that give favorable element reactions
    if (target->hasAura(ElementType::Hydro) && attacker->element() == ElementType::Pyro)
        score *= 1.5; // Vaporize
    if (target->hasAura(ElementType::Pyro) && attacker->element() == ElementType::Hydro)
        score *= 1.5; // Vaporize
    if (target->hasAura(ElementType::Cryo) && attacker->element() == ElementType::Pyro)
        score *= 1.5; // Melt

    // Frozen targets are easy prey
    if (target->isFrozen())
        score *= 1.3;

    // Energy full - about to burst, higher threat
    if (target->energyFull())
        score += 20;

    // Difficulty modifier
    double diffMod = 1.0;
    switch (m_difficulty) {
        case Easy:   diffMod = 0.7; break;
        case Normal: diffMod = 1.0; break;
        case Hard:   diffMod = 1.3; break;
    }
    score *= diffMod;

    // Add randomness
    auto *rng = QRandomGenerator::global();
    score += rng->bounded(10);

    return score;
}

bool AIController::shouldAvoidReaction(CharacterBase *attacker, CharacterBase *target,
                                        ElementType attackElement) const
{
    if (m_difficulty == Difficulty::Easy) return false;
    // Avoid triggering overload if we're adjacent (knockback wastes our turn)
    if (attackElement == ElementType::Pyro && target->hasAura(ElementType::Electro))
        return true;
    if (attackElement == ElementType::Electro && target->hasAura(ElementType::Pyro))
        return true;
    return false;
}

GridPos AIController::findBestPosition(Board *board, CharacterBase *piece) const
{
    GridPos current = piece->gridPos();
    auto validMoves = board->getValidMoves(piece);

    if (validMoves.isEmpty()) return current;

    // Check if current position already has targets
    int currentTargets = board->getValidAttackTargets(piece).size();
    if (currentTargets >= 1)
        return current;

    GridPos best = current;
    int bestTargetsReachable = currentTargets;

    // Block board signals during temporary evaluation moves
    board->setSignalBlocked(true);

    GridPos original = current;
    for (auto &move : validMoves) {
        board->removePiece(original);
        board->placePiece(move, piece);
        piece->setGridPos(move);

        int targets = board->getValidAttackTargets(piece).size();
        if (targets > bestTargetsReachable) {
            bestTargetsReachable = targets;
            best = move;
        }

        board->removePiece(move);
        board->placePiece(original, piece);
        piece->setGridPos(original);
    }

    board->setSignalBlocked(false);

    return best;
}
