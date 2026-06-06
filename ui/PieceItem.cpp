#include "PieceItem.h"
#include "core/CharacterBase.h"
#include "core/GameEngine.h"
#include "DragDropMimeData.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDrag>
#include <QWidget>
#include <QCursor>
#include <QFontMetrics>
#include <QtMath>

PieceItem::PieceItem(CharacterBase *piece, GameEngine *engine, int cellSize, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_piece(piece), m_engine(engine), m_cellSize(cellSize)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::OpenHandCursor));
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(10);
}

QRectF PieceItem::boundingRect() const
{
    double margin = 2;
    return QRectF(-m_cellSize/2.0 + margin, -m_cellSize/2.0 + margin,
                  m_cellSize - 2*margin, m_cellSize - 2*margin);
}

QColor PieceItem::elementColor() const
{
    if (!m_piece) return Qt::gray;
    switch (m_piece->element()) {
        case ElementType::Hydro:  return QColor(0, 128, 255);
        case ElementType::Pyro:   return QColor(255, 80, 40);
        case ElementType::Electro: return QColor(160, 80, 255);
        case ElementType::Cryo:   return QColor(140, 220, 255);
        case ElementType::Dendro: return QColor(80, 200, 60);
        case ElementType::Anemo:  return QColor(120, 200, 180);
        case ElementType::Geo:    return QColor(200, 160, 60);
        default: return Qt::gray;
    }
}

void PieceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_piece) return;

    painter->setRenderHint(QPainter::Antialiasing);

    QRectF r = boundingRect();
    double radius = r.width() / 2.0;
    QPointF center(0, 0);

    // Dead pieces are semi-transparent ghosts
    if (!m_piece->isAlive()) {
        painter->setOpacity(0.3);
    } else if (m_dragging) {
        painter->setOpacity(0.5);
    }

    // Draw team-side base ring (enemy/ally identification)
    QColor sideBaseColor = (m_piece->side() == TeamSide::Player)
        ? QColor(0, 180, 40, 180) : QColor(220, 30, 30, 180);
    painter->setBrush(sideBaseColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(center, radius + 3, radius + 3);

    // Draw circle (placeholder for character model)
    QColor elemCol = elementColor();
    QRadialGradient gradient(center, radius);
    gradient.setColorAt(0, elemCol.lighter(150));
    gradient.setColorAt(1, elemCol.darker(120));

    painter->setBrush(gradient);
    if (m_hovered || isSelected()) {
        painter->setPen(QPen(Qt::yellow, 2.5));
    } else if (m_mergeHighlight) {
        painter->setPen(QPen(QColor(255, 215, 0), 3));
    } else {
        painter->setPen(QPen(elemCol.darker(150), 1.5));
    }
    painter->drawEllipse(center, radius, radius);

    // Draw character name with constellation color
    QFont font;
    font.setPixelSize(static_cast<int>(radius * 0.55));
    font.setBold(true);
    painter->setFont(font);
    QColor nameCol(constellationColor(m_piece->constellation()));
    painter->setPen(nameCol);

    QString displayName = m_piece->name().left(3);
    QFontMetrics fm(font);
    QRectF textRect(-radius, -fm.height()/2, radius*2, fm.height());
    painter->drawText(textRect, Qt::AlignCenter, displayName);

    // HP bar below the circle
    if (m_piece->isAlive()) {
        double hpRatio = m_piece->currentHp() / m_piece->maxHp();
        double barWidth = radius * 1.6;
        double barHeight = 4;
        double barY = radius + 4;

        // Background
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(40, 40, 40));
        painter->drawRoundedRect(QRectF(-barWidth/2, barY, barWidth, barHeight), 2, 2);

        // HP fill
        QColor hpColor = (hpRatio > 0.5) ? Qt::green : (hpRatio > 0.25) ? QColor(255, 165, 0) : Qt::red;
        painter->setBrush(hpColor);
        painter->drawRoundedRect(QRectF(-barWidth/2, barY, barWidth * hpRatio, barHeight), 2, 2);

        // Energy bar
        if (m_piece->energyMax() > 0) {
            double energyRatio = static_cast<double>(m_piece->currentEnergy()) / m_piece->energyMax();
            double energyY = barY + barHeight + 2;
            painter->setBrush(QColor(60, 60, 60));
            painter->drawRoundedRect(QRectF(-barWidth/2, energyY, barWidth, 3), 1.5, 1.5);
            painter->setBrush(energyRatio >= 1.0 ? QColor(255, 215, 0) : QColor(100, 180, 255));
            painter->drawRoundedRect(QRectF(-barWidth/2, energyY, barWidth * energyRatio, 3), 1.5, 1.5);
        }
    }
}

void PieceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Dead pieces cannot be interacted with
    if (!m_piece || !m_piece->isAlive()) return;
    // No dragging during battle
    if (m_engine && m_engine->phase() != GamePhase::Preparation) return;
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
        m_dragging = false;
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
    QGraphicsObject::mousePressEvent(event);
}

void PieceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_piece || !m_piece->isAlive()) return;
    if (m_engine && m_engine->phase() != GamePhase::Preparation) return;
    if (!m_dragging && (event->pos() - m_dragStartPos).manhattanLength() > 10) {
        m_dragging = true;
        emit dragStarted(this);

        // Save data before entering nested event loop
        GridPos savedPos = m_piece->gridPos();
        auto *mimeData = new DragDropMimeData();
        mimeData->setSourceType(QStringLiteral("board"));
        mimeData->setGridPos(savedPos.row, savedPos.col);

        QWidget *dragSource = event->widget();
        if (!dragSource) {
            auto views = scene()->views();
            if (!views.isEmpty()) dragSource = views.first();
        }
        if (!dragSource) return;

        auto *drag = new QDrag(dragSource);
        drag->setMimeData(mimeData);

        QPixmap pixmap(static_cast<int>(boundingRect().width()),
                       static_cast<int>(boundingRect().height()));
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        paint(&p, nullptr, nullptr);
        p.end();
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));
        drag->exec(Qt::MoveAction);

        m_dragging = false;
        update();
    }
}

void PieceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_dragging && m_piece) {
        emit pieceClicked(this);
    }
    m_dragging = false;
    setCursor(QCursor(Qt::OpenHandCursor));
    QGraphicsObject::mouseReleaseEvent(event);
}

void PieceItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = true;
    update();
}

void PieceItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = false;
    update();
}

QString PieceItem::statsText() const
{
    if (!m_piece) return {};
    return QStringLiteral(
        "%1\n职业: %2\n元素: %3\nATK: %4\nHP: %5/%6\n"
        "暴击率: %7%\n暴击伤害: %8%\n元素精通: %9\n命之座: %10")
        .arg(m_piece->name())
        .arg(weaponTypeName(m_piece->weaponType()))
        .arg(elementName(m_piece->element()))
        .arg(m_piece->atk(), 0, 'f', 0)
        .arg(m_piece->currentHp(), 0, 'f', 0)
        .arg(m_piece->maxHp(), 0, 'f', 0)
        .arg(m_piece->critRate() * 100, 0, 'f', 1)
        .arg(m_piece->critDmg() * 100, 0, 'f', 1)
        .arg(m_piece->eleMastery(), 0, 'f', 0)
        .arg(m_piece->constellation());
}
