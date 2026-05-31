#include "MainWindow.h"
#include "BoardWidget.h"
#include "ShopWidget.h"
#include "StorageBar.h"
#include "InfoPanel.h"
#include "BattleLogWidget.h"
#include "BackpackWidget.h"
#include "MergePanel.h"
#include "core/GameEngine.h"
#include "core/Board.h"
#include "core/CharacterBase.h"
#include "core/SaveManager.h"
#include <QMenuBar>
#include <QMenu>
#include <QComboBox>
#include "core/AIController.h"
#include "core/Shop.h"
#include <QAction>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_engine = new GameEngine(this);
    m_battleTimer = new QTimer(this);
    m_battleTimer->setInterval(800);

    setupUI();
    setupMenuBar();
    setupConnections();
    setupBattleAutoPlay();

    setWindowTitle(QStringLiteral("原神自走棋 - Teyvat Tactics"));
    resize(1200, 780);
    setMinimumSize(1000, 680);

    // Start new game
    m_engine->startNewGame();
    m_boardWidget->refreshBoard();
    m_storageBar->refresh();
    m_shopWidget->refresh();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget;
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // --- Left panel: Info + Shop + Log ---
    auto *leftPanel = new QVBoxLayout;
    leftPanel->setSpacing(6);

    // Difficulty selector
    auto *diffLayout = new QHBoxLayout;
    auto *diffLabel = new QLabel(QStringLiteral("难度:"));
    diffLabel->setStyleSheet("color: #aaa; font-size: 12px;");
    m_difficultyCombo = new QComboBox;
    m_difficultyCombo->addItems({QStringLiteral("简单"), QStringLiteral("普通"), QStringLiteral("困难")});
    m_difficultyCombo->setCurrentIndex(1);
    m_difficultyCombo->setStyleSheet(
        "QComboBox { background: #252540; color: white; border: 1px solid #555; padding: 3px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #1a1a2e; color: white; selection-background-color: #3a3a5c; }"
    );
    diffLayout->addWidget(diffLabel);
    diffLayout->addWidget(m_difficultyCombo, 1);
    leftPanel->addLayout(diffLayout);

    m_infoPanel = new InfoPanel(m_engine);
    leftPanel->addWidget(m_infoPanel);

    m_shopWidget = new ShopWidget(m_engine);
    leftPanel->addWidget(m_shopWidget, 1);

    m_logWidget = new BattleLogWidget(m_engine);
    leftPanel->addWidget(m_logWidget);

    mainLayout->addLayout(leftPanel);

    // --- Center: Board ---
    auto *centerLayout = new QVBoxLayout;
    centerLayout->setSpacing(4);

    // Board title
    auto *boardTitle = new QLabel(QStringLiteral("棋盘 (上方1-4行敌方, 下方5-8行我方部署区)"));
    boardTitle->setStyleSheet("color: #aaa; font-size: 11px;");
    boardTitle->setAlignment(Qt::AlignCenter);
    centerLayout->addWidget(boardTitle);

    m_boardWidget = new BoardWidget(m_engine);
    centerLayout->addWidget(m_boardWidget);

    // Storage bar below board
    m_storageBar = new StorageBar(m_engine);
    centerLayout->addWidget(m_storageBar);

    // Embedded panels (toggle visibility)
    m_backpackWidget = new BackpackWidget(m_engine);
    m_backpackWidget->hide();
    centerLayout->addWidget(m_backpackWidget);

    m_mergePanel = new MergePanel(m_engine);
    m_mergePanel->hide();
    centerLayout->addWidget(m_mergePanel);

    // Buttons row: backpack + merge
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();

    m_backpackBtn = new QPushButton(QStringLiteral("背包"));
    m_backpackBtn->setStyleSheet(
        "QPushButton { background: #2a2a50; color: #aac; border: 1px solid #555; "
        "padding: 6px 14px; border-radius: 3px; font-size: 12px; }"
        "QPushButton:hover { background: #3a3a70; }"
    );
    btnRow->addWidget(m_backpackBtn);

    m_mergeBtn = new QPushButton(QStringLiteral("命之座合成"));
    m_mergeBtn->setStyleSheet(
        "QPushButton { background: #5a3a0a; color: #ffd700; border: 1px solid #886622; "
        "padding: 6px 14px; border-radius: 3px; font-size: 12px; }"
        "QPushButton:hover { background: #7a5a1a; }"
    );
    btnRow->addWidget(m_mergeBtn);
    centerLayout->addLayout(btnRow);

    mainLayout->addLayout(centerLayout);

    // Style the central area
    centralWidget->setStyleSheet("background: #0f0f23;");
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu(QStringLiteral("游戏(&G)"));

    auto *newGameAction = fileMenu->addAction(QStringLiteral("新游戏(&N)"));
    newGameAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+N")));
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);

    auto *saveAction = fileMenu->addAction(QStringLiteral("保存(&S)"));
    saveAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveGame);

    auto *loadAction = fileMenu->addAction(QStringLiteral("读取(&L)"));
    loadAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+L")));
    connect(loadAction, &QAction::triggered, this, &MainWindow::onLoadGame);

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction(QStringLiteral("退出(&Q)"));
    exitAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    auto *viewMenu = menuBar()->addMenu(QStringLiteral("视图(&V)"));

    auto *helpAction = viewMenu->addAction(QStringLiteral("游戏指南(&H)"));
    helpAction->setShortcut(QKeySequence(QStringLiteral("F1")));
    connect(helpAction, &QAction::triggered, this, &MainWindow::onToggleHelp);

    menuBar()->setStyleSheet(
        "QMenuBar { background: #1a1a2e; color: #ccc; }"
        "QMenuBar::item:selected { background: #3a3a5c; }"
        "QMenu { background: #1a1a2e; color: #ccc; border: 1px solid #444; }"
        "QMenu::item:selected { background: #3a3a5c; }"
    );
}

