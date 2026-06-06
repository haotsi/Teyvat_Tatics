#include "GameEngine.h"
#include "Board.h"
#include "Team.h"
#include "Shop.h"
#include "AIController.h"
#include "ElementSystem.h"
#include "CharacterBase.h"
#include "TestCharacter.h"
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

GameEngine::GameEngine(QObject *parent) : QObject(parent)
{
    m_board = new Board(this);
    m_playerTeam = new Team(TeamSide::Player);
    m_enemyTeam = new Team(TeamSide::Enemy);
    m_shop = new Shop(this);
    m_ai = new AIController;
    m_elementSystem = new ElementSystem;

    connect(m_board, &Board::boardChanged, this, &GameEngine::boardChanged);
    connect(m_shop, &Shop::shopChanged, this, &GameEngine::shopChanged);

    // Setup element system callbacks
    m_elementSystem->setDamageCallback([this](CharacterBase *target, double dmg, ReactionType rt) {
        Q_UNUSED(rt);
        if (target) target->takeDamage(dmg);
    });
}

GameEngine::~GameEngine()
{
    delete m_ai;
    delete m_elementSystem;
}

void GameEngine::startNewGame()
{
    m_primogems = INITIAL_PRIMOGEMS;
    m_mora = INITIAL_MORA;
    m_currentRound = 0;
    m_playerWins = 0;
    m_enemyWins = 0;
    m_populationLevel = INITIAL_POPULATION;

    m_storage.clear();
    m_weaponBackpack.clear();
    m_artifactBackpack.clear();
    m_battleLog.clear();
    m_playerTeam->clear();
    m_enemyTeam->clear();
    m_board->clear();

    m_characterPool = createAllCharacters();

    emit resourcesChanged();
    emit scoreChanged();
    emit populationChanged();

    startPreparationPhase();
}

void GameEngine::startPreparationPhase()
{
    m_phase = GamePhase::Preparation;
    m_board->clearSide(TeamSide::Enemy);
    m_enemyTeam->clear();
    m_enemyCharacters.clear();

    // Restore player pieces to full HP and original positions
    if (!m_savedPlayerPositions.isEmpty()) {
        // Remove all player pieces from board
        QVector<QPair<CharacterBase*, GridPos>> restoreList;
        for (auto it = m_savedPlayerPositions.begin(); it != m_savedPlayerPositions.end(); ++it) {
            CharacterBase *p = it.key();
            if (p && p->gridPos().isValid())
                m_board->removePiece(p->gridPos());
            restoreList.append(QPair<CharacterBase*, GridPos>(p, it.value()));
        }
        // Place them back at saved positions with full HP
        for (auto &pair : restoreList) {
            if (pair.first) {
                pair.first->setCurrentHp(pair.first->maxHp());
                if (!m_board->isOccupied(pair.second))
                    m_board->placePiece(pair.second, pair.first);
            }
        }
        m_savedPlayerPositions.clear();
    }

    m_currentRound++;
    if (m_currentRound > TOTAL_ROUNDS) {
        endGame();
        return;
    }

    calculateInterest();
    m_shop->resetRefreshCost();
    refreshShop();
    m_battlePhaseActive = false;
    m_battleLog.clear();

    emit phaseChanged(m_phase);
    emit roundChanged();
    emit messageLogged(QStringLiteral("第%1轮准备阶段开始").arg(m_currentRound));
}

void GameEngine::startBattlePhase()
{
    if (m_phase != GamePhase::Preparation) return;

    m_phase = GamePhase::Battle;
    m_battlePhaseActive = true;
    m_battleLog.clear();

    // Validate: player must have at least one piece on board
    auto playerPieces = m_board->playerPieces();
    if (playerPieces.isEmpty()) {
        emit messageLogged(QStringLiteral("场上没有棋子，自动判负！"));
        onBattleFinished(false);
        return;
    }

    // Generate enemy team
    generateEnemyTeam();

    // Setup battle state
    setupBattle();

    emit phaseChanged(m_phase);
    emit messageLogged(QStringLiteral("第%1轮战斗开始！").arg(m_currentRound));
}

