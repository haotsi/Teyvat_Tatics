#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include "core/GameTypes.h"

class GameEngine;
class Board;
class PieceItem;
class CharacterBase;

class BoardWidget : public QGraphicsView {
    Q_OBJECT
public:
    explicit BoardWidget(GameEngine *engine, QWidget *parent = nullptr);

    void refreshBoard();
    void setCellSize(int size);

    GridPos gridPosFromScene(const QPointF &scenePos) const;
    QPointF scenePosFromGrid(GridPos pos) const;
    QRectF cellRect(GridPos pos) const;

    int cellSize() const { return m_cellSize; }

signals:
    void pieceSelected(CharacterBase *piece);
    void pieceDroppedOnBoard(int storageIndex, GridPos pos);
    void boardPieceMoved(GridPos from, GridPos to);
    void pieceReturnedToStorage(GridPos pos);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    GameEngine *m_engine;
    QGraphicsScene *m_scene;
    QMap<QString, PieceItem*> m_pieceItems; // key: "row,col"
    int m_cellSize = 72;

    void clearPieces();
    void placePieceItem(CharacterBase *piece);
    PieceItem* itemAt(GridPos pos) const;
    QString posKey(GridPos pos) const;
    QString posKey(int row, int col) const;
};

#endif // BOARDWIDGET_H