void MainWindow::setupConnections()
{
    // Board piece selection -> Info panel
    connect(m_boardWidget, &BoardWidget::pieceSelected, this, &MainWindow::onPieceSelected);

    // Storage slot click -> Info panel
    connect(m_storageBar, &StorageBar::slotClicked, this, &MainWindow::onStorageSlotClicked);

    // Drag from storage to board
    connect(m_boardWidget, &BoardWidget::pieceDroppedOnBoard, this, &MainWindow::onPieceDroppedOnBoard);

    // Drag within board
    connect(m_boardWidget, &BoardWidget::boardPieceMoved, this, &MainWindow::onBoardPieceMoved);

    // Return piece to storage
    connect(m_boardWidget, &BoardWidget::pieceReturnedToStorage, this, &MainWindow::onPieceReturnedToStorage);

    // Info panel actions
    connect(m_infoPanel, &InfoPanel::returnPieceClicked, this, &MainWindow::onPieceReturnedToStorage);
    connect(m_infoPanel, &InfoPanel::sellPieceClicked, this, &MainWindow::onSellStoragePiece);

    // InfoPanel equip/unequip
    connect(m_infoPanel, &InfoPanel::unequipWeaponRequested, this, [this](CharacterBase *piece) {
        m_engine->unequipWeapon(piece);
        m_infoPanel->showCharacterInfo(piece);
        m_backpackWidget->refresh();
    });
    connect(m_infoPanel, &InfoPanel::unequipArtifactRequested, this, [this](CharacterBase *piece, ArtifactSlot slot) {
        m_engine->unequipArtifact(piece, slot);
        m_infoPanel->showCharacterInfo(piece);
        m_backpackWidget->refresh();
    });
    // Equip completed (from drop zones)
    connect(m_infoPanel, &InfoPanel::equipCompleted, this, [this]() {
        m_backpackWidget->refresh();
    });

    // Difficulty selector
    connect(m_difficultyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
        AIController::Difficulty diff;
        switch (idx) {
            case 0: diff = AIController::Difficulty::Easy; break;
            case 1: diff = AIController::Difficulty::Normal; break;
            case 2: diff = AIController::Difficulty::Hard; break;
            default: diff = AIController::Difficulty::Normal; break;
        }
        m_engine->ai()->setDifficulty(diff);
    });

    // Shop item click -> Info panel (preview mode for characters)
    connect(m_shopWidget, &ShopWidget::itemClicked, this, [this](int index) {
        auto *shop = m_engine->shop();
        auto *item = shop->itemAt(index);
        if (!item) return;
        switch (item->type) {
            case ShopItem::Item_Char:
                if (item->character)
                    m_infoPanel->showCharacterInfo(item->character, true);
                break;
            case ShopItem::Item_Weapon:
                m_infoPanel->showWeaponInfo(item->weapon);
                break;
            case ShopItem::Item_Artifact:
                m_infoPanel->showArtifactInfo(item->artifact);
                break;
        }
    });

    // Phase changes
    connect(m_engine, &GameEngine::phaseChanged, this, &MainWindow::onPhaseChanged);

    // Battle results
    connect(m_engine, &GameEngine::battleFinished, this, &MainWindow::onBattleFinished);

    // Game over
    connect(m_engine, &GameEngine::gameOver, this, &MainWindow::onGameOver);

    // Board change -> refresh
    connect(m_engine, &GameEngine::boardChanged, this, [this]() {
        m_boardWidget->refreshBoard();
        m_storageBar->refresh();
    });

    // Storage change -> refresh
    connect(m_engine, &GameEngine::storageChanged, this, [this]() {
        m_storageBar->refresh();
    });

    // Backpack button -> toggle visibility
    connect(m_backpackBtn, &QPushButton::clicked, this, [this]() {
        m_backpackWidget->setVisible(!m_backpackWidget->isVisible());
    });

    // Merge button -> toggle visibility
    connect(m_mergeBtn, &QPushButton::clicked, this, [this]() {
        m_mergePanel->setVisible(!m_mergePanel->isVisible());
    });

    // Backpack close button
    connect(m_backpackWidget, &BackpackWidget::closeRequested, this, [this]() {
        m_backpackWidget->hide();
    });

    // Merge panel close button
    connect(m_mergePanel, &MergePanel::closeRequested, this, [this]() {
        m_mergePanel->hide();
    });

    // Merge completed -> refresh board & storage
    connect(m_mergePanel, &MergePanel::mergeCompleted, this, [this](CharacterBase*) {
        m_boardWidget->refreshBoard();
        m_storageBar->refresh();
        m_infoPanel->clearInfo();
    });

    // Backpack auto-refresh via signal (only when visible)
    connect(m_engine, &GameEngine::backpackChanged, this, [this]() {
        if (m_backpackWidget->isVisible())
            m_backpackWidget->refresh();
    });

    // Backpack clicks -> InfoPanel
    connect(m_backpackWidget, &BackpackWidget::weaponClicked, this, [this](const Weapon &w, int idx) {
        m_infoPanel->showWeaponInfo(w, idx);
    });
    connect(m_backpackWidget, &BackpackWidget::artifactClicked, this, [this](const Artifact &a, int idx) {
        m_infoPanel->showArtifactInfo(a, idx);
    });

    // InfoPanel sell signals
    connect(m_infoPanel, &InfoPanel::sellWeaponRequested, this, [this](int idx) {
        m_engine->sellWeaponFromBackpack(idx);
        m_infoPanel->clearInfo();
        m_backpackWidget->refresh();
    });
    connect(m_infoPanel, &InfoPanel::sellArtifactRequested, this, [this](int idx) {
        m_engine->sellArtifactFromBackpack(idx);
        m_infoPanel->clearInfo();
        m_backpackWidget->refresh();
    });
}

