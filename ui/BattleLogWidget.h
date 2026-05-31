#ifndef BATTLELOGWIDGET_H
#define BATTLELOGWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>

class GameEngine;

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
};

#endif // BATTLELOGWIDGET_H
