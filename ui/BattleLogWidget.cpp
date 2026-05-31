#include "BattleLogWidget.h"
#include "core/GameEngine.h"
#include <QScrollBar>
#include <QTextCursor>

BattleLogWidget::BattleLogWidget(GameEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    auto *titleLabel = new QLabel(QStringLiteral("战斗日志"));
    titleLabel->setStyleSheet("color: #ffd700; font-size: 12px; font-weight: bold;");
    layout->addWidget(titleLabel);

    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet(
        "QTextEdit { background: #0d0d1a; color: #ccc; border: 1px solid #333; "
        "border-radius: 4px; font-size: 11px; font-family: monospace; }"
    );
    m_logView->setMaximumHeight(150);
    layout->addWidget(m_logView);

    setStyleSheet("background: #15152a; border-radius: 6px;");
    setMaximumHeight(190);

    connect(m_engine, &GameEngine::messageLogged, this, &BattleLogWidget::appendLog);
    connect(m_engine, &GameEngine::battleActionOccurred, this, [this](const BattleAction &action) {
        QString msg;
        if (action.damage > 0) {
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
        if (!msg.isEmpty())
            appendHtml(msg);
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
}
