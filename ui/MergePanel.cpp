#include "MergePanel.h"
#include "core/GameEngine.h"
#include "core/CharacterBase.h"
#include "ui/DragDropMimeData.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

// --- MergeSlotWidget ---

MergeSlotWidget::MergeSlotWidget(bool acceptDrops, QWidget *parent)
    : QFrame(parent), m_acceptDrops(acceptDrops)
{
    setFixedSize(90, 100);
    setFrameStyle(QFrame::Box);
    setStyleSheet(
        "MergeSlotWidget { background: #1e1e35; border: 1px dashed #555; border-radius: 4px; }"
    );
    if (m_acceptDrops)
        setAcceptDrops(true);
}

void MergeSlotWidget::setCharacter(CharacterBase *piece)
{
    m_character = piece;
    update();
}

void MergeSlotWidget::clear()
{
    m_character = nullptr;
    m_highlighted = false;
    update();
}

CharacterBase* MergeSlotWidget::takeCharacter()
{
    auto *c = m_character;
    m_character = nullptr;
    update();
    return c;
}

void MergeSlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_highlighted) {
        painter.setPen(QPen(QColor(255, 215, 0), 2));
        painter.drawRoundedRect(QRectF(1, 1, width()-2, height()-2), 5, 5);
    }

    if (!m_character) {
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(rect(), Qt::AlignCenter,
                         m_acceptDrops ? QStringLiteral("[拖放角色]") : QStringLiteral("[结果]"));
        return;
    }

    // Draw character portrait
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

    // Circle
    QPointF center(width()/2.0, 32);
    QRadialGradient grad(center, 20);
    grad.setColorAt(0, elemCol.lighter(150));
    grad.setColorAt(1, elemCol.darker(120));
    painter.setBrush(grad);
    painter.setPen(QPen(elemCol.darker(150), 1.5));
    painter.drawEllipse(center, 20, 20);

    // Name
    QFont font;
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);
    QColor consColor(constellationColor(m_character->constellation()));
    painter.setPen(consColor);
    painter.drawText(QRect(2, 54, width()-4, 16), Qt::AlignCenter, m_character->name().left(4));

    // Constellation
    font.setPixelSize(9);
    painter.setFont(font);
    painter.setPen(QColor(200, 200, 200));
    painter.drawText(QRect(2, 72, width()-4, 14), Qt::AlignCenter,
                     QStringLiteral("命之座: %1").arg(m_character->constellation()));
}

void MergeSlotWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_acceptDrops && DragDropMimeData::isPieceDrag(event->mimeData()))
        event->acceptProposedAction();
}

void MergeSlotWidget::dropEvent(QDropEvent *event)
{
    if (!m_acceptDrops) return;
    auto *mime = DragDropMimeData::fromMimeData(event->mimeData());
    if (!mime) return;

    // The actual character retrieval is handled externally via signals
    // For now, accept the drop and let MergePanel handle it
    event->acceptProposedAction();
    // MergePanel will process through external mechanisms
}

void MergeSlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && m_character) {
        // Right-click to remove
        m_character = nullptr;
        update();
        return;
    }
    QFrame::mousePressEvent(event);
}

// --- MergePanel ---

