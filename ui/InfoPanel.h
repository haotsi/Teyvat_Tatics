#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "core/GameTypes.h"
#include "core/Weapon.h"
#include "core/Artifact.h"

class GameEngine;
class CharacterBase;

class InfoPanel : public QWidget {
    Q_OBJECT
public:
    explicit InfoPanel(GameEngine *engine, QWidget *parent = nullptr);

    void showCharacterInfo(CharacterBase *piece, bool previewOnly = false);
    void showWeaponInfo(const Weapon &weapon, int backpackIndex = -1);
    void showArtifactInfo(const Artifact &artifact, int backpackIndex = -1);
    void clearInfo();
    void refreshResources();

signals:
    void returnPieceClicked(GridPos pos);
    void sellPieceClicked(int storageIndex);
    void sellWeaponRequested(int backpackIndex);
    void sellArtifactRequested(int backpackIndex);
    void upgradePopClicked();
    void unequipWeaponRequested(CharacterBase *piece);
    void unequipArtifactRequested(CharacterBase *piece, ArtifactSlot slot);
    void equipCompleted();

private:
    GameEngine *m_engine;

    QLabel *m_resourceLabel;
    QLabel *m_roundLabel;
    QLabel *m_scoreLabel;
    QLabel *m_popLabel;
    QPushButton *m_upgradePopBtn;
    QPushButton *m_startBattleBtn;

    // Character info section
    QWidget *m_charSection;
    QLabel *m_charNameLabel;
    QLabel *m_charElementLabel;
    QLabel *m_charStatsLabel;
    QLabel *m_charEquipmentLabel;
    QLabel *m_charSkillsLabel;
    QLabel *m_charConstellationLabel;
    QWidget *m_equipContainer;
    QVBoxLayout *m_equipLayout;
    QPushButton *m_returnBtn;
    QPushButton *m_sellBtn;

    CharacterBase *m_selectedPiece = nullptr;
    GridPos m_selectedPos{-1, -1};
    int m_selectedStorageIndex = -1;
    int m_selectedWeaponIndex = -1;
    int m_selectedArtifactIndex = -1;

    void buildUI();
    void clearLayout(QLayout *layout);
};

#endif // INFOPANEL_H