void GameEngine::generateEnemyTeam()
{
    m_board->clearSide(TeamSide::Enemy);
    m_enemyTeam->clear();
    m_enemyCharacters.clear();

    auto *rng = QRandomGenerator::global();
    int enemyCount = qMin(m_populationLevel, 2 + rng->bounded(2));

    m_enemyBattleOrder.clear();
    for (int i = 0; i < enemyCount; ++i) {
        int idx = rng->bounded(m_characterPool.size());
        auto enemy = m_characterPool[idx]->clone();
        enemy->setSide(TeamSide::Enemy);
        enemy->setId(i);
        enemy->setDeployIndex(i);
        enemy->setWeapon(Weapon(enemy->weaponType(), 2 + rng->bounded(3)));

        auto *raw = enemy.get();
        m_enemyTeam->addPiece(raw);
        m_enemyBattleOrder.append(raw);

        int row = ENEMY_START_ROW + (i / 2);
        int col = (i % 2 == 0) ? 2 : 5;
        m_board->placePiece(row, col, raw);

        m_enemyCharacters.push_back(std::move(enemy));
    }
}

void GameEngine::setupBattle()
{
    // Build battle order from board
    buildBattleOrder();

    // Save player positions for post-battle restore
    m_savedPlayerPositions.clear();
    for (auto *p : m_playerBattleOrder) {
        m_savedPlayerPositions.insert(p, p->gridPos());
    }

    // Reset all pieces
    for (auto *p : m_playerBattleOrder) {
        p->setCurrentHp(p->maxHp());
        p->setCurrentEnergy(0);
        p->auras().clear();
        p->setQuicken(0);
        p->setBurning(0);
        p->setFrozen(0);
        p->clearShield();
    }
    for (auto *p : m_enemyBattleOrder) {
        p->setCurrentHp(p->maxHp());
        p->setCurrentEnergy(0);
        p->auras().clear();
        p->setQuicken(0);
        p->setBurning(0);
        p->setFrozen(0);
        p->clearShield();
    }

    m_currentActorIndex = 0;
    m_playerTurn = true;
}

void GameEngine::buildBattleOrder()
{
    m_playerBattleOrder.clear();
    m_enemyBattleOrder.clear();

    auto playerPieces = m_board->playerPieces();
    auto enemyPieces = m_board->enemyPieces();

    // Sort by deploy index
    std::sort(playerPieces.begin(), playerPieces.end(), [](CharacterBase *a, CharacterBase *b) {
        return a->deployIndex() < b->deployIndex();
    });
    std::sort(enemyPieces.begin(), enemyPieces.end(), [](CharacterBase *a, CharacterBase *b) {
        return a->deployIndex() < b->deployIndex();
    });

    m_playerBattleOrder = playerPieces;
    m_enemyBattleOrder = enemyPieces;

    // Set IDs
    for (int i = 0; i < m_playerBattleOrder.size(); ++i)
        m_playerBattleOrder[i]->setId(i);
    for (int i = 0; i < m_enemyBattleOrder.size(); ++i)
        m_enemyBattleOrder[i]->setId(i);
}

void GameEngine::executeBattleStep()
{
    if (!m_battlePhaseActive || isBattleOver()) return;

    auto &currentOrder = m_playerTurn ? m_playerBattleOrder : m_enemyBattleOrder;

    // Skip dead/frozen pieces
    while (m_currentActorIndex < currentOrder.size()) {
        auto *piece = currentOrder[m_currentActorIndex];
        if (piece->isAlive() && !piece->isFrozen())
            break;
        if (piece->isFrozen())
            piece->tickFrozen();
        m_currentActorIndex++;
    }

    if (m_currentActorIndex >= currentOrder.size()) {
        // Side's turn is done, tick auras and switch sides
        tickAllAuras(m_playerTurn ? TeamSide::Player : TeamSide::Enemy);
        m_currentActorIndex = 0;
        m_playerTurn = !m_playerTurn;

        if (isBattleOver()) {
            m_battlePhaseActive = false;
            bool playerWin = !m_board->enemyPieces().isEmpty() ? false :
                             !m_board->playerPieces().isEmpty() ? true : false;
            // Check: if all enemies dead, player wins; if all players dead, enemy wins
            if (m_board->enemyPieces().isEmpty())
                playerWin = true;
            else if (m_board->playerPieces().isEmpty())
                playerWin = false;

            onBattleFinished(playerWin);
            return;
        }
        return;
    }

    auto *piece = currentOrder[m_currentActorIndex];
    processCharacterAction(piece);
    m_currentActorIndex++;
}

