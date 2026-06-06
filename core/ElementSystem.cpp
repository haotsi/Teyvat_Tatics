#include "ElementSystem.h"
#include "CharacterBase.h"
#include <QtMath>

ElementSystem::ElementSystem() = default;

double ElementSystem::masteryZone(double em, ReactionType type)
{
    if (type == ReactionType::Vaporize || type == ReactionType::Melt)
        return (2.8 * em) / (em + 1400.0);
    if (type == ReactionType::Spread || type == ReactionType::Aggravate)
        return (5.0 * em) / (em + 1200.0);
    // All other transformative reactions
    return (6.0 * em) / (em + 1400.0);
}

double ElementSystem::transformativeDamage(double em, double baseMultiplier)
{
    return REACTION_CONSTANT * baseMultiplier * (1.0 + (6.0 * em) / (em + 1400.0));
}

double ElementSystem::burningDamage(double em)
{
    return REACTION_CONSTANT * BURNING_MULT * (1.0 + (6.0 * em) / (em + 1400.0));
}

ReactionType ElementSystem::checkReactionBetween(ElementType a, ElementType b)
{
    // Ensure consistent ordering: lower enum value first
    // Enum order: Hydro=0, Pyro=1, Electro=2, Cryo=3, Dendro=4, Anemo=5, Geo=6, None=7
    if (static_cast<int>(a) > static_cast<int>(b))
        std::swap(a, b);

    // All conditions use SORTED order (a = lower enum value, b = higher)
    // Vaporize: Hydro(0) + Pyro(1)
    if (a == ElementType::Hydro && b == ElementType::Pyro) return ReactionType::Vaporize;
    // Melt: Pyro(1) + Cryo(3)
    if (a == ElementType::Pyro && b == ElementType::Cryo) return ReactionType::Melt;
    // Overload: Pyro(1) + Electro(2)
    if (a == ElementType::Pyro && b == ElementType::Electro) return ReactionType::Overload;
    // ElectroCharged: Hydro(0) + Electro(2)
    if (a == ElementType::Hydro && b == ElementType::Electro) return ReactionType::ElectroCharged;
    // Superconduct: Electro(2) + Cryo(3)
    if (a == ElementType::Electro && b == ElementType::Cryo) return ReactionType::Superconduct;
    // Frozen: Hydro(0) + Cryo(3)
    if (a == ElementType::Hydro && b == ElementType::Cryo) return ReactionType::Frozen;
    // Bloom: Hydro(0) + Dendro(4)
    if (a == ElementType::Hydro && b == ElementType::Dendro) return ReactionType::Bloom;
    // Burning: Pyro(1) + Dendro(4)
    if (a == ElementType::Pyro && b == ElementType::Dendro) return ReactionType::Burning;
    // Quicken: Electro(2) + Dendro(4)
    if (a == ElementType::Electro && b == ElementType::Dendro) return ReactionType::Quicken;

    return ReactionType::None;
}

