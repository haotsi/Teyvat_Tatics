#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "GameTypes.h"
#include "Weapon.h"
#include "Artifact.h"
#include "ElementSystem.h"
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QMap>
#include <QHash>
#include <QPair>
#include <memory>
#include <vector>

// Backpack capacity limits
constexpr int MAX_WEAPON_BACKPACK = 10;
constexpr int MAX_ARTIFACT_BACKPACK = 10;

class Board;
class Team;
class Shop;
class AIController;
class CharacterBase;
struct BattleAction;

class GameEngine : public QObject {
    Q_OBJECT
    Q_PROPERTY(int primogems READ primogems NOTIFY resourcesChanged)
    Q_PROPERTY(int mora READ mora NOTIFY resourcesChanged)
    Q_PROPERTY(int currentRound READ currentRound NOTIFY roundChanged)
    Q_PROPERTY(int playerWins READ playerWins NOTIFY scoreChanged)
    Q_PROPERTY(int enemyWins READ enemyWins NOTIFY scoreChanged)
    Q_PROPERTY(int populationLevel READ populationLevel NOTIFY populationChanged)
    Q_PROPERTY(int maxPopulation READ maxPopulation CONSTANT)
    Q_PROPERTY(int phase READ phaseInt NOTIFY phaseChanged)

public:
    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    // Core components
    Board *board() const { return m_board; }
    Team *playerTeam() const { return m_playerTeam; }
    Team *enemyTeam() const { return m_enemyTeam; }
    Shop *shop() const { return m_shop; }
    AIController *ai() const { return m_ai; }
    ElementSystem *elementSystem() const { return m_elementSystem; }

    // Resources
    int primogems() const { return m_primogems; }
    int mora() const { return m_mora; }
    void setPrimogems(int p) { m_primogems = p; emit resourcesChanged(); }
    void setMora(int m) { m_mora = m; emit resourcesChanged(); }

    // Round & score
    int currentRound() const { return m_currentRound; }
    int playerWins() const { return m_playerWins; }
    int enemyWins() const { return m_enemyWins; }
    void setCurrentRound(int r) { m_currentRound = r; emit roundChanged(); }
    void setPlayerWins(int w) { m_playerWins = w; emit scoreChanged(); }
    void setEnemyWins(int w) { m_enemyWins = w; emit scoreChanged(); }

    // Population
    int populationLevel() const { return m_populationLevel; }
    void setPopulationLevel(int l);
    int maxPopulation() const { return MAX_POPULATION; }
    bool upgradePopulation();

    // Phase
    GamePhase phase() const { return m_phase; }
    int phaseInt() const { return static_cast<int>(m_phase); }

    // Character pool (all available characters)
    const std::vector<std::unique_ptr<CharacterBase>> &characterPool() const { return m_characterPool; }

    // Storage (bench)
    QVector<CharacterBase*> &storage() { return m_storage; }
    int storageCapacity() const { return STORAGE_CAPACITY; }
    bool addToStorage(CharacterBase *piece);
    void removeFromStorage(CharacterBase *piece);
    CharacterBase* takeFromStorage(int index);

    // Backpack for equipment
    QVector<Weapon> &weaponBackpack() { return m_weaponBackpack; }
    QVector<Artifact> &artifactBackpack() { return m_artifactBackpack; }

    // Game flow
    void startNewGame();
    void startPreparationPhase();
    void startBattlePhase();
    void onBattleFinished(bool playerWin);
    void endGame();

    // Battle system
    void executeBattleStep();
    void executeBattleRound(); // full round: player acts, then enemy acts
    bool isBattleOver() const;

    // Shop
    void refreshShop();
    bool manualRefreshShop();
    bool buyShopItem(int index);
    bool sellStorageItem(int index);
    bool sellWeaponFromBackpack(int index);
    bool sellArtifactFromBackpack(int index);

    // Board deployment
    bool deployFromStorage(int storageIndex, GridPos pos);
    bool returnToStorage(GridPos pos);
    bool moveDeployedPiece(GridPos from, GridPos to);

    // Equipment management
    bool equipWeapon(CharacterBase *piece, int backpackIndex);
    bool equipArtifact(CharacterBase *piece, int backpackIndex, ArtifactSlot slot);
    bool unequipWeapon(CharacterBase *piece);
    bool unequipArtifact(CharacterBase *piece, ArtifactSlot slot);

    // Constellation / merge
    bool mergeCharacters(int storageIndexA, int storageIndexB);
    CharacterBase* mergeCharactersDirect(CharacterBase *a, CharacterBase *b);
    CharacterBase* createMergedCharacter(CharacterBase *a, CharacterBase *b);
    QVector<QPair<CharacterBase*, CharacterBase*>> findDuplicates() const;

    // Character registry for safe ID-based lookup
    void registerCharacter(CharacterBase *p);
    void unregisterCharacter(CharacterBase *p);
    CharacterBase* findCharacterById(int persistentId) const;

    // Interest
    void calculateInterest();

    // Battle log
    const QVector<BattleAction> &battleLog() const { return m_battleLog; }
    void clearBattleLog() { m_battleLog.clear(); }

    // Save/Load helpers
    friend class SaveManager;

    // Effect interface for testing
    void effectPlay(const QString &type, GridPos pos);

signals:
    void resourcesChanged();
    void roundChanged();
    void scoreChanged();
    void populationChanged();
    void phaseChanged(GamePhase phase);
    void shopChanged();
    void boardChanged();
    void storageChanged();
    void battleActionOccurred(const BattleAction &action);
    void battleFinished(bool playerWin);
    void gameOver(bool playerWin);
    void messageLogged(const QString &message);
    void backpackChanged();

private:
    Board *m_board;
    Team *m_playerTeam;
    Team *m_enemyTeam;
    Shop *m_shop;
    AIController *m_ai;
    ElementSystem *m_elementSystem;

    int m_primogems = INITIAL_PRIMOGEMS;
    int m_mora = INITIAL_MORA;
    int m_currentRound = 0;
    int m_playerWins = 0;
    int m_enemyWins = 0;
    int m_populationLevel = INITIAL_POPULATION;

    GamePhase m_phase = GamePhase::Preparation;

    std::vector<std::unique_ptr<CharacterBase>> m_characterPool;
    std::vector<std::unique_ptr<CharacterBase>> m_enemyCharacters; // enemy pieces for current battle
    QVector<CharacterBase*> m_storage; // owned characters not on board
    QVector<Weapon> m_weaponBackpack;
    QVector<Artifact> m_artifactBackpack;
    QHash<int, CharacterBase*> m_characterRegistry; // persistentId -> character lookup

    QVector<BattleAction> m_battleLog;

    // Battle state tracking
    QVector<CharacterBase*> m_playerBattleOrder;
    QVector<CharacterBase*> m_enemyBattleOrder;
    QMap<CharacterBase*, GridPos> m_savedPlayerPositions;
    int m_currentActorIndex = 0;
    bool m_playerTurn = true;
    bool m_battlePhaseActive = false;

    // Internal battle methods
    void setupBattle();
    void processCharacterAction(CharacterBase *piece);
    void processAttack(CharacterBase *attacker, CharacterBase *defender, bool isBurst);
    void applyKnockback(CharacterBase *target);
    void processReaction(CharacterBase *attacker, CharacterBase *defender,
                         const ReactionResult &reaction);
    void buildBattleOrder();
    void tickAllAuras(TeamSide side);
    void generateEnemyTeam();
};

#endif // GAMEENGINE_H