void GameEngine::executeBattleRound()
{
    // Execute full round: player acts, then enemy acts
    // Player actions
    for (auto *piece : m_playerBattleOrder) {
        if (!piece->isAlive()) continue;
        if (piece->isFrozen()) {
            piece->tickFrozen();
            continue;
        }
        // AI controls movement/attack for demo, or we could make this player-controlled
        // For PvE, the enemy uses AI, player pieces can use AI or manual control
        // In test version, both sides use AI for battle phase
        processCharacterAction(piece);
        if (isBattleOver()) {
            onBattleFinished(true);
            return;
        }
    }
    tickAllAuras(TeamSide::Player);

    if (isBattleOver()) {
        onBattleFinished(true);
        return;
    }

    // Enemy actions
    for (auto *piece : m_enemyBattleOrder) {
        if (!piece->isAlive()) continue;
        if (piece->isFrozen()) {
            piece->tickFrozen();
            continue;
        }
        processCharacterAction(piece);
        if (isBattleOver()) {
            onBattleFinished(false);
            return;
        }
    }
    tickAllAuras(TeamSide::Enemy);
}

void GameEngine::processCharacterAction(CharacterBase *piece)
{
    if (!piece || !piece->isAlive()) return;

    BattleAction action;
    action.attackerId = piece->id();
    action.attackerSide = piece->side();

    // Tick burning damage
    if (piece->burningTurns() > 0) {
        double burnDmg = ElementSystem::burningDamage(piece->eleMastery());
        piece->takeDamage(burnDmg);
        piece->tickBurning();
        action.damage = burnDmg;
        action.reaction = ReactionType::Burning;
        action.skillName = QStringLiteral("燃烧");
        m_battleLog.append(action);
        emit battleActionOccurred(action);
    }

    // Tick quicken
    if (piece->inQuicken()) piece->tickQuicken();

    // Tick shield (handled in absorbDamage)

    // --- Movement ---
    GridPos oldPos = piece->gridPos();
    GridPos newPos = m_ai->chooseMove(m_board, piece);
    if (newPos != oldPos && newPos.isValid() && m_board->isEmpty(newPos)) {
        m_board->movePiece(oldPos, newPos);
        action.moved = true;
        action.fromPos = oldPos;
        action.toPos = newPos;
    } else {
        action.fromPos = oldPos;
        action.toPos = oldPos;
    }

    // --- Energy gain ---
    piece->addEnergy(static_cast<int>(piece->energyRcVal()));

    // --- Attack ---
    CharacterBase *target = m_ai->chooseTarget(m_board, piece);
    if (target && target->isAlive()) {
        bool useBurst = piece->energyFull();
        if (piece->weaponType() == WeaponType::Polearm) {
            // Polearm pierce: hit all enemies along attack direction
            GridPos atkPos = piece->gridPos();
            GridPos tgtPos = target->gridPos();
            int dr = 0, dc = 0;
            if (tgtPos.row > atkPos.row) dr = 1;
            else if (tgtPos.row < atkPos.row) dr = -1;
            if (tgtPos.col > atkPos.col) dc = 1;
            else if (tgtPos.col < atkPos.col) dc = -1;
            for (int dist = 1; dist <= 2; ++dist) {
                GridPos pos{atkPos.row + dr * dist, atkPos.col + dc * dist};
                if (!pos.isValid()) break;
                auto *enemy = m_board->pieceAt(pos);
                if (enemy && enemy->side() != piece->side() && enemy->isAlive())
                    processAttack(piece, enemy, useBurst);
            }
        } else {
            processAttack(piece, target, useBurst);
        }
        if (useBurst) {
            piece->setCurrentEnergy(0);
            action.skillName = piece->burstInfo().name;
        } else {
            action.skillName = piece->normalAttackInfo().name;
        }
    }

    m_battleLog.append(action);
    emit battleActionOccurred(action);
}

