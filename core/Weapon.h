#ifndef WEAPON_H
#define WEAPON_H

#include "GameTypes.h"

class Weapon {
public:
    Weapon() = default;
    Weapon(WeaponType type, int stars);

    WeaponType type() const { return m_type; }
    int stars() const { return m_stars; }
    int atk() const { return m_atk; }
    QString name() const;

    void setStars(int s);

    int cost() const;
    int sellPrice() const;

private:
    WeaponType m_type = WeaponType::Sword;
    int m_stars = 2;
    int m_atk = 243;
};

#endif // WEAPON_H
