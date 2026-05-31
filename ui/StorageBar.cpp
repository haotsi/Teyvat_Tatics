#include "StorageBar.h"
#include "core/GameEngine.h"
#include "core/CharacterBase.h"
#include "ui/DragDropMimeData.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDrag>
#include <QCursor>
#include <QFontMetrics>
#include <QToolTip>

// --- StorageSlotWidget ---

StorageSlotWidget::StorageSlotWidget(int index, QWidget *parent)
    : QFrame(parent), m_index(index)
{
    setFixedSize(64, 78);
    setAcceptDrops(true);
    setFrameStyle(QFrame::Box);
    setStyleSheet(
        "StorageSlotWidget { background: #1e1e35; border: 1px solid #444; border-radius: 4px; }"
    );
    setCursor(QCursor(Qt::OpenHandCursor));
}

void StorageSlotWidget::setCharacter(CharacterBase *piece)
{
    m_character = piece;
    update();
}

void StorageSlotWidget::clear()
{
    m_character = nullptr;
    update();
}

void StorageSlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_hovered) {
        painter.fillRect(rect(), QColor(255, 255, 255, 20));
    }

    if (!m_character) {
        painter.setPen(QColor(80, 80, 80));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("空"));
        return;
    }

    // Draw mini portrait
    QColor elemCol;
    switch (m_character->element()) {
        case ElementType::Hydro:  elemCol = QColor(0, 128, 255); break;
        case ElementType::Pyro:   elemCol = QColor(255, 80, 40); break;
        case ElementType::Electro: elemCol = QColor(160, 80, 255); break;
        case ElementType::Cryo:   elemCol = QColor(140, 220, 255); break;
        case ElementType::Dendro: elemCol = QColor(80, 200, 60); break;
        case ElementType::Anemo:  elemCol = QColor(120, 200, 180); break;
        case ElementType::Geo:    elemCol = QColor(200, 160, 60); break;
        default: elemCol = Qt::gray;
    }

    QRectF circleRect(width()/2 - 18, 8, 36, 36);
    QRadialGradient grad(circleRect.center(), 18);
    grad.setColorAt(0, elemCol.lighter(150));
    grad.setColorAt(1, elemCol.darker(120));
    painter.setBrush(grad);
    painter.setPen(QPen(elemCol.darker(150), 1));
    painter.drawEllipse(circleRect);

    // Merge highlight border
    if (m_mergeHighlight) {
        painter.setPen(QPen(QColor(255, 215, 0), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(1, 1, width()-2, height()-2), 5, 5);
    }

    // Name with constellation color
    QFont font;
    font.setPixelSize(10);
    font.setBold(true);
    painter.setFont(font);
    QColor consCol(constellationColor(m_character->constellation()));
    painter.setPen(consCol);
    QRectF nameRect(width()/2 - 20, 46, 40, 14);
    painter.drawText(nameRect, Qt::AlignCenter, m_character->name());

    // Constellation dots
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < 6; ++i) {
        int x = width()/2 - 10 + i * 4;
        painter.setBrush((i < m_character->constellation()) ? QColor(255, 200, 50) : QColor(60, 60, 60));
        painter.drawEllipse(QPointF(x, 64), 2, 2);
    }
}

void StorageSlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
}

void StorageSlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_character) return;
    if ((event->pos() - m_dragStartPos).manhattanLength() < 10) return;

    emit dragStarted(m_index);

    auto *mimeData = new DragDropMimeData();
    mimeData->setSourceType(QStringLiteral("storage"));
    mimeData->setSourceIndex(m_index);

    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    QPixmap pixmap(36, 36);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setBrush(QColor(255, 255, 255, 180));
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, 36, 36);
    p.setPen(Qt::black);
    QFont f;
    f.setPixelSize(10);
    p.setFont(f);
    p.drawText(QRect(0, 8, 36, 20), Qt::AlignCenter, m_character->name().left(2));
    p.end();
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(18, 18));
    drag->exec(Qt::MoveAction);

    setCursor(QCursor(Qt::OpenHandCursor));
}

void StorageSlotWidget::enterEvent(QEnterEvent *)
{
    m_hovered = true;
    if (m_character) {
        setToolTip(m_character->name() + QStringLiteral("\n命之座: ") +
                   QString::number(m_character->constellation()));
    }
    update();
}

void StorageSlotWidget::leaveEvent(QEvent *)
{
    m_hovered = false;
    update();
}

// --- StorageBar ---

StorageBar::StorageBar(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 4, 8, 4);
    m_layout->setSpacing(4);

    m_titleLabel = new QLabel(QStringLiteral("储存栏"));
    m_titleLabel->setStyleSheet("color: #aaa; font-size: 11px; font-weight: bold;");
    m_layout->addWidget(m_titleLabel);

    for (int i = 0; i < STORAGE_CAPACITY; ++i) {
        auto *slot = new StorageSlotWidget(i);
        m_slots.append(slot);
        m_layout->addWidget(slot);

        connect(slot, &StorageSlotWidget::clicked, this, [this](int idx) {
            emit slotClicked(idx);
        });
        connect(slot, &StorageSlotWidget::dragStarted, this, [this](int idx) {
            emit pieceDragStarted(idx);
        });
    }

    m_countLabel = new QLabel;
    m_countLabel->setStyleSheet("color: #888; font-size: 10px;");
    m_layout->addWidget(m_countLabel);

    m_layout->addStretch();

    setStyleSheet("background: #15152a; border-radius: 6px;");
    setFixedHeight(90);
    setAcceptDrops(true);
}

void StorageBar::refresh()
{
    auto &storage = m_engine->storage();
    for (int i = 0; i < m_slots.size(); ++i) {
        if (i < storage.size())
            m_slots[i]->setCharacter(storage[i]);
        else
            m_slots[i]->clear();
    }
    m_countLabel->setText(QStringLiteral("%1/%2").arg(storage.size()).arg(STORAGE_CAPACITY));
}

StorageSlotWidget* StorageBar::slotAt(int index) const
{
    if (index < 0 || index >= m_slots.size()) return nullptr;
    return m_slots[index];
}
