#include "InfoPanel.h"
#include "core/GameEngine.h"
#include "core/Board.h"
#include "core/CharacterBase.h"
#include "core/Team.h"
#include "ui/DragDropMimeData.h"
#include <QScrollArea>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTimer>
#include <functional>

InfoPanel::InfoPanel(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    buildUI();
    refreshResources();

    connect(m_engine, &GameEngine::resourcesChanged, this, &InfoPanel::refreshResources);
    connect(m_engine, &GameEngine::roundChanged, this, &InfoPanel::refreshResources);
    connect(m_engine, &GameEngine::scoreChanged, this, &InfoPanel::refreshResources);
    connect(m_engine, &GameEngine::populationChanged, this, &InfoPanel::refreshResources);
}

void InfoPanel::buildUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    setStyleSheet("background: #1a1a2e; border-radius: 6px;");
    setMinimumWidth(220);
    setMaximumWidth(280);

    // Resources
    m_resourceLabel = new QLabel;
    m_resourceLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(m_resourceLabel);

    // Round & Score
    auto *infoLayout = new QHBoxLayout;
    m_roundLabel = new QLabel;
    m_roundLabel->setStyleSheet("color: #aad; font-size: 12px;");
    infoLayout->addWidget(m_roundLabel);

    m_scoreLabel = new QLabel;
    m_scoreLabel->setStyleSheet("color: #ffd700; font-size: 12px; font-weight: bold;");
    infoLayout->addWidget(m_scoreLabel);
    mainLayout->addLayout(infoLayout);

    // Population
    auto *popLayout = new QHBoxLayout;
    m_popLabel = new QLabel;
    m_popLabel->setStyleSheet("color: #aaa; font-size: 12px;");
    popLayout->addWidget(m_popLabel);

    m_upgradePopBtn = new QPushButton(QStringLiteral("升级人口"));
    m_upgradePopBtn->setStyleSheet(
        "QPushButton { background: #3a3a5c; color: #ffd700; border: 1px solid #555; "
        "padding: 3px 8px; border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background: #4a4a7c; }"
    );
    connect(m_upgradePopBtn, &QPushButton::clicked, this, [this]() {
        m_engine->upgradePopulation();
        refreshResources();
    });
    popLayout->addWidget(m_upgradePopBtn);
    mainLayout->addLayout(popLayout);

    // Start Battle button
    m_startBattleBtn = new QPushButton(QStringLiteral("开始战斗"));
    m_startBattleBtn->setStyleSheet(
        "QPushButton { background: #8b0000; color: white; font-size: 14px; font-weight: bold; "
        "border: 2px solid #ff4444; padding: 8px; border-radius: 4px; }"
        "QPushButton:hover { background: #a00000; }"
        "QPushButton:disabled { background: #444; border-color: #666; color: #888; }"
    );
    connect(m_startBattleBtn, &QPushButton::clicked, this, [this]() {
        m_engine->startBattlePhase();
    });
    mainLayout->addWidget(m_startBattleBtn);

    // Separator
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #444;");
    mainLayout->addWidget(sep);

    // Team Bonuses
    auto *bonusLabel = new QLabel;
    bonusLabel->setStyleSheet("color: #8cf; font-size: 11px;");
    connect(m_engine, &GameEngine::boardChanged, this, [this, bonusLabel]() {
        bonusLabel->setText(QStringLiteral("羁绊: ") + m_engine->playerTeam()->activeBonusesText());
    });
    bonusLabel->setText(QStringLiteral("羁绊: ") + m_engine->playerTeam()->activeBonusesText());
    mainLayout->addWidget(bonusLabel);

    // Separator
    auto *sep2 = new QFrame;
    sep2->setFrameShape(QFrame::HLine);
    sep2->setStyleSheet("color: #444;");
    mainLayout->addWidget(sep2);

    // Character info section
    m_charSection = new QWidget;
    auto *charLayout = new QVBoxLayout(m_charSection);
    charLayout->setContentsMargins(0, 0, 0, 0);
    charLayout->setSpacing(3);

    m_charNameLabel = new QLabel(QStringLiteral("点击棋子查看信息"));
    m_charNameLabel->setStyleSheet("color: #ffd700; font-size: 14px; font-weight: bold;");
    m_charNameLabel->setWordWrap(true);
    charLayout->addWidget(m_charNameLabel);

    m_charElementLabel = new QLabel;
    m_charElementLabel->setStyleSheet("color: #ccc; font-size: 11px;");
    charLayout->addWidget(m_charElementLabel);

    m_charStatsLabel = new QLabel;
    m_charStatsLabel->setStyleSheet("color: #bbb; font-size: 11px;");
    m_charStatsLabel->setWordWrap(true);
    charLayout->addWidget(m_charStatsLabel);

    m_charSkillsLabel = new QLabel;
    m_charSkillsLabel->setStyleSheet("color: #aac; font-size: 11px;");
    m_charSkillsLabel->setWordWrap(true);
    charLayout->addWidget(m_charSkillsLabel);

    m_charEquipmentLabel = new QLabel;
    m_charEquipmentLabel->setStyleSheet("color: #8b8; font-size: 11px;");
    m_charEquipmentLabel->setWordWrap(true);
    charLayout->addWidget(m_charEquipmentLabel);

    m_equipContainer = new QWidget;
    m_equipLayout = new QVBoxLayout(m_equipContainer);
    m_equipLayout->setContentsMargins(0, 0, 0, 0);
    m_equipLayout->setSpacing(2);
    charLayout->addWidget(m_equipContainer);

    m_charConstellationLabel = new QLabel;
    m_charConstellationLabel->setStyleSheet("color: #e8c; font-size: 11px;");
    charLayout->addWidget(m_charConstellationLabel);

    auto *btnLayout = new QHBoxLayout;
    m_sellBtn = new QPushButton(QStringLiteral("出售"));
    m_sellBtn->setStyleSheet(
        "QPushButton { background: #5a2020; color: #faa; border: 1px solid #844; "
        "padding: 3px 10px; border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background: #7a3030; }"
    );
    m_sellBtn->hide();
    connect(m_sellBtn, &QPushButton::clicked, this, [this]() {
        if (m_selectedStorageIndex >= 0) {
            emit sellPieceClicked(m_selectedStorageIndex);
        }
    });
    btnLayout->addWidget(m_sellBtn);
    btnLayout->addStretch();

    m_returnBtn = new QPushButton(QStringLiteral("撤回储存栏"));
    m_returnBtn->setStyleSheet(
        "QPushButton { background: #2a2a50; color: #aac; border: 1px solid #555; "
        "padding: 3px 10px; border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background: #3a3a70; }"
    );
    m_returnBtn->hide();
    connect(m_returnBtn, &QPushButton::clicked, this, [this]() {
        if (m_selectedPos.isValid())
            emit returnPieceClicked(m_selectedPos);
    });
    btnLayout->addWidget(m_returnBtn);
    charLayout->addLayout(btnLayout);

    mainLayout->addWidget(m_charSection);
    mainLayout->addStretch();
}

