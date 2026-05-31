#include "BoardWidget.h"
#include "PieceItem.h"
#include "DragDropMimeData.h"
#include "core/GameEngine.h"
#include "core/Board.h"
#include "core/CharacterBase.h"
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

BoardWidget::BoardWidget(GameEngine *engine, QWidget *parent)
    : QGraphicsView(parent), m_engine(engine)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    int totalW = BOARD_COLS * m_cellSize;
    int totalH = BOARD_ROWS * m_cellSize;
    m_scene->setSceneRect(0, 0, totalW, totalH);

    setFixedSize(totalW + 4, totalH + 4);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setAcceptDrops(true);
    setDragMode(QGraphicsView::NoDrag);

    setStyleSheet("background: transparent; border: 2px solid #444;");
}

void BoardWidget::setCellSize(int size)
{
    m_cellSize = size;
    int totalW = BOARD_COLS * m_cellSize;
    int totalH = BOARD_ROWS * m_cellSize;
    m_scene->setSceneRect(0, 0, totalW, totalH);
    setFixedSize(totalW + 4, totalH + 4);

    for (auto *item : m_pieceItems)
        item->setCellSize(m_cellSize);
    refreshBoard();
}

void BoardWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            QRectF cellRect(c * m_cellSize, r * m_cellSize, m_cellSize, m_cellSize);

            // Color based on zone
            if (r >= PLAYER_START_ROW - 1) {
                // Player zone (rows 5-8 in 1-indexed = rows 4-7 in 0-indexed)
                // Actually: player deploys in rows 5-8, which are rows 4-7 (0-indexed)
                painter->fillRect(cellRect, (r + c) % 2 == 0 ? QColor(25, 60, 95) : QColor(20, 50, 80));
            } else {
                // Enemy zone (rows 1-4)
                painter->fillRect(cellRect, (r + c) % 2 == 0 ? QColor(80, 35, 35) : QColor(65, 28, 28));
            }

            // Grid lines
            painter->setPen(QPen(QColor(60, 60, 60), 0.5));
            painter->drawRect(cellRect);

            // Coordinate labels
            if (c == 0) {
                painter->setPen(QColor(150, 150, 150));
                QFont font;
                font.setPixelSize(9);
                painter->setFont(font);
                painter->drawText(QRectF(2, r * m_cellSize, 14, 14), Qt::AlignCenter,
                                  QString::number(r + 1));
            }
            if (r == 0) {
                painter->setPen(QColor(150, 150, 150));
                QFont font;
                font.setPixelSize(9);
                painter->setFont(font);
                painter->drawText(QRectF(c * m_cellSize + m_cellSize - 14, 2, 14, 14), Qt::AlignCenter,
                                  QString(QChar('A' + c)));
            }
        }
    }

    // Draw dividing line between enemy and player zones
    painter->setPen(QPen(QColor(255, 215, 0, 100), 2));
    double y = PLAYER_START_ROW * m_cellSize;
    painter->drawLine(QPointF(0, y), QPointF(BOARD_COLS * m_cellSize, y));
}

void BoardWidget::refreshBoard()
{
    // Remove all existing piece items
    qDeleteAll(m_pieceItems);
    m_pieceItems.clear();

    auto *board = m_engine->board();
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            auto *piece = board->pieceAt(r, c);
            if (piece)
                placePieceItem(piece);
        }
    }
    m_scene->update();
}

void BoardWidget::clearPieces()
{
    qDeleteAll(m_pieceItems);
    m_pieceItems.clear();
}

void BoardWidget::placePieceItem(CharacterBase *piece)
{
    if (!piece) return;

    GridPos pos = piece->gridPos();
    auto *item = new PieceItem(piece, m_cellSize);
    QPointF scenePos = scenePosFromGrid(pos);
    item->setPos(scenePos);
    m_scene->addItem(item);
    m_pieceItems[posKey(pos)] = item;

    connect(item, &PieceItem::pieceClicked, this, [this](PieceItem *i) {
        emit pieceSelected(i->piece());
    });
    connect(item, &PieceItem::dragStarted, this, [this](PieceItem *i) {
        Q_UNUSED(i);
        // Piece is being dragged from board - it may be dropped back on board or on storage
    });
}

PieceItem* BoardWidget::itemAt(GridPos pos) const
{
    return m_pieceItems.value(posKey(pos), nullptr);
}

QString BoardWidget::posKey(GridPos pos) const { return posKey(pos.row, pos.col); }
QString BoardWidget::posKey(int row, int col) const { return QStringLiteral("%1,%2").arg(row).arg(col); }

GridPos BoardWidget::gridPosFromScene(const QPointF &scenePos) const
{
    int col = static_cast<int>(scenePos.x()) / m_cellSize;
    int row = static_cast<int>(scenePos.y()) / m_cellSize;
    return {row, col};
}

QPointF BoardWidget::scenePosFromGrid(GridPos pos) const
{
    return QPointF(pos.col * m_cellSize + m_cellSize / 2.0,
                   pos.row * m_cellSize + m_cellSize / 2.0);
}

QRectF BoardWidget::cellRect(GridPos pos) const
{
    return QRectF(pos.col * m_cellSize, pos.row * m_cellSize, m_cellSize, m_cellSize);
}

void BoardWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (DragDropMimeData::isPieceDrag(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void BoardWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (DragDropMimeData::isPieceDrag(event->mimeData())) {
        event->acceptProposedAction();
    }
}

void BoardWidget::dropEvent(QDropEvent *event)
{
    auto *mimeData = DragDropMimeData::fromMimeData(event->mimeData());
    if (!mimeData) return;

    GridPos targetPos = gridPosFromScene(mapToScene(event->position().toPoint()));

    if (mimeData->sourceType() == QStringLiteral("storage")) {
        // Dragged from storage bar to board
        if (targetPos.isValid()) {
            emit pieceDroppedOnBoard(mimeData->sourceIndex(), targetPos);
        }
    } else if (mimeData->sourceType() == QStringLiteral("board")) {
        // Dragged from one board position to another
        GridPos fromPos{mimeData->gridRow(), mimeData->gridCol()};
        if (targetPos.isValid() && targetPos != fromPos) {
            // Check if this is a return-to-storage (dropped on non-deploy zone)
            auto *board = m_engine->board();
            if (!board->isPlayerDeployZone(targetPos) && board->isPlayerDeployZone(fromPos)) {
                emit pieceReturnedToStorage(fromPos);
            } else {
                emit boardPieceMoved(fromPos, targetPos);
            }
        }
    }

    event->acceptProposedAction();
    refreshBoard();
}

void BoardWidget::mousePressEvent(QMouseEvent *event)
{
    // Deselect all when clicking empty space
    QGraphicsView::mousePressEvent(event);
    GridPos gp = gridPosFromScene(mapToScene(event->pos()));
    if (!itemAt(gp)) {
        m_scene->clearSelection();
        emit pieceSelected(nullptr);
    }
}
