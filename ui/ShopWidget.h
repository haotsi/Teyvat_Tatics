#ifndef SHOPWIDGET_H
#define SHOPWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QEvent>

class GameEngine;
class Shop;

class ShopWidget : public QWidget {
    Q_OBJECT
public:
    explicit ShopWidget(GameEngine *engine, QWidget *parent = nullptr);

    void refresh();

signals:
    void itemPurchased(int index);
    void itemClicked(int index);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    GameEngine *m_engine;
    QVBoxLayout *m_mainLayout;
    QWidget *m_itemsContainer;
    QVBoxLayout *m_itemsLayout;
    QLabel *m_titleLabel;
    QLabel *m_refreshCostLabel;
    QPushButton *m_refreshButton;
    QPushButton *m_lockButton;

    void buildItemWidgets();
    QWidget* createItemCard(int index);
};

#endif // SHOPWIDGET_H