void GameEngine::processAttack(CharacterBase *attacker, CharacterBase *defender, bool isBurst)
{
    if (!attacker || !defender) return;

    ElementType atkElem;
    double skillMult;
    if (isBurst) {
        auto info = attacker->burstInfo();
        atkElem = info.elementAttachment;
        skillMult = attacker->burstMultiplier();
    } else {
        auto info = attacker->normalAttackInfo();
        atkElem = info.elementAttachment;
        skillMult = attacker->normalAttackMultiplier();
    }

    // Try elemental reaction first
    ReactionResult reaction = m_elementSystem->tryReaction(attacker, defender, atkElem, isBurst);

    double totalDmg = 0;
    bool crit = QRandomGenerator::global()->bounded(100) / 100.0 < attacker->critRate();

    if (reaction.type != ReactionType::None && reaction.isAmplifying) {
        // Amplifying reaction
        totalDmg = attacker->calcAmplifyingDamage(skillMult, reaction.reactionMultiplier,
                                                   reaction.masteryZone, crit);
    } else {
        // Normal damage
        totalDmg = attacker->calcDamage(skillMult, crit);
    }

    // Apply shield absorption
    totalDmg = defender->absorbDamage(totalDmg);

    // Apply reaction damage (transformative)
    if (reaction.type != ReactionType::None && !reaction.isAmplifying) {
        totalDmg += reaction.damage;
    }

    defender->takeDamage(totalDmg);

    // Handle reaction effects
    processReaction(attacker, defender, reaction);

    BattleAction logEntry;
    logEntry.attackerId = attacker->id();
    logEntry.targetId = defender->id();
    logEntry.damage = totalDmg;
    logEntry.reaction = reaction.type;
    logEntry.skillName = isBurst ? attacker->burstInfo().name : attacker->normalAttackInfo().name;
    logEntry.crit = crit;
    logEntry.attackElement = atkElem;
    logEntry.reactionElement = reactionElementColor(reaction.type);
    logEntry.attackerSide = attacker->side();
    m_battleLog.append(logEntry);
    emit battleActionOccurred(logEntry);
}

void GameEngine::processReaction(CharacterBase *attacker, CharacterBase *defender,
                                  const ReactionResult &reaction)
{
    if (!defender) return;

    switch (reaction.type) {
        case ReactionType::Frozen:
            defender->setFrozen(1);
            break;
        case ReactionType::Overload:
            if (reaction.knockback)
                applyKnockback(defender);
            break;
        case ReactionType::Quicken:
            if (reaction.entersQuicken)
                defender->setQuicken(3);
            break;
        case ReactionType::Spread:
        case ReactionType::Aggravate:
            if (reaction.exitsQuicken)
                defender->setQuicken(0);
            break;
        case ReactionType::Burning:
            if (reaction.appliesBurning)
                defender->setBurning(2);
            break;
        case ReactionType::Crystallize:
            if (reaction.createsShield)
                attacker->addShield(0.4);
            break;
        case ReactionType::Bloom:
            if (reaction.createsSeed)
                effectPlay(QStringLiteral("seed"), defender->gridPos());
            break;
        default: break;
    }

    // Splash damage
    if (reaction.splashDamage > 0) {
        QVector<GridPos> adj;
        if (reaction.splashDamage == 1)
            adj = m_board->adjacentPositions4(defender->gridPos());
        else
            adj = m_board->adjacentPositions8(defender->gridPos());

        for (auto &pos : adj) {
            auto *neighbor = m_board->pieceAt(pos);
            if (neighbor && neighbor->side() != attacker->side()) {
                double splashDmg = ElementSystem::transformativeDamage(attacker->eleMastery(), reaction.splashMultiplier);
                neighbor->takeDamage(splashDmg);
            }
        }

        // If splash includes self/ally damage (bloom)
        if (reaction.splashDamage == 2) {
            for (auto &pos : adj) {
                auto *ally = m_board->pieceAt(pos);
                if (ally && ally->side() == attacker->side()) {
                    double selfDmg = ElementSystem::transformativeDamage(attacker->eleMastery(), BLOOM_SELF_MULT);
                    ally->takeDamage(selfDmg);
                }
            }
        }
    }

    // Swirl spreads element
    if (reaction.spreadElement != ElementType::None) {
        auto adj = m_board->adjacentPositions4(defender->gridPos());
        for (auto &pos : adj) {
            auto *neighbor = m_board->pieceAt(pos);
            if (neighbor)
                neighbor->applyAura(reaction.spreadElement, 1);
        }
    }
}

void GameEngine::applyKnockback(CharacterBase *target)
{
    if (!target) return;
    GridPos pos = target->gridPos();
    // Find direction away from center of attacker side
    int pushDir = (target->side() == TeamSide::Player) ? -1 : 1; // push away
    GridPos newPos{pos.row + pushDir, pos.col};

    if (newPos.isValid() && m_board->isEmpty(newPos)) {
        m_board->movePiece(pos, newPos);
    }
}

