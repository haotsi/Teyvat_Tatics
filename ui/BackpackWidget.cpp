#include "BackpackWidget.h"
#include "core/GameEngine.h"
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QDrag>
#include <QMimeData>
#include "ui/DragDropMimeData.h"

// --- BackpackSlotWidget ---

BackpackSlotWidget::BackpackSlotWidget(int index, QWidget *parent)
    : QFrame(parent), m_index(index)
{
    setFixedSize(64, 64);
    setFrameStyle(QFrame::Box);
    setStyleSheet(
        "BackpackSlotWidget { background: #1e1e35; border: 1px solid #444; border-radius: 4px; }"
    );
    setCursor(QCursor(Qt::PointingHandCursor));
}

void BackpackSlotWidget::setWeapon(const Weapon &w, int sourceIndex)
{
    m_type = WeaponSlot;
    m_weapon = w;
    m_sourceIndex = sourceIndex;
    m_artifact = Artifact();
    update();
}

void BackpackSlotWidget::setArtifact(const Artifact &a, int sourceIndex)
{
    m_type = ArtifactSlot;
    m_artifact = a;
    m_sourceIndex = sourceIndex;
    m_weapon = Weapon();
    update();
}

void BackpackSlotWidget::clear()
{
    m_type = Empty;
    update();
}

void BackpackSlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !isEmpty()) {
        m_dragStartPos = event->pos();
    }
    QFrame::mousePressEvent(event);
}

void BackpackSlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isEmpty()) return;
    if ((event->pos() - m_dragStartPos).manhattanLength() < 10) return;

    auto *mime = new DragDropMimeData();
    if (m_type == WeaponSlot) {
        mime->setSourceType(QStringLiteral("backpack_weapon"));
        mime->setBackpackIndex(m_sourceIndex);
    } else if (m_type == ArtifactSlot) {
        mime->setSourceType(QStringLiteral("backpack_artifact"));
        mime->setBackpackIndex(m_sourceIndex);
    }

    auto *drag = new QDrag(this);
    drag->setMimeData(mime);

    QPixmap pixmap(40, 40);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor(255, 255, 255, 180));
    if (m_type == WeaponSlot)
        p.setPen(QColor(255, 200, 100));
    else
        p.setPen(QColor(120, 200, 120));
    p.drawEllipse(2, 2, 36, 36);
    QString label = (m_type == WeaponSlot) ? weaponTypeName(m_weapon.type()).left(2) : artifactSlotName(m_artifact.slot());
    p.setPen(Qt::black);
    p.drawText(QRect(2, 10, 36, 20), Qt::AlignCenter, label);
    p.end();
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::MoveAction);

    emit dragStarted(m_index);
}

void BackpackSlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_hovered) {
        painter.fillRect(rect(), QColor(255, 255, 255, 15));
    }

    if (isEmpty()) {
        painter.setPen(QColor(80, 80, 80));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("空"));
        return;
    }

    QFont font;
    font.setPixelSize(9);
    font.setBold(true);
    painter.setFont(font);

    if (m_type == WeaponSlot) {
        QString starStr;
        switch (m_weapon.stars()) {
            case 5: starStr = QStringLiteral("★★★★★"); break;
            case 4: starStr = QStringLiteral("★★★★"); break;
            case 3: starStr = QStringLiteral("★★★"); break;
            case 2: starStr = QStringLiteral("★★"); break;
            default: starStr = QStringLiteral("★"); break;
        }

        // Star-based color
        QColor starColor;
        switch (m_weapon.stars()) {
            case 5: starColor = QColor(255, 215, 0); break;
            case 4: starColor = QColor(163, 53, 238); break;
            case 3: starColor = QColor(0, 112, 221); break;
            default: starColor = QColor(30, 255, 0); break;
        }

        painter.setPen(starColor);
        painter.drawText(QRect(2, 4, 60, 16), Qt::AlignCenter, starStr);

        painter.setPen(Qt::white);
        painter.drawText(QRect(2, 22, 60, 14), Qt::AlignCenter, weaponTypeName(m_weapon.type()));

        font.setPixelSize(9);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(200, 200, 200));
        painter.drawText(QRect(2, 38, 60, 14), Qt::AlignCenter,
                         QStringLiteral("ATK:%1").arg(m_weapon.atk()));
    } else if (m_type == ArtifactSlot) {
        painter.setPen(QColor(120, 200, 120));
        painter.drawText(QRect(2, 4, 60, 16), Qt::AlignCenter, artifactSlotName(m_artifact.slot()));

        painter.setPen(Qt::white);
        QFont smallFont;
        smallFont.setPixelSize(7);
        painter.setFont(smallFont);
        painter.drawText(QRect(2, 22, 60, 36), Qt::AlignCenter | Qt::TextWordWrap,
                         statName(m_artifact.mainStat()));
    }
}

