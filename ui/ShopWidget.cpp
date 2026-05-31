#include "ShopWidget.h"
#include "core/GameEngine.h"
#include "core/Shop.h"
#include "core/CharacterBase.h"
#include <QScrollArea>
#include <QGroupBox>
#include <QMessageBox>

ShopWidget::ShopWidget(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(4, 4, 4, 4);

    // Title bar
    auto *titleBar = new QHBoxLayout;
    m_titleLabel = new QLabel(QStringLiteral("商店"));
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ffd700;");
    titleBar->addWidget(m_titleLabel);
    titleBar->addStretch();

    m_refreshCostLabel = new QLabel;
    m_refreshCostLabel->setStyleSheet("color: #aaa; font-size: 12px;");
    titleBar->addWidget(m_refreshCostLabel);

    m_refreshButton = new QPushButton(QStringLiteral("刷新"));
    m_refreshButton->setStyleSheet(
        "QPushButton { background: #3a3a5c; color: white; border: 1px solid #555; "
        "padding: 4px 12px; border-radius: 3px; }"
        "QPushButton:hover { background: #4a4a7c; }"
    );
    connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
        if (m_engine->manualRefreshShop())
            refresh();
    });
    titleBar->addWidget(m_refreshButton);

    m_mainLayout->addLayout(titleBar);

    // Items container in scroll area
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_itemsContainer = new QWidget;
    m_itemsLayout = new QVBoxLayout(m_itemsContainer);
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(2);
    scrollArea->setWidget(m_itemsContainer);

    m_mainLayout->addWidget(scrollArea);

    setStyleSheet("background: #1a1a2e; border-radius: 6px;");
    setMinimumWidth(200);
    setMaximumHeight(400);

    connect(m_engine->shop(), &Shop::shopChanged, this, &ShopWidget::refresh);
}

void ShopWidget::refresh()
{
    m_refreshCostLabel->setText(QStringLiteral("刷新费: %1摩拉").arg(m_engine->shop()->refreshCost()));
    buildItemWidgets();
}

void ShopWidget::buildItemWidgets()
{
    // Clear existing
    QLayoutItem *child;
    while ((child = m_itemsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    auto *shop = m_engine->shop();
    for (int i = 0; i < shop->itemCount(); ++i) {
        QWidget *card = createItemCard(i);
        m_itemsLayout->addWidget(card);
    }
    m_itemsLayout->addStretch();
}

QWidget* ShopWidget::createItemCard(int index)
{
    auto *shop = m_engine->shop();
    auto *item = shop->itemAt(index);
    if (!item) return nullptr;

    auto *card = new QWidget;
    card->setStyleSheet(
        "QWidget { background: #252540; border-radius: 4px; padding: 3px; }"
        "QWidget:hover { background: #303055; }"
    );
    card->setProperty("shopIndex", index);
    card->installEventFilter(this);

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(6);

    QString icon, name, costText, nameColor = QStringLiteral("white");
    switch (item->type) {
        case ShopItem::Item_Char: {
            icon = QStringLiteral("[角]");
            name = item->character ? item->character->name() : QStringLiteral("???");
            costText = QStringLiteral("原石%1").arg(CHARACTER_COST);
            if (item->character)
                nameColor = constellationColor(item->character->constellation());
            break;
        }
        case ShopItem::Item_Weapon: {
            icon = QStringLiteral("[武]");
            name = item->weapon.name();
            costText = QStringLiteral("摩拉%1").arg(item->weapon.cost());
            switch (item->weapon.stars()) {
                case 5: nameColor = QStringLiteral("#FFD700"); break;
                case 4: nameColor = QStringLiteral("#A335EE"); break;
                case 3: nameColor = QStringLiteral("#0070DD"); break;
                case 2: nameColor = QStringLiteral("#1EFF00"); break;
            }
            break;
        }
        case ShopItem::Item_Artifact:
            icon = QStringLiteral("[圣]");
            name = item->artifact.name();
            costText = QStringLiteral("摩拉%1").arg(ARTIFACT_COST);
            break;
    }

    auto *iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet("font-size: 18px;");
    layout->addWidget(iconLabel);

    auto *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 12px;").arg(nameColor));
    layout->addWidget(nameLabel, 1);

    auto *costLabel = new QLabel(costText);
    costLabel->setStyleSheet("color: #ffd700; font-size: 11px;");
    layout->addWidget(costLabel);

    auto *buyBtn = new QPushButton(QStringLiteral("购买"));
    buyBtn->setFixedSize(48, 24);
    buyBtn->setStyleSheet(
        "QPushButton { background: #2a7a2a; color: white; border: none; "
        "border-radius: 2px; font-size: 11px; }"
        "QPushButton:hover { background: #3a9a3a; }"
    );
    connect(buyBtn, &QPushButton::clicked, this, [this, index]() {
        if (m_engine->buyShopItem(index))
            emit itemPurchased(index);
    });
    layout->addWidget(buyBtn);

    if (item->locked) {
        card->setStyleSheet(card->styleSheet() + " border: 1px solid #ffd700;");
    }

    return card;
}

bool ShopWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *w = qobject_cast<QWidget*>(obj);
        if (w && w->property("shopIndex").isValid()) {
            int idx = w->property("shopIndex").toInt();
            emit itemClicked(idx);
            return false; // let the event propagate for buy button
        }
    }
    return QWidget::eventFilter(obj, event);
}
