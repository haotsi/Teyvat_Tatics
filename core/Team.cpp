#include "Team.h"
#include "CharacterBase.h"

Team::Team(TeamSide side) : m_side(side) {}
Team::~Team() = default;

void Team::addPiece(CharacterBase *piece)
{
    if (piece && !m_pieces.contains(piece)) {
        piece->setSide(m_side);
        m_pieces.append(piece);
    }
}

void Team::removePiece(CharacterBase *piece)
{
    m_pieces.removeAll(piece);
}

void Team::clear()
{
    m_pieces.clear();
}

int Team::countElement(ElementType e) const
{
    int count = 0;
    for (auto *p : m_pieces)
        if (p->element() == e) ++count;
    return count;
}

Team::TeamBonuses Team::calculateBonuses() const
{
    TeamBonuses b;
    if (countElement(ElementType::Electro) >= 2) b.voltage = true;
    if (countElement(ElementType::Pyro) >= 2)   b.flames = true;
    if (countElement(ElementType::Hydro) >= 2)   b.water = true;
    if (countElement(ElementType::Dendro) >= 2)  b.greenery = true;
    if (countElement(ElementType::Cryo) >= 2)    b.ice = true;
    return b;
}

void Team::applyBonuses()
{
    auto bonuses = calculateBonuses();
    // Note: Bonuses are checked dynamically in stat calculations
    // This method exists as a hook for future use
    Q_UNUSED(bonuses);
}

QString Team::activeBonusesText() const
{
    auto b = calculateBonuses();
    QStringList list;
    if (b.voltage)  list << QStringLiteral("强能雷:能量恢复+5");
    if (b.flames)   list << QStringLiteral("热诚火:攻击力+25%");
    if (b.water)    list << QStringLiteral("疗愈水:生命值+25%");
    if (b.greenery) list << QStringLiteral("蔓生草:元素精通+100");
    if (b.ice)      list << QStringLiteral("粉碎冰:暴击率+15%");
    return list.isEmpty() ? QStringLiteral("无羁绊") : list.join(QStringLiteral(", "));
}