void InfoPanel::refreshResources()
{
    m_resourceLabel->setText(QStringLiteral("原石: %1    摩拉: %2")
                             .arg(m_engine->primogems()).arg(m_engine->mora()));
    m_roundLabel->setText(QStringLiteral("第%1/%2轮").arg(m_engine->currentRound()).arg(TOTAL_ROUNDS));
    m_scoreLabel->setText(QStringLiteral("比分 %1:%2").arg(m_engine->playerWins()).arg(m_engine->enemyWins()));
    m_popLabel->setText(QStringLiteral("人口 %1/%2").arg(m_engine->populationLevel()).arg(MAX_POPULATION));

    GamePhase phase = m_engine->phase();
    if (phase == GamePhase::Preparation) {
        m_startBattleBtn->setEnabled(true);
        m_startBattleBtn->setText(QStringLiteral("开始战斗"));
    } else if (phase == GamePhase::Battle) {
        m_startBattleBtn->setEnabled(false);
        m_startBattleBtn->setText(QStringLiteral("战斗中..."));
    } else {
        m_startBattleBtn->setEnabled(false);
        m_startBattleBtn->setText(QStringLiteral("游戏结束"));
    }
}

// Helper class for drag-drop equipment rows
class EquipDropRow : public QFrame {
public:
    EquipDropRow(bool isWeapon, ArtifactSlot artSlot, int pieceId,
                 GameEngine *engine, std::function<void()> onEquipped, QWidget *parent)
        : QFrame(parent), m_isWeapon(isWeapon), m_artSlot(artSlot)
        , m_pieceId(pieceId), m_engine(engine), m_onEquipped(onEquipped)
    {
        setAcceptDrops(true);
        setFixedHeight(16);
        setStyleSheet("EquipDropRow { background: transparent; border: 1px dashed transparent; border-radius: 2px; }");
    }
protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (!m_engine || !m_engine->findCharacterById(m_pieceId)) return;
        auto *mime = DragDropMimeData::fromMimeData(event->mimeData());
        if (!mime) return;
        if (m_isWeapon && mime->sourceType() == QStringLiteral("backpack_weapon")) {
            event->acceptProposedAction();
            setStyleSheet("EquipDropRow { background: #2a3a2a; border: 1px dashed #5a5; border-radius: 2px; }");
        } else if (!m_isWeapon && mime->sourceType() == QStringLiteral("backpack_artifact")) {
            event->acceptProposedAction();
            setStyleSheet("EquipDropRow { background: #2a3a2a; border: 1px dashed #5a5; border-radius: 2px; }");
        }
    }
    void dragLeaveEvent(QDragLeaveEvent*) override {
        setStyleSheet("EquipDropRow { background: transparent; border: 1px dashed transparent; border-radius: 2px; }");
    }
    void dropEvent(QDropEvent *event) override {
        setStyleSheet("EquipDropRow { background: transparent; border: 1px dashed transparent; border-radius: 2px; }");
        auto *mime = DragDropMimeData::fromMimeData(event->mimeData());
        if (!mime || !m_engine) return;
        auto *piece = m_engine->findCharacterById(m_pieceId);
        if (!piece) return;
        int idx = mime->backpackIndex();
        if (idx < 0) return;
        if (m_isWeapon && mime->sourceType() == QStringLiteral("backpack_weapon")) {
            if (m_engine->equipWeapon(piece, idx)) {
                event->acceptProposedAction();
                if (m_onEquipped) {
                    // Delay callback to avoid use-after-free (this may be deleted by callback)
                    QTimer::singleShot(0, this, [cb = m_onEquipped]() { cb(); });
                }
            }
        } else if (!m_isWeapon && mime->sourceType() == QStringLiteral("backpack_artifact")) {
            if (m_engine->equipArtifact(piece, idx, m_artSlot)) {
                event->acceptProposedAction();
                if (m_onEquipped) {
                    QTimer::singleShot(0, this, [cb = m_onEquipped]() { cb(); });
                }
            }
        }
    }