void GameEngine::tickAllAuras(TeamSide side)
{
    auto pieces = (side == TeamSide::Player) ? m_board->playerPieces() : m_board->enemyPieces();
    for (auto *p : pieces) {
        p->tickAuras();
        if (p->inQuicken()) p->tickQuicken();
        if (p->burningTurns() > 0) {
            double dmg = ElementSystem::burningDamage(p->eleMastery());
            p->takeDamage(dmg);
            p->tickBurning();
        }
    }
}

bool GameEngine::isBattleOver() const
{
    if (!m_battlePhaseActive) return true;

    bool playerAlive = false, enemyAlive = false;
    for (auto *p : m_board->playerPieces()) {
        if (p->isAlive()) { playerAlive = true; break; }
    }
    for (auto *p : m_board->enemyPieces()) {
        if (p->isAlive()) { enemyAlive = true; break; }
    }
    return !playerAlive || !enemyAlive;
}

void GameEngine::onBattleFinished(bool playerWin)
{
    m_battlePhaseActive = false;

    if (playerWin) {
        m_playerWins++;
        m_primogems += WIN_PRIMOGEMS;
        m_mora += WIN_MORA;
    } else {
        m_enemyWins++;
        m_primogems += LOSE_PRIMOGEMS;
        m_mora += LOSE_MORA;
    }

    emit battleFinished(playerWin);
    emit resourcesChanged();
    emit scoreChanged();

    if (m_playerWins >= WINS_NEEDED) {
        emit messageLogged(QStringLiteral("恭喜！你已赢得%1局，获得最终胜利！").arg(m_playerWins));
        endGame();
    } else if (m_enemyWins >= WINS_NEEDED) {
        emit messageLogged(QStringLiteral("很遗憾，对手已赢得%1局，你输了。").arg(m_enemyWins));
        endGame();
    } else if (m_currentRound >= TOTAL_ROUNDS) {
        if (m_playerWins > m_enemyWins) {
            emit messageLogged(QStringLiteral("11轮结束，你以%1:%2获胜！").arg(m_playerWins).arg(m_enemyWins));
        } else if (m_enemyWins > m_playerWins) {
            emit messageLogged(QStringLiteral("11轮结束，你以%1:%2落败。").arg(m_playerWins).arg(m_enemyWins));
        } else {
            emit messageLogged(QStringLiteral("11轮结束，平局！"));
        }
        endGame();
    } else {
        startPreparationPhase();
    }
}

void GameEngine::endGame()
{
    m_phase = GamePhase::GameOver;
    m_battlePhaseActive = false;
    bool playerWin = m_playerWins > m_enemyWins;
    emit phaseChanged(m_phase);
    emit gameOver(playerWin);
}

void GameEngine::calculateInterest()
{
    int primoInterest = static_cast<int>(m_primogems * INTEREST_RATE);
    int moraInterest = static_cast<int>(m_mora * INTEREST_RATE);
    m_primogems += primoInterest;
    m_mora += moraInterest;
    if (primoInterest > 0 || moraInterest > 0) {
        emit messageLogged(QStringLiteral("利息: +%1原石 +%2摩拉").arg(primoInterest).arg(moraInterest));
    }
    emit resourcesChanged();
}

void GameEngine::refreshShop()
{
    m_shop->refresh(m_characterPool);
}

bool GameEngine::manualRefreshShop()
{
    int cost = m_shop->refreshCost();
    if (m_mora < cost) {
        emit messageLogged(QStringLiteral("摩拉不足，需要%1摩拉！").arg(cost));
        return false;
    }
    m_mora -= cost;
    m_shop->refresh(m_characterPool);
    emit resourcesChanged();
    return true;
}

