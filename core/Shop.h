#ifndef SHOP_H
#define SHOP_H

#include "GameTypes.h"
#include "Weapon.h"
#include "Artifact.h"
#include "CharacterBase.h"
#include <QObject>
#include <QVector>
#include <memory>
#include <vector>

struct ShopItem {
    enum Type { Item_Char, Item_Weapon, Item_Artifact } type;
    CharacterBase* character = nullptr;
    Weapon weapon;
    Artifact artifact;
    bool locked = false;

    ShopItem() = default;
    ShopItem(const ShopItem&) = delete;
    ShopItem(ShopItem&& other) noexcept
        : type(other.type), character(other.character), weapon(other.weapon)
        , artifact(other.artifact), locked(other.locked)
    {
        other.character = nullptr;
    }
    ShopItem& operator=(const ShopItem&) = delete;
    ShopItem& operator=(ShopItem&& other) noexcept {
        if (this != &other) {
            delete character;
            type = other.type;
            character = other.character;
            weapon = other.weapon;
            artifact = other.artifact;
            locked = other.locked;
            other.character = nullptr;
        }
        return *this;
    }
    ~ShopItem() { delete character; }
};

class Shop : public QObject {
    Q_OBJECT
public:
    explicit Shop(QObject *parent = nullptr);

    const std::vector<ShopItem> &items() const { return m_items; }
    int itemCount() const { return static_cast<int>(m_items.size()); }

    void refresh(const std::vector<std::unique_ptr<CharacterBase>> &characterPool);
    int refreshCost() const { return m_refreshCost; }
    void resetRefreshCost() { m_refreshCost = REFRESH_COST_BASE; }

    ShopItem* itemAt(int index);
    bool purchaseItem(int index, int &primogems, int &mora);
    void removeItem(int index);
    void toggleLock(int index);
    int sellPrice(int index) const;

    void clear();

signals:
    void shopChanged();

private:
    std::vector<ShopItem> m_items;
    int m_refreshCost = REFRESH_COST_BASE;

    Weapon generateRandomWeapon();
    Artifact generateRandomArtifact();
};

#endif // SHOP_H
