#ifndef BATTLELOGWIDGET_H
#define BATTLELOGWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include "core/GameTypes.h"

class GameEngine;

struct LogEntry {
    QString html;
    TeamSide attackerSide;
};

class BattleLogWidget : public QWidget {
    Q_OBJECT
public:
    explicit BattleLogWidget(GameEngine *engine, QWidget *parent = nullptr);

    void appendLog(const QString &msg);
    void appendHtml(const QString &html);
    void clear();

private:
    GameEngine *m_engine;
    QTextEdit *m_logView;
    QPushButton *m_filterAllBtn;
    QPushButton *m_filterPlayerBtn;
    QPushButton *m_filterEnemyBtn;
    QVector<LogEntry> m_logCache;
    int m_filterMode = 0; // 0=all, 1=player only, 2=enemy only

    void applyFilter();
};

#endif // BATTLELOGWIDGET_H
