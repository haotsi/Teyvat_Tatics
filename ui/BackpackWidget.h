#ifndef BACKPACKWIDGET_H
#define BACKPACKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QVector>
#include "core/GameTypes.h"
#include "core/Weapon.h"
#include "core/Artifact.h"

class GameEngine;

class BackpackSlotWidget : public QFrame {
    Q_OBJECT
public:
    enum SlotType { Empty, WeaponSlot, ArtifactSlot };

    explicit BackpackSlotWidget(int index, QWidget *parent = nullptr);

    void setWeapon(const Weapon &w, int sourceIndex);
    void setArtifact(const Artifact &a, int sourceIndex);
    void clear();
    bool isEmpty() const { return m_type == Empty; }
    int index() const { return m_index; }
    int sourceIndex() const { return m_sourceIndex; }
    SlotType slotType() const { return m_type; }

    Weapon weapon() const { return m_weapon; }
    Artifact artifact() const { return m_artifact; }

signals:
    void clicked(int displayIndex);
    void sellRequested(int sourceIndex);
    void dragStarted(int displayIndex);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    int m_index;
    int m_sourceIndex = -1;
    SlotType m_type = Empty;
    Weapon m_weapon;
    Artifact m_artifact;
    bool m_hovered = false;
    QPoint m_dragStartPos;
};

class BackpackWidget : public QWidget {
    Q_OBJECT
public:
    explicit BackpackWidget(GameEngine *engine, QWidget *parent = nullptr);
    void refresh();

signals:
    void weaponClicked(const Weapon &weapon, int backpackIndex);
    void artifactClicked(const Artifact &artifact, int backpackIndex);
    void closeRequested();

protected:
    void showEvent(QShowEvent *event) override;

private:
    GameEngine *m_engine;
    QVBoxLayout *m_mainLayout;
    QVector<BackpackSlotWidget*> m_slots;
    QLabel *m_titleLabel;

    static constexpr int BACKPACK_CAPACITY = 10;
    void buildUI();
};

#endif // BACKPACKWIDGET_H