bool GameEngine::buyShopItem(int index)
{
    auto *item = m_shop->itemAt(index);
    if (!item) return false;

    // Check affordability first
    int costPrimo = 0, costMora = 0;
    ShopItem::Type itemType = item->type;

    // Copy data before purchase (which removes the item)
    std::unique_ptr<CharacterBase> purchasedChar;
    Weapon purchasedWeapon;
    Artifact purchasedArtifact;

    switch (itemType) {
        case ShopItem::Item_Char:
            costPrimo = CHARACTER_COST;
            if (item->character)
                purchasedChar = item->character->clone();
            break;
        case ShopItem::Item_Weapon:
            costMora = item->weapon.cost();
            purchasedWeapon = item->weapon;
            break;
        case ShopItem::Item_Artifact:
            costMora = ARTIFACT_COST;
            purchasedArtifact = item->artifact;
            break;
    }

    if (m_primogems < costPrimo || m_mora < costMora) {
        emit messageLogged(QStringLiteral("货币不足！"));
        return false;
    }

    m_primogems -= costPrimo;
    m_mora -= costMora;
    m_shop->removeItem(index);

    // Add to player's inventory
    switch (itemType) {
        case ShopItem::Item_Char:
            if (purchasedChar) {
                if (!addToStorage(purchasedChar.release())) {
                    m_primogems += costPrimo; // refund if storage full
                    emit messageLogged(QStringLiteral("储存栏已满，无法购买角色！"));
                }
            }
            break;
        case ShopItem::Item_Weapon:
            if (m_weaponBackpack.size() >= MAX_WEAPON_BACKPACK) {
                m_mora += costMora; // refund
                emit messageLogged(QStringLiteral("武器背包已满！"));
                return false;
            }
            m_weaponBackpack.append(purchasedWeapon);
            emit backpackChanged();
            break;
        case ShopItem::Item_Artifact:
            if (m_artifactBackpack.size() >= MAX_ARTIFACT_BACKPACK) {
                m_mora += costMora; // refund
                emit messageLogged(QStringLiteral("圣遗物背包已满！"));
                return false;
            }
            m_artifactBackpack.append(purchasedArtifact);
            emit backpackChanged();
            break;
    }

    emit resourcesChanged();
    return true;
}

bool GameEngine::sellStorageItem(int index)
{
    if (index < 0 || index >= m_storage.size()) return false;
    auto *piece = m_storage[index];
    // Sell price: 90 base + 15 per constellation
    m_primogems += 90 + piece->constellation() * 15;
    // Also sell equipped items
    if (piece->hasWeapon()) {
        m_mora += piece->weapon().sellPrice();
    }
    for (int i = 0; i < 5; ++i) {
        auto slot = static_cast<ArtifactSlot>(i);
        if (piece->hasArtifact(slot))
            m_mora += piece->artifact(slot).sellPrice();
    }
    unregisterCharacter(piece);
    delete piece;
    m_storage.removeAt(index);
    emit resourcesChanged();
    emit storageChanged();
    return true;
}

bool GameEngine::sellWeaponFromBackpack(int index)
{
    if (index < 0 || index >= m_weaponBackpack.size()) return false;
    m_mora += m_weaponBackpack[index].sellPrice();
    m_weaponBackpack.removeAt(index);
    emit resourcesChanged();
    emit backpackChanged();
    return true;
}

bool GameEngine::sellArtifactFromBackpack(int index)
{
    if (index < 0 || index >= m_artifactBackpack.size()) return false;
    m_mora += m_artifactBackpack[index].sellPrice();
    m_artifactBackpack.removeAt(index);
    emit resourcesChanged();
    emit backpackChanged();
    return true;
}

bool GameEngine::addToStorage(CharacterBase *piece)
{
    if (m_storage.size() >= STORAGE_CAPACITY) return false;
    piece->setSide(TeamSide::Player);
    m_storage.append(piece);
    registerCharacter(piece);
    emit storageChanged();
    return true;
}

void GameEngine::removeFromStorage(CharacterBase *piece)
{
    m_storage.removeAll(piece);
    emit storageChanged();
}

CharacterBase* GameEngine::takeFromStorage(int index)
{
    if (index < 0 || index >= m_storage.size()) return nullptr;
    auto *p = m_storage.takeAt(index);
    emit storageChanged();
    return p;
}

bool GameEngine::deployFromStorage(int storageIndex, GridPos pos)
{
    if (storageIndex < 0 || storageIndex >= m_storage.size()) return false;
    if (!pos.isValid()) return false;
    if (!m_board->isPlayerDeployZone(pos)) {
        emit messageLogged(QStringLiteral("只能部署在我方区域（5-8行）"));
        return false;
    }
    if (m_board->isOccupied(pos)) return false;

    int currentOnBoard = m_board->playerPieces().size();
    if (currentOnBoard >= m_populationLevel) {
        emit messageLogged(QStringLiteral("人口不足，请升级人口或撤回棋子"));
        return false;
    }

    auto *piece = takeFromStorage(storageIndex);
    if (!piece) return false;

    piece->setDeployIndex(currentOnBoard);
    piece->setSide(TeamSide::Player);
    m_board->placePiece(pos, piece);
    m_playerTeam->addPiece(piece);
    emit boardChanged();
    return true;
}