ReactionResult ElementSystem::tryReaction(CharacterBase *attacker, CharacterBase *defender,
                                           ElementType attackElement, bool isBurst)
{
    ReactionResult result;
    if (attackElement == ElementType::None) return result;

    // Apply attack element as aura (Anemo and Geo don't apply auras)
    if (attackElement != ElementType::Anemo && attackElement != ElementType::Geo)
        defender->applyAura(attackElement, 1);

    auto &auras = defender->auras();

    // Check for Anemo swirl with existing aura (before any pair-check block)
    if (attackElement == ElementType::Anemo && !auras.isEmpty()) {
        ElementType existing = auras.last().element;
        if (existing == ElementType::Hydro || existing == ElementType::Pyro ||
            existing == ElementType::Electro || existing == ElementType::Cryo) {
            result.type = ReactionType::Swirl;
            result.damage = REACTION_CONSTANT * SWIRL_MULT *
                            (1.0 + (6.0 * attacker->eleMastery()) / (attacker->eleMastery() + 1400.0));
            result.consumesAuras = false;
            result.spreadElement = existing;
            result.splashDamage = 1;
            result.splashMultiplier = SWIRL_SPREAD_MULT;
            result.description = reactionName(ReactionType::Swirl);
            return result;
        }
    }

    // Check for Geo crystallize with existing aura (independent of aura count)
    if (attackElement == ElementType::Geo && !auras.isEmpty()) {
        ElementType existing = auras.last().element;
        if (existing == ElementType::Hydro || existing == ElementType::Pyro ||
            existing == ElementType::Electro || existing == ElementType::Cryo) {
            result.type = ReactionType::Crystallize;
            result.createsShield = true;
            result.consumesAuras = true;
            auras.clear();
            result.description = reactionName(ReactionType::Crystallize);
            return result;
        }
        return result; // Geo attack but no crystallizable aura
    }

    // Need at least 2 auras for pair reactions
    if (auras.size() < 2)
        return result;

    // Check all pairs of auras for reactions
    for (int i = 0; i < auras.size(); ++i) {
        for (int j = i + 1; j < auras.size(); ++j) {
            ReactionType rt = checkReactionBetween(auras[i].element, auras[j].element);
            if (rt == ReactionType::None) continue;

            // Found a reaction
            result.type = rt;

            // Determine the trigger character (the one whose attack caused this)
            CharacterBase *trigger = attacker;

            switch (rt) {
                case ReactionType::Vaporize:
                case ReactionType::Melt: {
                    if (isBurst) {
                        // Burst cannot trigger amplifying reactions
                        result.type = ReactionType::None;
                        continue;
                    }
                    result.isAmplifying = true;
                    result.reactionMultiplier = (rt == ReactionType::Vaporize) ? VAPORIZE_MULT : MELT_MULT;
                    result.masteryZone = masteryZone(trigger->eleMastery(), rt);
                    result.description = reactionName(rt);
                    break;
                }
                case ReactionType::Overload: {
                    result.damage = transformativeDamage(trigger->eleMastery(), OVERLOAD_MULT);
                    result.knockback = true;
                    result.splashDamage = 0;
                    result.description = reactionName(ReactionType::Overload);
                    break;
                }
                case ReactionType::ElectroCharged: {
                    result.damage = transformativeDamage(trigger->eleMastery(), ELECTRO_CHARGED_MULT);
                    result.splashDamage = 1;
                    result.splashMultiplier = ELECTRO_CHARGED_SPLASH_MULT;
                    result.description = reactionName(ReactionType::ElectroCharged);
                    break;
                }
                case ReactionType::Superconduct: {
                    result.damage = transformativeDamage(trigger->eleMastery(), SUPERCONDUCT_MULT);
                    result.splashDamage = 1;
                    result.splashMultiplier = SUPERCONDUCT_SPLASH_MULT;
                    result.description = reactionName(ReactionType::Superconduct);
                    break;
                }
                case ReactionType::Frozen: {
                    result.freezes = true;
                    result.damage = 0;
                    result.description = reactionName(ReactionType::Frozen);
                    break;
                }
                case ReactionType::Swirl: {
                    // Already handled above, but just in case
                    result.damage = transformativeDamage(trigger->eleMastery(), SWIRL_MULT);
                    result.consumesAuras = false;
                    result.spreadElement = auras[i].element != ElementType::Anemo ? auras[i].element : auras[j].element;
                    result.splashDamage = 1;
                    result.splashMultiplier = SWIRL_SPREAD_MULT;
                    result.description = reactionName(ReactionType::Swirl);
                    break;
                }
                case ReactionType::Bloom: {
                    result.createsSeed = true;
                    result.damage = 0;
                    result.description = reactionName(ReactionType::Bloom);
                    break;
                }
                case ReactionType::Burning: {
                    result.appliesBurning = true;
                    result.damage = transformativeDamage(trigger->eleMastery(), BURNING_MULT);
                    result.description = reactionName(ReactionType::Burning);
                    break;
                }
                case ReactionType::Quicken: {
                    result.entersQuicken = true;
                    result.damage = 0;
                    result.description = reactionName(ReactionType::Quicken);
                    break;
                }
                default: break;
            }

            // Consume the reacting auras
            if (result.consumesAuras) {
                auras.removeAt(j);
                auras.removeAt(i);
            }
            return result;
        }
    }

    // Check Quicken + new element
    if (defender->inQuicken()) {
        if (attackElement == ElementType::Dendro) {
            result.type = ReactionType::Spread;
            result.exitsQuicken = true;
            result.damage = REACTION_CONSTANT * SPREAD_AGGRAVATE_MULT *
                            (1.0 + (5.0 * attacker->eleMastery()) / (attacker->eleMastery() + 1200.0));
            result.description = reactionName(ReactionType::Spread);
            result.consumesAuras = false;
            return result;
        }
        if (attackElement == ElementType::Electro) {
            result.type = ReactionType::Aggravate;
            result.exitsQuicken = true;
            result.damage = REACTION_CONSTANT * SPREAD_AGGRAVATE_MULT *
                            (1.0 + (5.0 * attacker->eleMastery()) / (attacker->eleMastery() + 1200.0));
            result.description = reactionName(ReactionType::Aggravate);
            result.consumesAuras = false;
            return result;
        }
    }

    return result;
}

ReactionResult ElementSystem::processSeed(CharacterBase *trigger, ElementType triggerElement,
                                           CharacterBase *target)
{
    ReactionResult result;
    double em = trigger ? trigger->eleMastery() : 0;

    if (triggerElement == ElementType::Pyro) {
        // Burgeon: damages adjacent enemies (1.5x) and adjacent allies (0.6x)
        result.type = ReactionType::Burgeon;
        result.damage = transformativeDamage(em, BURGEON_MULT);
        result.splashDamage = 2; // adjacent all (enemies + allies)
        result.splashMultiplier = BURGEON_MULT; // full for enemies
        result.description = reactionName(ReactionType::Burgeon);
    } else if (triggerElement == ElementType::Electro) {
        // Hyperbloom - two arrows
        result.type = ReactionType::Hyperbloom;
        result.damage = transformativeDamage(em, HYPERBLOOM_MULT) * 2;
        result.description = reactionName(ReactionType::Hyperbloom);
    } else {
        // Bloom explosion (natural)
        result.type = ReactionType::Bloom;
        result.damage = transformativeDamage(em, BLOOM_MULT);
        result.splashDamage = 2; // adjacent all (both sides)
        result.splashMultiplier = BLOOM_MULT;
        result.description = reactionName(ReactionType::Bloom);
    }
    return result;
}
