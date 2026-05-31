#ifndef MERGEPANEL_H
#define MERGEPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSet>
#include "core/GameTypes.h"

class GameEngine;
class CharacterBase;
class MergePanel;

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

    void setEngine(GameEngine *engine) { m_engine = engine; }
    void setPanel(MergePanel *panel) { m_panel = panel; }

    // Source info for delayed deletion
    QString sourceType() const { return m_sourceType; }
    int sourceIndex() const { return m_sourceIndex; }
    GridPos sourcePos() const { return m_sourcePos; }

signals:
    void characterDropped();

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
    MergePanel *m_panel = nullptr;

    // Source info for delayed deletion (only records reference, no ownership)
    QString m_sourceType;     // "storage" / "board" / "none"
    int m_sourceIndex = -1;   // only valid for "storage"
    GridPos m_sourcePos;      // only valid for "board"
};

class MergePanel : public QWidget {
    Q_OBJECT
public:
    explicit MergePanel(GameEngine *engine, QWidget *parent = nullptr);
    void refresh();

    // Reference tracking to prevent same character in multiple slots
    bool isReferenced(CharacterBase *p) const { return m_referencedPieces.contains(p); }
    void addReference(CharacterBase *p) { m_referencedPieces.insert(p); }
    void removeReference(CharacterBase *p) { m_referencedPieces.remove(p); }
    void clearAllReferences() { m_referencedPieces.clear(); }

signals:
    void mergeCompleted(CharacterBase *result);
    void closeRequested();

private:
    GameEngine *m_engine;
    MergeSlotWidget *m_slotA;
    MergeSlotWidget *m_slotB;
    MergeSlotWidget *m_slotResult;
    QPushButton *m_mergeBtn;
    QLabel *m_statusLabel;
    QSet<CharacterBase*> m_referencedPieces;

    void updateMergeState();
    void performMerge();
};

#endif // MERGEPANEL_H