void MainWindow::setupBattleAutoPlay()
{
    connect(m_battleTimer, &QTimer::timeout, this, [this]() {
        if (m_engine->phase() == GamePhase::Battle) {
            m_engine->executeBattleRound();
            m_boardWidget->refreshBoard();
        }
    });
}

void MainWindow::onNewGame()
{
    auto reply = QMessageBox::question(this, QStringLiteral("新游戏"),
                                        QStringLiteral("确定要开始新游戏吗？当前进度将丢失。"));
    if (reply == QMessageBox::Yes) {
        m_battleTimer->stop();
        m_engine->startNewGame();
        m_boardWidget->refreshBoard();
        m_storageBar->refresh();
        m_shopWidget->refresh();
        m_infoPanel->refreshResources();
        m_infoPanel->clearInfo();
        m_logWidget->clear();
    }
}

void MainWindow::onSaveGame()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("保存游戏"),
                                                 QStringLiteral("savegame.json"),
                                                 QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) return;
    if (SaveManager::saveToFile(path, m_engine))
        QMessageBox::information(this, QStringLiteral("保存"), QStringLiteral("保存成功！"));
    else
        QMessageBox::warning(this, QStringLiteral("保存"), QStringLiteral("保存失败！"));
}

void MainWindow::onLoadGame()
{
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("读取游戏"),
                                                 QStringLiteral("savegame.json"),
                                                 QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) return;
    if (SaveManager::loadFromFile(path, m_engine)) {
        m_boardWidget->refreshBoard();
        m_storageBar->refresh();
        m_shopWidget->refresh();
        m_infoPanel->refreshResources();
        m_infoPanel->clearInfo();
        m_logWidget->clear();
        QMessageBox::information(this, QStringLiteral("读取"), QStringLiteral("读取成功！"));
    } else {
        QMessageBox::warning(this, QStringLiteral("读取"), QStringLiteral("读取失败！"));
    }
}

