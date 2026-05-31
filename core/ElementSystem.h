#ifndef ELEMENTSYSTEM_H
#define ELEMENTSYSTEM_H

#include "GameTypes.h"
#include <functional>

class CharacterBase;

struct ReactionResult {
    ReactionType type = ReactionType::None;
    double damage = 0;
    bool isAmplifying = false; // true = amplifying (scales with atk), false = transformative
    double reactionMultiplier = 1.0;
    double masteryZone = 0;
    bool consumesAuras = true;
    ElementType spreadElement = ElementType::None; // for swirl spread
    bool freezes = false;
    bool knockback = false;
    bool createsSeed = false;
    bool entersQuicken = false;
    bool exitsQuicken = false;
    bool appliesBurning = false;
    bool createsShield = false;
    int splashDamage = 0; // 0=none, 1=adjacent enemies, 2=adjacent all
    double splashMultiplier = 0;
    QString description;
};

class ElementSystem {
public:
    ElementSystem();

    using DamageCallback = std::function<void(CharacterBase*, double, ReactionType)>;
    using SplashCallback = std::function<void(double, ReactionType)>;

    void setDamageCallback(DamageCallback cb) { m_damageCallback = cb; }
    void setSplashCallback(SplashCallback cb) { m_splashCallback = cb; }

    // Try to react when attacker (trigger) attacks defender with given element
    // Returns the reaction result, or None if no reaction
    ReactionResult tryReaction(CharacterBase *attacker, CharacterBase *defender,
                               ElementType attackElement, bool isBurst = false);

    // Check if the defender's auras contain two elements that can react
    static ReactionType checkReactionBetween(ElementType a, ElementType b);

    // Calculate mastery zone
    static double masteryZone(double em, ReactionType type);

    // Calculate transformative reaction damage
    static double transformativeDamage(double em, double baseMultiplier);

    // Process seed explosion (bloom/burgeon/hyperbloom)
    ReactionResult processSeed(CharacterBase *trigger, ElementType triggerElement,
                               CharacterBase *target);

    // Process burning tick damage
    static double burningDamage(double em);

private:
    DamageCallback m_damageCallback;
    SplashCallback m_splashCallback;
};

#endif // ELEMENTSYSTEM_H