bool GameEngine::returnToStorage(GridPos pos)
{
    if (!m_board->isPlayerDeployZone(pos)) return false;
    auto *piece = m_board->removePiece(pos);
    if (!piece) return false;

    m_playerTeam->removePiece(piece);
    return addToStorage(piece);
}

bool GameEngine::moveDeployedPiece(GridPos from, GridPos to)
{
    if (!m_board->isPlayerDeployZone(from)) return false;
    if (!m_board->isPlayerDeployZone(to)) return false;
    if (m_board->isOccupied(to)) return false;
    if (!m_board->isOccupied(from)) return false;

    m_board->movePiece(from, to);
    return true;
}

bool GameEngine::equipWeapon(CharacterBase *piece, int backpackIndex)
{
    if (!piece || backpackIndex < 0 || backpackIndex >= m_weaponBackpack.size()) return false;
    // Check weapon type matches character
    if (m_weaponBackpack[backpackIndex].type() != piece->weaponType()) return false;
    // Check capacity if swapping (old weapon goes to backpack)
    if (piece->hasWeapon() && m_weaponBackpack.size() >= MAX_WEAPON_BACKPACK) {
        emit messageLogged(QStringLiteral("武器背包已满，无法卸下旧武器！"));
        return false;
    }
    Weapon w = m_weaponBackpack.takeAt(backpackIndex);
    if (piece->hasWeapon())
        m_weaponBackpack.append(piece->weapon());
    piece->setWeapon(w);
    emit backpackChanged();
    return true;
}

bool GameEngine::equipArtifact(CharacterBase *piece, int backpackIndex, ArtifactSlot slot)
{
    if (!piece || backpackIndex < 0 || backpackIndex >= m_artifactBackpack.size()) return false;
    // Check artifact slot matches target
    if (m_artifactBackpack[backpackIndex].slot() != slot) return false;
    // Check capacity if swapping (old artifact goes to backpack)
    if (piece->hasArtifact(slot) && m_artifactBackpack.size() >= MAX_ARTIFACT_BACKPACK) {
        emit messageLogged(QStringLiteral("圣遗物背包已满，无法卸下旧圣遗物！"));
        return false;
    }
    Artifact a = m_artifactBackpack.takeAt(backpackIndex);
    if (piece->hasArtifact(slot))
        m_artifactBackpack.append(piece->artifact(slot));
    piece->setArtifact(slot, a);
    emit backpackChanged();
    return true;
}

bool GameEngine::unequipWeapon(CharacterBase *piece)
{
    if (!piece || !piece->hasWeapon()) return false;
    if (m_weaponBackpack.size() >= MAX_WEAPON_BACKPACK) {
        emit messageLogged(QStringLiteral("武器背包已满，无法卸下！"));
        return false;
    }
    m_weaponBackpack.append(piece->weapon());
    piece->clearWeapon();
    emit backpackChanged();
    return true;
}

bool GameEngine::unequipArtifact(CharacterBase *piece, ArtifactSlot slot)
{
    if (!piece || !piece->hasArtifact(slot)) return false;
    if (m_artifactBackpack.size() >= MAX_ARTIFACT_BACKPACK) {
        emit messageLogged(QStringLiteral("圣遗物背包已满，无法卸下！"));
        return false;
    }
    m_artifactBackpack.append(piece->artifact(slot));
    piece->clearArtifact(slot);
    emit backpackChanged();
    return true;
}

bool GameEngine::mergeCharacters(int storageIndexA, int storageIndexB)
{
    if (storageIndexA < 0 || storageIndexA >= m_storage.size()) return false;
    if (storageIndexB < 0 || storageIndexB >= m_storage.size()) return false;
    if (storageIndexA == storageIndexB) return false;

    auto *a = m_storage[storageIndexA];
    auto *b = m_storage[storageIndexB];

    if (a->name() != b->name()) {
        emit messageLogged(QStringLiteral("只能合成相同角色！"));
        return false;
    }

    int newConst = a->constellation() + b->constellation() + 1;
    if (newConst > 6) {
        // Will be clamped, warn player
        newConst = 6;
    }

    auto *merged = a;
    merged->setConstellation(newConst);

    // Transfer better equipment from b to a
    if (b->hasWeapon() && b->weapon().stars() > merged->weapon().stars())
        merged->setWeapon(b->weapon());

    for (int i = 0; i < 5; ++i) {
        auto slot = static_cast<ArtifactSlot>(i);
        if (b->hasArtifact(slot) && !merged->hasArtifact(slot))
            merged->setArtifact(slot, b->artifact(slot));
    }

    // Remove b
    delete b;
    // Remove the right index (handle order)
    if (storageIndexB > storageIndexA)
        m_storage.removeAt(storageIndexB);
    else {
        m_storage.removeAt(storageIndexA);
        // merged pointer might have shifted
    }

    emit storageChanged();
    emit messageLogged(QStringLiteral("合成成功！%1命之座提升至%2").arg(merged->name()).arg(merged->constellation()));
    return true;
}

