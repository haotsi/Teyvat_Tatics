#ifndef MERGEPANEL_H
#define MERGEPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include "core/GameTypes.h"

class GameEngine;
class CharacterBase;

class MergeSlotWidget : public QFrame {
    Q_OBJECT
public:
    MergeSlotWidget(bool acceptDrops, QWidget *parent = nullptr);
    void setCharacter(CharacterBase *piece);
    CharacterBase* character() const { return m_character; }
    bool isEmpty() const { return m_character == nullptr; }
    void clear();
    void setHighlighted(bool h) { m_highlighted = h; update(); }
    CharacterBase* takeCharacter();

protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    CharacterBase *m_character = nullptr;
    bool m_acceptDrops;
    bool m_highlighted = false;
    GameEngine *m_engine = nullptr;
};

class MergePanel : public QWidget {
    Q_OBJECT
public:
    explicit MergePanel(GameEngine *engine, QWidget *parent = nullptr);
    void refresh();

signals:
    void mergeCompleted(CharacterBase *result);

private:
    GameEngine *m_engine;
    MergeSlotWidget *m_slotA;
    MergeSlotWidget *m_slotB;
    MergeSlotWidget *m_slotResult;
    QPushButton *m_mergeBtn;
    QLabel *m_statusLabel;

    void updateMergeState();
    void performMerge();
};

#endif // MERGEPANEL_H
