#include "Shop.h"
#include "CharacterBase.h"
#include <QRandomGenerator>
#include <vector>

Shop::Shop(QObject *parent) : QObject(parent) {}

void Shop::refresh(const std::vector<std::unique_ptr<CharacterBase>> &characterPool)
{
    // Keep locked items
    std::vector<ShopItem> lockedItems;
    for (auto &item : m_items) {
        if (item.locked)
            lockedItems.push_back(std::move(item));
    }

    // Clear remaining items (move constructor nullified pointers for locked items)
    m_items.clear();

    // Move locked items back
    for (auto &item : lockedItems)
        m_items.push_back(std::move(item));

    // Fill remaining slots (up to 6 items)
    auto *rng = QRandomGenerator::global();
    while (m_items.size() < 6) {
        int category = rng->bounded(3);
        ShopItem item;
        switch (category) {
            case 0: // Character
                if (!characterPool.empty()) {
                    int idx = rng->bounded(static_cast<int>(characterPool.size()));
                    item.type = ShopItem::Item_Char;
                    item.character = characterPool[idx]->clone().release();
                }
                break;
            case 1: // Weapon
                item.type = ShopItem::Item_Weapon;
                item.weapon = generateRandomWeapon();
                break;
            case 2: // Artifact
                item.type = ShopItem::Item_Artifact;
                item.artifact = generateRandomArtifact();
                break;
        }
        m_items.push_back(std::move(item));
    }

    m_refreshCost += REFRESH_COST_INCREMENT;
    emit shopChanged();
}

void Shop::clear()
{
    m_items.clear();
    emit shopChanged();
}

ShopItem* Shop::itemAt(int index)
{
    if (index < 0 || index >= static_cast<int>(m_items.size())) return nullptr;
    return &m_items[index];
}

bool Shop::purchaseItem(int index, int &primogems, int &mora)
{
    auto *item = itemAt(index);
    if (!item) return false;

    int costPrimo = 0, costMora = 0;
    switch (item->type) {
        case ShopItem::Item_Char:    costPrimo = CHARACTER_COST; break;
        case ShopItem::Item_Weapon:  costMora = item->weapon.cost(); break;
        case ShopItem::Item_Artifact: costMora = ARTIFACT_COST; break;
    }

    if (primogems < costPrimo || mora < costMora) return false;

    primogems -= costPrimo;
    mora -= costMora;
    m_items.erase(m_items.begin() + index);
    emit shopChanged();
    return true;
}

void Shop::removeItem(int index)
{
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_items.erase(m_items.begin() + index);
        emit shopChanged();
    }
}

void Shop::toggleLock(int index)
{
    auto *item = itemAt(index);
    if (item) {
        item->locked = !item->locked;
        emit shopChanged();
    }
}

int Shop::sellPrice(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_items.size())) return 0;
    const auto &item = m_items[index];
    switch (item.type) {
        case ShopItem::Item_Char:    return qMax(1, static_cast<int>(CHARACTER_COST * SELL_RATIO));
        case ShopItem::Item_Weapon:  return item.weapon.sellPrice();
        case ShopItem::Item_Artifact: return item.artifact.sellPrice();
    }
    return 0;
}

Weapon Shop::generateRandomWeapon()
{
    auto *rng = QRandomGenerator::global();
    ::WeaponType wtypes[] = {::WeaponType::Sword, ::WeaponType::Claymore, ::WeaponType::Catalyst,
                           ::WeaponType::Bow, ::WeaponType::Polearm};
    ::WeaponType wtype = wtypes[rng->bounded(5)];

    int roll = rng->bounded(100);
    int stars = 2;
    if (roll < 40)       stars = 2;
    else if (roll < 70)  stars = 3;
    else if (roll < 90)  stars = 4;
    else                 stars = 5;

    return Weapon(wtype, stars);
}

Artifact Shop::generateRandomArtifact()
{
    auto *rng = QRandomGenerator::global();
    ::ArtifactSlot aSlots[] = {::ArtifactSlot::Flower, ::ArtifactSlot::Plume, ::ArtifactSlot::Sands,
                            ::ArtifactSlot::Goblet, ::ArtifactSlot::Circlet};
    ::ArtifactSlot aslot = aSlots[rng->bounded(5)];

    ::ArtifactMainStat stat = ::ArtifactMainStat::NONE;
    switch (aslot) {
        case ::ArtifactSlot::Flower:  stat = ::ArtifactMainStat::HP_NUM; break;
        case ::ArtifactSlot::Plume:   stat = ::ArtifactMainStat::ATK_NUM; break;
        case ::ArtifactSlot::Sands: {
            ::ArtifactMainStat opts[] = {::ArtifactMainStat::ATK_PER, ::ArtifactMainStat::HP_PER,
                                       ::ArtifactMainStat::ENERGY_RC, ::ArtifactMainStat::ELE_MASTERY};
            stat = opts[rng->bounded(4)];
            break;
        }
        case ::ArtifactSlot::Goblet: {
            ::ArtifactMainStat opts[] = {::ArtifactMainStat::ATK_PER, ::ArtifactMainStat::HP_PER,
                                       ::ArtifactMainStat::ELE_MASTERY, ::ArtifactMainStat::DMG_BONUS};
            stat = opts[rng->bounded(4)];
            break;
        }
        case ::ArtifactSlot::Circlet: {
            ::ArtifactMainStat opts[] = {::ArtifactMainStat::ATK_PER, ::ArtifactMainStat::HP_PER,
                                       ::ArtifactMainStat::CRIT_DMG, ::ArtifactMainStat::CRIT_RATE,
                                       ::ArtifactMainStat::ELE_MASTERY};
            stat = opts[rng->bounded(5)];
            break;
        }
    }
    return Artifact(aslot, stat);
}