void MainWindow::onToggleHelp()
{
    QString helpText = QStringLiteral(
        "=== 原神自走棋 游戏指南 ===\n\n"
        "【基本流程】\n"
        "1. 准备阶段：从商店购买棋子/武器/圣遗物\n"
        "2. 将储存栏中的棋子拖拽到我方区域(第5-8行)\n"
        "3. 点击\"开始战斗\"进入战斗阶段\n"
        "4. 战斗自动进行，11局6胜制\n\n"
        "【元素反应】\n"
        "蒸发(水+火): 1.8倍增幅伤害\n"
        "融化(火+冰): 1.8倍增幅伤害\n"
        "超载(火+雷): 3.0倍剧变伤害+击退\n"
        "感电(水+雷): 2.0倍剧变伤害+溅射\n"
        "超导(冰+雷): 1.5倍剧变伤害+溅射\n"
        "冻结(水+冰): 冻结1回合\n"
        "扩散(风+水/火/雷/冰): 0.8倍+扩散元素\n"
        "绽放(水+草): 生成草种子\n"
        "激化(雷+草): 进入激化状态\n"
        "燃烧(火+草): 0.8倍持续伤害\n"
        "结晶(岩+水/火/雷/冰): 生成护盾\n\n"
        "【羁绊】\n"
        "2雷: 能量恢复+5\n"
        "2火: 攻击力+25%\n"
        "2水: 生命值+25%\n"
        "2草: 元素精通+100\n"
        "2冰: 暴击率+15%\n\n"
        "【快捷键】\n"
        "Ctrl+N: 新游戏\n"
        "Ctrl+S: 保存\n"
        "Ctrl+L: 读取\n"
        "F1: 帮助"
    );
    QMessageBox::information(this, QStringLiteral("游戏指南"), helpText);
}