MergePanel::MergePanel(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    auto *titleLabel = new QLabel(QStringLiteral("命之座合成"));
    titleLabel->setStyleSheet("color: #ffd700; font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    auto *slotsLayout = new QHBoxLayout;
    slotsLayout->setSpacing(8);

    // Slot A
    auto *slotALayout = new QVBoxLayout;
    auto *labelA = new QLabel(QStringLiteral("角色A"));
    labelA->setStyleSheet("color: #aaa; font-size: 10px;");
    labelA->setAlignment(Qt::AlignCenter);
    slotALayout->addWidget(labelA);
    m_slotA = new MergeSlotWidget(true);
    slotALayout->addWidget(m_slotA);
    slotsLayout->addLayout(slotALayout);

    // "+" label
    auto *plusLabel = new QLabel(QStringLiteral("+"));
    plusLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    plusLabel->setAlignment(Qt::AlignCenter);
    slotsLayout->addWidget(plusLabel);

    // Slot B
    auto *slotBLayout = new QVBoxLayout;
    auto *labelB = new QLabel(QStringLiteral("角色B"));
    labelB->setStyleSheet("color: #aaa; font-size: 10px;");
    labelB->setAlignment(Qt::AlignCenter);
    slotBLayout->addWidget(labelB);
    m_slotB = new MergeSlotWidget(true);
    slotBLayout->addWidget(m_slotB);
    slotsLayout->addLayout(slotBLayout);

    // "=" label
    auto *eqLabel = new QLabel(QStringLiteral("="));
    eqLabel->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");
    eqLabel->setAlignment(Qt::AlignCenter);
    slotsLayout->addWidget(eqLabel);

    // Slot Result
    auto *slotRLayout = new QVBoxLayout;
    auto *labelR = new QLabel(QStringLiteral("结果"));
    labelR->setStyleSheet("color: #aaa; font-size: 10px;");
    labelR->setAlignment(Qt::AlignCenter);
    slotRLayout->addWidget(labelR);
    m_slotResult = new MergeSlotWidget(false);
    slotRLayout->addWidget(m_slotResult);
    slotsLayout->addLayout(slotRLayout);

    mainLayout->addLayout(slotsLayout);

    // Merge button
    m_mergeBtn = new QPushButton(QStringLiteral("合成升星"));
    m_mergeBtn->setEnabled(false);
    m_mergeBtn->setStyleSheet(
        "QPushButton { background: #8b0000; color: white; font-size: 14px; font-weight: bold; "
        "border: 2px solid #ff4444; padding: 8px; border-radius: 4px; }"
        "QPushButton:hover { background: #a00000; }"
        "QPushButton:disabled { background: #444; border-color: #666; color: #888; }"
    );
    connect(m_mergeBtn, &QPushButton::clicked, this, &MergePanel::performMerge);
    mainLayout->addWidget(m_mergeBtn);

    m_statusLabel = new QLabel;
    m_statusLabel->setStyleSheet("color: #aaa; font-size: 11px;");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    setStyleSheet("background: #1a1a2e; border: 1px solid #555; border-radius: 6px;");
    setFixedSize(460, 260);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
}

void MergePanel::refresh()
{
    m_slotA->clear();
    m_slotB->clear();
    m_slotResult->clear();
    m_mergeBtn->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("将两个同名角色拖入上方格子"));
    updateMergeState();
}

void MergePanel::updateMergeState()
{
    auto *a = m_slotA->character();
    auto *b = m_slotB->character();

    if (a && b && a->name() == b->name()) {
        // Same character - highlight merge
        int newConst = qMin(a->constellation() + b->constellation() + 1, 6);
        m_mergeBtn->setEnabled(true);
        m_mergeBtn->setStyleSheet(
            "QPushButton { background: #006400; color: #ffd700; font-size: 14px; font-weight: bold; "
            "border: 2px solid #ffd700; padding: 8px; border-radius: 4px; }"
            "QPushButton:hover { background: #008000; }"
            "QPushButton:disabled { background: #444; border-color: #666; color: #888; }"
        );
        m_slotResult->setHighlighted(true);
        m_statusLabel->setText(QStringLiteral("可合成！命之座: %1 → %2")
                                   .arg(a->constellation()).arg(newConst));
    } else {
        m_mergeBtn->setEnabled(false);
        m_mergeBtn->setStyleSheet(
            "QPushButton { background: #444; font-size: 14px; font-weight: bold; "
            "border: 2px solid #666; padding: 8px; border-radius: 4px; color: #888; }"
            "QPushButton:hover { background: #444; }"
        );
        m_slotResult->setHighlighted(false);
        if (a && b && a->name() != b->name())
            m_statusLabel->setText(QStringLiteral("角色名称不同，无法合成"));
        else
            m_statusLabel->setText(QStringLiteral("将两个同名角色拖入上方格子"));
    }
}

void MergePanel::performMerge()
{
    auto *a = m_slotA->takeCharacter();
    auto *b = m_slotB->takeCharacter();
    if (!a || !b) return;

    auto *result = m_engine->mergeCharactersDirect(a, b);
    if (!result) {
        // Put back characters
        m_slotA->setCharacter(a);
        m_slotB->setCharacter(b);
        return;
    }

    m_slotResult->setCharacter(result);
    m_mergeBtn->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("合成完成！结果可在结果槽中查看"));
    emit mergeCompleted(result);
}
