#include "BattleLogWidget.h"
#include "core/GameEngine.h"
#include <QScrollBar>
#include <QTextCursor>

BattleLogWidget::BattleLogWidget(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    // Title + filter row
    auto *titleRow = new QHBoxLayout;
    auto *titleLabel = new QLabel(QStringLiteral("战斗日志"));
    titleLabel->setStyleSheet("color: #ffd700; font-size: 12px; font-weight: bold;");
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    // Filter buttons
    auto makeFilterBtn = [this](const QString &text, int mode) {
        auto *btn = new QPushButton(text);
        btn->setFixedSize(48, 18);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { background: #2a2a3a; color: #888; border: 1px solid #444; "
            "border-radius: 2px; font-size: 9px; }"
            "QPushButton:checked { background: #3a3a5c; color: #ffd700; border-color: #ffd700; }"
        );
        connect(btn, &QPushButton::clicked, this, [this, mode]() {
            m_filterMode = mode;
            // Update button states
            m_filterAllBtn->setChecked(mode == 0);
            m_filterPlayerBtn->setChecked(mode == 1);
            m_filterEnemyBtn->setChecked(mode == 2);
            applyFilter();
        });
        return btn;
    };

    m_filterAllBtn = makeFilterBtn(QStringLiteral("全部"), 0);
    m_filterAllBtn->setChecked(true);
    m_filterPlayerBtn = makeFilterBtn(QStringLiteral("我方"), 1);
    m_filterEnemyBtn = makeFilterBtn(QStringLiteral("敌方"), 2);

    titleRow->addWidget(m_filterAllBtn);
    titleRow->addWidget(m_filterPlayerBtn);
    titleRow->addWidget(m_filterEnemyBtn);
    layout->addLayout(titleRow);

    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet(
        "QTextEdit { background: #0d0d1a; color: #ccc; border: 1px solid #333; "
        "border-radius: 4px; font-size: 11px; font-family: monospace; }"
    );
    m_logView->setMaximumHeight(150);
    layout->addWidget(m_logView);

    setStyleSheet("background: #15152a; border-radius: 6px;");
    setMaximumHeight(220);

    // System messages go directly to log view
    connect(m_engine, &GameEngine::messageLogged, this, &BattleLogWidget::appendLog);
    // Battle actions get cached with side info
    connect(m_engine, &GameEngine::battleActionOccurred, this, [this](const BattleAction &action) {
        QString msg;
        if (action.damage > 0) {
            // Side prefix with color
            bool isPlayer = (action.attackerSide == TeamSide::Player);
            QString sidePrefix = isPlayer
                ? QStringLiteral("<span style='color:#50C83C;font-weight:bold'>[我方]</span> ")
                : QStringLiteral("<span style='color:#FF5028;font-weight:bold'>[敌方]</span> ");

            msg += sidePrefix;

            QString atkColor = elementColorHex(action.attackElement);
            // Skill name with attack element color
            msg += QStringLiteral("<span style='color:%1'>%2</span>")
                       .arg(atkColor, action.skillName);
            // Target
            msg += QStringLiteral(" 对 #%1").arg(action.targetId + 1);
            // Damage value in white bold
            msg += QStringLiteral(" 造成 <span style='color:white;font-weight:bold'>%1</span> 伤害")
                       .arg(action.damage, 0, 'f', 0);
            // Element type label
            if (action.attackElement != ElementType::None) {
                QString elemCol = elementColorHex(action.attackElement);
                msg += QStringLiteral(" <span style='color:%1'>[%2]</span>")
                           .arg(elemCol, elementName(action.attackElement));
            }
            // Reaction label
            if (action.reaction != ReactionType::None) {
                ElementType rxnElem = reactionElementColor(action.reaction);
                QString rxnColor = elementColorHex(rxnElem);
                msg += QStringLiteral(" <span style='color:%1;font-weight:bold'>[%2]</span>")
                           .arg(rxnColor, reactionName(action.reaction));
            }
            // Crit
            if (action.crit)
                msg += QStringLiteral(" <span style='color:yellow;font-weight:bold'>暴击!</span>");
        }
        if (!msg.isEmpty()) {
            // Cache the log entry
            LogEntry entry;
            entry.html = msg;
            entry.attackerSide = action.attackerSide;
            m_logCache.append(entry);
            // Show if passes filter
            if (m_filterMode == 0
                || (m_filterMode == 1 && entry.attackerSide == TeamSide::Player)
                || (m_filterMode == 2 && entry.attackerSide == TeamSide::Enemy)) {
                appendHtml(msg);
            }
        }
    });
}

void BattleLogWidget::appendLog(const QString &msg)
{
    m_logView->append(msg);
    auto sb = m_logView->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void BattleLogWidget::appendHtml(const QString &html)
{
    m_logView->moveCursor(QTextCursor::End);
    m_logView->insertHtml(html + QStringLiteral("<br>"));
    auto sb = m_logView->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void BattleLogWidget::clear()
{
    m_logView->clear();
    m_logCache.clear();
}

void BattleLogWidget::applyFilter()
{
    m_logView->clear();
    for (const auto &entry : m_logCache) {
        if (m_filterMode == 0
            || (m_filterMode == 1 && entry.attackerSide == TeamSide::Player)
            || (m_filterMode == 2 && entry.attackerSide == TeamSide::Enemy)) {
            m_logView->moveCursor(QTextCursor::End);
            m_logView->insertHtml(entry.html + QStringLiteral("<br>"));
        }
    }
    auto sb = m_logView->verticalScrollBar();
    sb->setValue(sb->maximum());
}