void MainWindow::onPieceSelected(CharacterBase *piece)
{
    m_infoPanel->showCharacterInfo(piece);
}

void MainWindow::onStorageSlotClicked(int index)
{
    auto &storage = m_engine->storage();
    if (index >= 0 && index < storage.size())
        m_infoPanel->showCharacterInfo(storage[index]);
    else
        m_infoPanel->clearInfo();
}

void MainWindow::onPieceDroppedOnBoard(int storageIndex, GridPos pos)
{
    if (m_engine->phase() != GamePhase::Preparation) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("只能在准备阶段部署棋子！"));
        return;
    }

    m_engine->deployFromStorage(storageIndex, pos);
    m_boardWidget->refreshBoard();
    m_storageBar->refresh();
}

void MainWindow::onBoardPieceMoved(GridPos from, GridPos to)
{
    if (m_engine->phase() != GamePhase::Preparation) return;
    m_engine->moveDeployedPiece(from, to);
    m_boardWidget->refreshBoard();
}

void MainWindow::onPieceReturnedToStorage(GridPos pos)
{
    if (m_engine->phase() != GamePhase::Preparation) return;
    m_engine->returnToStorage(pos);
    m_boardWidget->refreshBoard();
    m_storageBar->refresh();
    m_infoPanel->clearInfo();
}

void MainWindow::onSellStoragePiece(int index)
{
    if (m_engine->phase() != GamePhase::Preparation) return;
    auto reply = QMessageBox::question(this, QStringLiteral("出售"),
                                        QStringLiteral("确定出售此棋子吗？"));
    if (reply == QMessageBox::Yes) {
        m_engine->sellStorageItem(index);
        m_storageBar->refresh();
        m_infoPanel->clearInfo();
    }
}

void MainWindow::onPhaseChanged(GamePhase phase)
{
    m_infoPanel->refreshResources();

    if (phase == GamePhase::Battle) {
        m_shopWidget->setEnabled(false);
        m_storageBar->setEnabled(false);
        m_logWidget->clear();
        m_battleTimer->start();
    } else if (phase == GamePhase::Preparation) {
        m_battleTimer->stop();
        m_shopWidget->setEnabled(true);
        m_storageBar->setEnabled(true);
        m_boardWidget->refreshBoard();
        m_storageBar->refresh();
    } else {
        m_battleTimer->stop();
    }
}

void MainWindow::onBattleFinished(bool playerWin)
{
    m_battleTimer->stop();
    m_boardWidget->refreshBoard();

    QString result = playerWin ? QStringLiteral("胜利！") : QStringLiteral("失败。");
    QMessageBox::information(this, QStringLiteral("战斗结束"),
                             QStringLiteral("第%1轮战斗%2\n比分: %3:%4")
                             .arg(m_engine->currentRound()).arg(result)
                             .arg(m_engine->playerWins()).arg(m_engine->enemyWins()));

    m_shopWidget->refresh();
}

void MainWindow::onGameOver(bool playerWin)
{
    m_battleTimer->stop();
    QString title = playerWin ? QStringLiteral("恭喜！") : QStringLiteral("遗憾！");
    QString msg = playerWin
        ? QStringLiteral("你赢得了最终胜利！\n比分: %1:%2").arg(m_engine->playerWins()).arg(m_engine->enemyWins())
        : QStringLiteral("你输了。\n比分: %1:%2").arg(m_engine->playerWins()).arg(m_engine->enemyWins());

    QMessageBox::information(this, title, msg);

    auto reply = QMessageBox::question(this, QStringLiteral("再来一局？"),
                                        QStringLiteral("是否开始新游戏？"));
    if (reply == QMessageBox::Yes) {
        m_engine->startNewGame();
        m_boardWidget->refreshBoard();
        m_storageBar->refresh();
        m_shopWidget->refresh();
        m_infoPanel->refreshResources();
        m_infoPanel->clearInfo();
        m_logWidget->clear();
    }
}
