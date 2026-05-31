#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include "core/GameTypes.h"

class GameEngine;
class BoardWidget;
class ShopWidget;
class StorageBar;
class InfoPanel;
class BattleLogWidget;
class BackpackWidget;
class MergePanel;
class CharacterBase;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGame();
    void onSaveGame();
    void onLoadGame();
    void onToggleHelp();
    void onPieceSelected(CharacterBase *piece);
    void onStorageSlotClicked(int index);
    void onPieceDroppedOnBoard(int storageIndex, GridPos pos);
    void onBoardPieceMoved(GridPos from, GridPos to);
    void onPieceReturnedToStorage(GridPos pos);
    void onSellStoragePiece(int index);
    void onPhaseChanged(GamePhase phase);
    void onBattleFinished(bool playerWin);
    void onGameOver(bool playerWin);

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();
    void setupBattleAutoPlay();

    GameEngine *m_engine;
    BoardWidget *m_boardWidget;
    ShopWidget *m_shopWidget;
    StorageBar *m_storageBar;
    InfoPanel *m_infoPanel;
    BattleLogWidget *m_logWidget;
    BackpackWidget *m_backpackWidget = nullptr;
    MergePanel *m_mergePanel = nullptr;

    QPushButton *m_backpackBtn = nullptr;
    QPushButton *m_mergeBtn = nullptr;
    QComboBox *m_difficultyCombo = nullptr;
    QTimer *m_battleTimer;
};

#endif // MAINWINDOW_H
