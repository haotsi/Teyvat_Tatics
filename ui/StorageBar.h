#ifndef STORAGEBAR_H
#define STORAGEBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QVector>
#include <QFrame>

class GameEngine;
class CharacterBase;

class StorageSlotWidget : public QFrame {
    Q_OBJECT
public:
    explicit StorageSlotWidget(int index, QWidget *parent = nullptr);

    void setCharacter(CharacterBase *piece);
    CharacterBase* character() const { return m_character; }
    int storageIndex() const { return m_index; }
    void clear();
    bool isEmpty() const { return m_character == nullptr; }
    void setMergeHighlight(bool h) { m_mergeHighlight = h; update(); }

signals:
    void clicked(int index);
    void dragStarted(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_index;
    CharacterBase *m_character = nullptr;
    QPoint m_dragStartPos;
    bool m_hovered = false;
    bool m_mergeHighlight = false;
};

class StorageBar : public QWidget {
    Q_OBJECT
public:
    explicit StorageBar(GameEngine *engine, QWidget *parent = nullptr);

    void refresh();
    int slotCount() const { return m_slots.size(); }
    StorageSlotWidget* slotAt(int index) const;

signals:
    void slotClicked(int index);
    void pieceDragStarted(int index);

private:
    GameEngine *m_engine;
    QHBoxLayout *m_layout;
    QVector<StorageSlotWidget*> m_slots;
    QLabel *m_titleLabel;
    QLabel *m_countLabel;
};

#endif // STORAGEBAR_H