void BackpackSlotWidget::enterEvent(QEnterEvent *)
{
    m_hovered = true;
    QString tooltip;
    if (m_type == WeaponSlot) {
        tooltip = m_weapon.name() + QStringLiteral("\nATK: %1\n售价: %2 摩拉")
                      .arg(m_weapon.atk()).arg(m_weapon.sellPrice());
    } else if (m_type == ArtifactSlot) {
        tooltip = m_artifact.name() + QStringLiteral("\n售价: %1 摩拉")
                      .arg(m_artifact.sellPrice());
    }
    setToolTip(tooltip);
    update();
}

void BackpackSlotWidget::leaveEvent(QEvent *)
{
    m_hovered = false;
    update();
}

void BackpackSlotWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (isEmpty()) return;
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background: #1a1a2e; color: white; border: 1px solid #555; }"
        "QMenu::item:selected { background: #3a3a5c; }"
    );
    QAction *sellAction = menu.addAction(QStringLiteral("出售"));
    if (menu.exec(event->globalPos()) == sellAction) {
        emit sellRequested(m_index);
    }
}

// --- BackpackWidget ---

BackpackWidget::BackpackWidget(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    buildUI();
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
}

void BackpackWidget::buildUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(4);

    m_titleLabel = new QLabel(QStringLiteral("背包 (武器/圣遗物)"));
    m_titleLabel->setStyleSheet("color: #ffd700; font-size: 12px; font-weight: bold;");
    m_mainLayout->addWidget(m_titleLabel);

    // Grid of slots: 5x2
    for (int row = 0; row < 2; ++row) {
        auto *rowLayout = new QHBoxLayout;
        rowLayout->setSpacing(3);
        for (int col = 0; col < 5; ++col) {
            int idx = row * 5 + col;
            auto *slot = new BackpackSlotWidget(idx);
            m_slots.append(slot);
            rowLayout->addWidget(slot);

            connect(slot, &BackpackSlotWidget::clicked, this, [this](int displayIdx) {
                auto *s = m_slots[displayIdx];
                if (s->slotType() == BackpackSlotWidget::WeaponSlot)
                    emit weaponClicked(s->weapon(), s->sourceIndex());
                else if (s->slotType() == BackpackSlotWidget::ArtifactSlot)
                    emit artifactClicked(s->artifact(), s->sourceIndex());
            });

            connect(slot, &BackpackSlotWidget::sellRequested, this, [this](int sourceIdx) {
                auto *s = m_slots[sourceIdx];
                if (s->slotType() == BackpackSlotWidget::WeaponSlot)
                    m_engine->sellWeaponFromBackpack(s->sourceIndex());
                else if (s->slotType() == BackpackSlotWidget::ArtifactSlot)
                    m_engine->sellArtifactFromBackpack(s->sourceIndex());
                refresh();
            });
        }
        m_mainLayout->addLayout(rowLayout);
    }

    setStyleSheet("background: #15152a; border: 1px solid #555; border-radius: 6px;");
    setFixedSize(370, 200);
}

void BackpackWidget::refresh()
{
    auto &weapons = m_engine->weaponBackpack();
    auto &artifacts = m_engine->artifactBackpack();

    // First fill with weapons, then artifacts
    int slotIdx = 0;
    for (int i = 0; i < weapons.size() && slotIdx < BACKPACK_CAPACITY; ++i, ++slotIdx)
        m_slots[slotIdx]->setWeapon(weapons[i], i);
    for (int i = 0; i < artifacts.size() && slotIdx < BACKPACK_CAPACITY; ++i, ++slotIdx)
        m_slots[slotIdx]->setArtifact(artifacts[i], i);
    // Clear remaining
    for (; slotIdx < BACKPACK_CAPACITY; ++slotIdx)
        m_slots[slotIdx]->clear();
}