void GameEngine::setPopulationLevel(int l)
{
    m_populationLevel = qBound(INITIAL_POPULATION, l, MAX_POPULATION);
    emit populationChanged();
}

bool GameEngine::upgradePopulation()
{
    if (m_populationLevel >= MAX_POPULATION) {
        emit messageLogged(QStringLiteral("已达最大人口！"));
        return false;
    }
    if (m_primogems < POPULATION_UPGRADE_COST) {
        emit messageLogged(QStringLiteral("原石不足！"));
        return false;
    }
    m_primogems -= POPULATION_UPGRADE_COST;
    m_populationLevel++;
    emit resourcesChanged();
    emit populationChanged();
    emit messageLogged(QStringLiteral("人口升级至%1").arg(m_populationLevel));
    return true;
}

void GameEngine::effectPlay(const QString &type, GridPos pos)
{
    Q_UNUSED(pos);
    emit messageLogged(QStringLiteral("[特效] %1 在 (%2,%3)").arg(type).arg(pos.row).arg(pos.col));
}

CharacterBase* GameEngine::mergeCharactersDirect(CharacterBase *a, CharacterBase *b)
{
    if (!a || !b || a->name() != b->name()) return nullptr;

    int newConst = a->constellation() + b->constellation() + 1;
    newConst = qMin(newConst, 6);

    auto merged = a->clone();
    merged->setConstellation(newConst);

    if (b->hasWeapon() && b->weapon().stars() > (a->hasWeapon() ? a->weapon().stars() : 0))
        merged->setWeapon(b->weapon());
    else if (a->hasWeapon())
        merged->setWeapon(a->weapon());

    for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) {
        auto slot = static_cast<ArtifactSlot>(i);
        if (b->hasArtifact(slot) && !a->hasArtifact(slot))
            merged->setArtifact(slot, b->artifact(slot));
        else if (a->hasArtifact(slot))
            merged->setArtifact(slot, a->artifact(slot));
    }

    return merged.release();
}

QVector<QPair<CharacterBase*, CharacterBase*>> GameEngine::findDuplicates() const
{
    QVector<QPair<CharacterBase*, CharacterBase*>> result;
    QVector<CharacterBase*> all;

    // Collect all player characters (storage + board)
    for (auto *p : m_storage) all.append(p);
    for (auto *p : m_board->playerPieces()) all.append(p);

    for (int i = 0; i < all.size(); ++i) {
        for (int j = i + 1; j < all.size(); ++j) {
            if (all[i]->name() == all[j]->name())
                result.append(QPair<CharacterBase*, CharacterBase*>(all[i], all[j]));
        }
    }
    return result;
}

void GameEngine::registerCharacter(CharacterBase *p)
{
    if (!p) return;
    m_characterRegistry.insert(p->persistentId(), p);
}

void GameEngine::unregisterCharacter(CharacterBase *p)
{
    if (!p) return;
    m_characterRegistry.remove(p->persistentId());
}

CharacterBase* GameEngine::findCharacterById(int persistentId) const
{
    return m_characterRegistry.value(persistentId, nullptr);
}

CharacterBase* GameEngine::createMergedCharacter(CharacterBase *a, CharacterBase *b)
{
    if (!a || !b || a->name() != b->name()) return nullptr;

    int newConst = a->constellation() + b->constellation() + 1;
    newConst = qMin(newConst, 6);

    auto merged = a->clone();
    merged->setConstellation(newConst);

    // Clear all equipment (new character starts empty)
    merged->clearWeapon();
    for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) {
        auto slot = static_cast<ArtifactSlot>(i);
        merged->clearArtifact(slot);
    }

    return merged.release();
}