private:
    bool m_isWeapon;
    ArtifactSlot m_artSlot;
    int m_pieceId;
    GameEngine *m_engine;
    std::function<void()> m_onEquipped;
};

void InfoPanel::showCharacterInfo(CharacterBase *piece, bool previewOnly)
{
    m_selectedPiece = piece;
    m_selectedPos = piece ? piece->gridPos() : GridPos{-1, -1};

    if (!piece) {
        clearInfo();
        return;
    }

    auto na = piece->normalAttackInfo();
    auto burst = piece->burstInfo();

    m_charNameLabel->setText(piece->name());
    m_charElementLabel->setText(QStringLiteral("%1 | %2 | 命之座: %3")
                                .arg(elementName(piece->element()))
                                .arg(weaponTypeName(piece->weaponType()))
                                .arg(piece->constellation()));
    m_charStatsLabel->setText(
        QStringLiteral("ATK: %1 | HP: %2/%3\n暴击率: %4% | 暴击伤害: %5%\n元素精通: %6 | 能量: %7/%8")
        .arg(piece->atk(), 0, 'f', 0)
        .arg(piece->currentHp(), 0, 'f', 0)
        .arg(piece->maxHp(), 0, 'f', 0)
        .arg(piece->critRate() * 100, 0, 'f', 1)
        .arg(piece->critDmg() * 100, 0, 'f', 1)
        .arg(piece->eleMastery(), 0, 'f', 0)
        .arg(piece->currentEnergy()).arg(piece->energyMax())
    );
    m_charSkillsLabel->setText(
        QStringLiteral("普攻: %1 (倍率%2%, 元素:%3, 回能:%4)\n大招: %5 (倍率%6%, 元素:%7, 消耗:%8)")
        .arg(na.name).arg(na.multiplier * 100, 0, 'f', 0)
        .arg(elementName(na.elementAttachment)).arg(na.energyRecovery)
        .arg(burst.name).arg(burst.multiplier * 100, 0, 'f', 0)
        .arg(elementName(burst.elementAttachment)).arg(piece->energyMax())
    );

    // Equipment info
    QString equipText = QStringLiteral("武器: ");
    equipText += piece->hasWeapon() ? piece->weapon().name() : QStringLiteral("无");
    equipText += QStringLiteral("\n圣遗物: ");
    QStringList artList;
    for (int i = 0; i < 5; ++i) {
        auto slot = static_cast<ArtifactSlot>(i);
        if (piece->hasArtifact(slot))
            artList << piece->artifact(slot).name();
    }
    equipText += artList.isEmpty() ? QStringLiteral("无") : artList.join(QStringLiteral(", "));
    m_charEquipmentLabel->setText(equipText);

    // Build equipment management UI
    clearLayout(m_equipLayout);
    auto *p = piece;

    if (previewOnly) {
        // Preview mode: show equipment info only, no interactive buttons
        auto *wpnLabel = new QLabel(QStringLiteral("武器: %1")
            .arg(p->hasWeapon() ? p->weapon().name() : QStringLiteral("无")));
        wpnLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
                               .arg(p->hasWeapon() ? QStringLiteral("#ccc") : QStringLiteral("#888")));
        m_equipLayout->addWidget(wpnLabel);

        for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) {
            auto slot = static_cast<ArtifactSlot>(i);
            auto *artLabel = new QLabel(artifactSlotName(slot) + QStringLiteral(": ") +
                (p->hasArtifact(slot) ? p->artifact(slot).name() : QStringLiteral("空")));
            artLabel->setStyleSheet("color: #bbb; font-size: 10px;");
            m_equipLayout->addWidget(artLabel);
        }
    } else {
        // Normal mode: drop zones + unequip buttons
        // Weapon row with drop zone
        auto *weaponRow = new QHBoxLayout;
        auto *wpnLabel = new QLabel(p->hasWeapon() ? p->weapon().name() : QStringLiteral("武器: 无"));
        wpnLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 11px;")
                               .arg(p->hasWeapon() ? QStringLiteral("#ccc") : QStringLiteral("#888")));
        weaponRow->addWidget(wpnLabel, 1);
        if (p->hasWeapon()) {
            auto *unequipBtn = new QPushButton(QStringLiteral("卸下"));
            unequipBtn->setFixedSize(36, 18);
            unequipBtn->setStyleSheet("QPushButton{background:#5a2020;color:#faa;border:1px solid #844;border-radius:2px;font-size:9px;}QPushButton:hover{background:#7a3030;}");
            connect(unequipBtn, &QPushButton::clicked, this, [this, p]() { emit unequipWeaponRequested(p); });
            weaponRow->addWidget(unequipBtn);
        }
        m_equipLayout->addLayout(weaponRow);
        // Weapon drop zone hint
        auto onEquipped = [this, p]() {
            showCharacterInfo(p);
            emit equipCompleted();
        };
        auto *wpnDrop = new EquipDropRow(true, ArtifactSlot::NONE, p->persistentId(), m_engine, onEquipped, m_equipContainer);
        wpnDrop->setToolTip(QStringLiteral("从背包拖拽武器到此处装备"));
        m_equipLayout->addWidget(wpnDrop);

        // Artifact slot rows with drop zones
        for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) {
            auto slot = static_cast<ArtifactSlot>(i);
            auto *artRow = new QHBoxLayout;
            QString artText = artifactSlotName(slot) + QStringLiteral(": ");
            artText += p->hasArtifact(slot) ? p->artifact(slot).name() : QStringLiteral("空");
            auto *artLabel = new QLabel(artText);
            artLabel->setStyleSheet("color: #bbb; font-size: 10px;");
            artRow->addWidget(artLabel, 1);
            if (p->hasArtifact(slot)) {
                auto *unequipBtn = new QPushButton(QStringLiteral("卸下"));
                unequipBtn->setFixedSize(36, 18);
                unequipBtn->setStyleSheet("QPushButton{background:#5a2020;color:#faa;border:1px solid #844;border-radius:2px;font-size:9px;}QPushButton:hover{background:#7a3030;}");
                connect(unequipBtn, &QPushButton::clicked, this, [this, p, slot]() { emit unequipArtifactRequested(p, slot); });
                artRow->addWidget(unequipBtn);
            }
            m_equipLayout->addLayout(artRow);
            // Artifact drop zone
            auto *artDrop = new EquipDropRow(false, slot, p->persistentId(), m_engine, onEquipped, m_equipContainer);
            artDrop->setToolTip(QStringLiteral("从背包拖拽圣遗物到此处装备"));
            m_equipLayout->addWidget(artDrop);
        }
    }

    m_charConstellationLabel->setText(
        QStringLiteral("命之座效果: ATK倍率x%1 | 回能倍率x%2 | 普攻加成+%3% | 大招加成+%4% | 元素伤害+%5%")
        .arg(piece->constellationBaseAtkMultiplier())
        .arg(piece->constellationBaseEnergyRcValMultiplier())
        .arg(piece->constellationNormalAtkBonus() * 100, 0, 'f', 0)
        .arg(piece->constellationBurstBonus() * 100, 0, 'f', 0)
        .arg(piece->constellationBaseDmgBonus() * 100, 0, 'f', 0)
    );

    // Show return button only if piece is on player deploy zone (and not preview)
    bool onBoard = !previewOnly && piece->gridPos().isValid()
                   && m_engine->board()->isPlayerDeployZone(piece->gridPos());
    m_returnBtn->setVisible(onBoard);

    // Show sell button only if piece is in storage (and not preview)
    m_sellBtn->setVisible(false);
    if (!previewOnly) {
        auto &storage = m_engine->storage();
        for (int i = 0; i < storage.size(); ++i) {
            if (storage[i] == piece) {
                m_sellBtn->setVisible(true);
                m_selectedStorageIndex = i;
                int price = 90 + piece->constellation() * 15;
                m_sellBtn->setText(QStringLiteral("出售 (%1原石)").arg(price));
                break;
            }
        }
    }
}

