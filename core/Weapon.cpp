#include "Weapon.h"
#include <QtMath>

Weapon::Weapon(WeaponType type, int stars)
    : m_type(type), m_stars(qBound(2, stars, 5))
{
    m_atk = weaponBaseAtk(m_stars, m_type);
}

QString Weapon::name() const
{
    QString starStr;
    for (int i = 0; i < m_stars; ++i) starStr += QStringLiteral("★");
    return starStr + weaponTypeName(m_type);
}

void Weapon::setStars(int s)
{
    m_stars = qBound(2, s, 5);
    m_atk = weaponBaseAtk(m_stars, m_type);
}

int Weapon::cost() const
{
    return WEAPON_BASE_COST + (m_stars - 2) * 50;
}

int Weapon::sellPrice() const
{
    return qMax(1, static_cast<int>(cost() * SELL_RATIO));
}
