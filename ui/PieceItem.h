#ifndef PIECEITEM_H
#define PIECEITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include "core/GameTypes.h"

class CharacterBase;
class GameEngine;

class PieceItem : public QGraphicsObject {
    Q_OBJECT
public:
    explicit PieceItem(CharacterBase *piece, GameEngine *engine, int cellSize, QGraphicsItem *parent = nullptr);

    CharacterBase* piece() const { return m_piece; }
    void setPiece(CharacterBase *piece) { m_piece = piece; update(); }
    void setEngine(GameEngine *engine) { m_engine = engine; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setHighlighted(bool h) { m_highlighted = h; update(); }
    void setMergeHighlight(bool h) { m_mergeHighlight = h; update(); }
    void setCellSize(int s) { m_cellSize = s; update(); }

    // Drag support
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    // Display stats text
    QString statsText() const;

signals:
    void pieceClicked(PieceItem *item);
    void dragStarted(PieceItem *item);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    CharacterBase *m_piece;
    GameEngine *m_engine = nullptr;
    int m_cellSize;
    bool m_highlighted = false;
    bool m_mergeHighlight = false;
    bool m_hovered = false;
    QPointF m_dragStartPos;
    bool m_dragging = false;

    QColor elementColor() const;
};

#endif // PIECEITEM_H