void InfoPanel::clearInfo()
{
    m_selectedPiece = nullptr;
    m_selectedPos = {-1, -1};
    m_selectedStorageIndex = -1;
    m_selectedWeaponIndex = -1;
    m_selectedArtifactIndex = -1;
    m_charNameLabel->setText(QStringLiteral("点击棋子查看信息"));
    m_charElementLabel->clear();
    m_charStatsLabel->clear();
    m_charSkillsLabel->clear();
    m_charEquipmentLabel->clear();
    m_charConstellationLabel->clear();
    m_returnBtn->hide();
    m_sellBtn->hide();
    clearLayout(m_equipLayout);
}

void InfoPanel::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->layout())
            clearLayout(child->layout());
        delete child->widget();
        delete child;
    }
}

void InfoPanel::showWeaponInfo(const Weapon &weapon, int backpackIndex)
{
    clearInfo();
    m_selectedWeaponIndex = backpackIndex;
    m_charNameLabel->setText(weapon.name());
    m_charElementLabel->setText(QStringLiteral("武器"));
    QString starStr;
    for (int i = 0; i < weapon.stars(); ++i) starStr += QStringLiteral("★");
    m_charStatsLabel->setText(
        QStringLiteral("品质: %1\n攻击力: %2\n类型: %3\n购买价: %4 摩拉\n出售价: %5 摩拉")
        .arg(starStr).arg(weapon.atk())
        .arg(weaponTypeName(weapon.type()))
        .arg(weapon.cost()).arg(weapon.sellPrice())
    );
    m_charSkillsLabel->clear();
    m_charEquipmentLabel->clear();
    m_charConstellationLabel->clear();

    if (backpackIndex >= 0) {
        m_sellBtn->setText(QStringLiteral("出售武器"));
        m_sellBtn->show();
        m_sellBtn->disconnect();
        connect(m_sellBtn, &QPushButton::clicked, this, [this]() {
            if (m_selectedWeaponIndex >= 0)
                emit sellWeaponRequested(m_selectedWeaponIndex);
        });
    }
}

void InfoPanel::showArtifactInfo(const Artifact &artifact, int backpackIndex)
{
    clearInfo();
    m_selectedArtifactIndex = backpackIndex;
    m_charNameLabel->setText(artifact.name());
    m_charElementLabel->setText(QStringLiteral("圣遗物"));
    m_charStatsLabel->setText(
        QStringLiteral("部位: %1\n主词条: %2\n售价: %3 摩拉")
        .arg(artifactSlotName(artifact.slot()))
        .arg(statName(artifact.mainStat()))
        .arg(artifact.sellPrice())
    );
    m_charSkillsLabel->clear();
    m_charEquipmentLabel->clear();
    m_charConstellationLabel->clear();

    if (backpackIndex >= 0) {
        m_sellBtn->setText(QStringLiteral("出售圣遗物"));
        m_sellBtn->show();
        m_sellBtn->disconnect();
        connect(m_sellBtn, &QPushButton::clicked, this, [this]() {
            if (m_selectedArtifactIndex >= 0)
                emit sellArtifactRequested(m_selectedArtifactIndex);
        });
    }
}
