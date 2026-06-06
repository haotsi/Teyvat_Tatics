#include "SaveManager.h"
#include "GameEngine.h"
#include "Board.h"
#include "CharacterBase.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

bool SaveManager::saveToFile(const QString &filePath, const GameEngine *engine)
{
    QJsonObject root = serialize(engine);
    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonObject SaveManager::serialize(const GameEngine *engine)
{
    QJsonObject root;
    // Core game state
    root["primogems"] = engine->primogems();
    root["mora"] = engine->mora();
    root["currentRound"] = engine->currentRound();
    root["playerWins"] = engine->playerWins();
    root["enemyWins"] = engine->enemyWins();
    root["populationLevel"] = engine->populationLevel();
    root["phase"] = static_cast<int>(engine->phase());

    // Storage characters with equipment
    QJsonArray storageArr;
    for (auto *p : engine->m_storage) {
        QJsonObject obj;
        obj["name"] = p->name();
        obj["constellation"] = p->constellation();
        obj["currentHp"] = p->currentHp();
        obj["currentEnergy"] = p->currentEnergy();
        obj["hasWeapon"] = p->hasWeapon();
        if (p->hasWeapon()) {
            obj["weaponType"] = static_cast<int>(p->weapon().type());
            obj["weaponStars"] = p->weapon().stars();
        }
        QJsonArray artArr;
        for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) {
            auto slot = static_cast<ArtifactSlot>(i);
            QJsonObject artObj;
            artObj["hasArtifact"] = p->hasArtifact(slot);
            if (p->hasArtifact(slot)) {
                artObj["slot"] = static_cast<int>(p->artifact(slot).slot());
                artObj["mainStat"] = static_cast<int>(p->artifact(slot).mainStat());
            }
            artArr.append(artObj);
        }
        obj["artifacts"] = artArr;
        storageArr.append(obj);
    }
    root["storage"] = storageArr;

    // Board pieces with equipment and position
    QJsonArray boardArr;
    for (auto *p : engine->m_board->allPieces()) {
        QJsonObject obj;
        obj["name"] = p->name();
        obj["constellation"] = p->constellation();
        obj["currentHp"] = p->currentHp();
        obj["currentEnergy"] = p->currentEnergy();
        obj["row"] = p->gridPos().row;
        obj["col"] = p->gridPos().col;
        obj["side"] = static_cast<int>(p->side());
        obj["hasWeapon"] = p->hasWeapon();
        if (p->hasWeapon()) {
            obj["weaponType"] = static_cast<int>(p->weapon().type());
            obj["weaponStars"] = p->weapon().stars();
        }
        QJsonArray artArr;
        for (int i = 0; i < MAX_ARTIFACT_SLOTS; ++i) {
            auto slot = static_cast<ArtifactSlot>(i);
            QJsonObject artObj;
            artObj["hasArtifact"] = p->hasArtifact(slot);
            if (p->hasArtifact(slot)) {
                artObj["slot"] = static_cast<int>(p->artifact(slot).slot());
                artObj["mainStat"] = static_cast<int>(p->artifact(slot).mainStat());
            }
            artArr.append(artObj);
        }
        obj["artifacts"] = artArr;
        boardArr.append(obj);
    }
    root["boardPieces"] = boardArr;

    // Weapon backpack
    QJsonArray wpnArr;
    for (auto &w : engine->m_weaponBackpack) {
        QJsonObject obj;
        obj["type"] = static_cast<int>(w.type());
        obj["stars"] = w.stars();
        wpnArr.append(obj);
    }
    root["weaponBackpack"] = wpnArr;

    // Artifact backpack
    QJsonArray artBackArr;
    for (auto &a : engine->m_artifactBackpack) {
        QJsonObject obj;
        obj["slot"] = static_cast<int>(a.slot());
        obj["mainStat"] = static_cast<int>(a.mainStat());
        artBackArr.append(obj);
    }
    root["artifactBackpack"] = artBackArr;

    return root;
}

bool SaveManager::loadFromFile(const QString &filePath, GameEngine *engine)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isNull()) return false;
    return deserialize(doc.object(), engine);
}

bool SaveManager::deserialize(const QJsonObject &json, GameEngine *engine)
{
    if (!engine) return false;

    engine->setPrimogems(json["primogems"].toInt(INITIAL_PRIMOGEMS));
    engine->setMora(json["mora"].toInt(INITIAL_MORA));
    engine->setCurrentRound(json["currentRound"].toInt(1));
    engine->setPlayerWins(json["playerWins"].toInt(0));
    engine->setEnemyWins(json["enemyWins"].toInt(0));
    engine->setPopulationLevel(json["populationLevel"].toInt(INITIAL_POPULATION));

    // Clear existing state
    engine->m_storage.clear();
    engine->m_board->clear();
    engine->m_weaponBackpack.clear();
    engine->m_artifactBackpack.clear();
    engine->m_characterRegistry.clear();

    // Rebuild character pool reference for cloning
    auto &pool = engine->m_characterPool;
    auto findInPool = [&pool](const QString &name) -> CharacterBase* {
        for (auto &c : pool)
            if (c->name() == name) return c.get();
        return nullptr;
    };

    // Restore storage characters
    QJsonArray storageArr = json["storage"].toArray();
    for (auto val : storageArr) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();
        auto *proto = findInPool(name);
        if (!proto) continue;
        auto cloned = proto->clone();
        auto *p = cloned.release();
        p->setConstellation(obj["constellation"].toInt(0));
        p->setCurrentHp(obj["currentHp"].toInt(p->maxHp()));
        p->setCurrentEnergy(obj["currentEnergy"].toInt(0));
        if (obj["hasWeapon"].toBool(false)) {
            ::WeaponType wt = static_cast<::WeaponType>(obj["weaponType"].toInt());
            int stars = obj["weaponStars"].toInt(2);
            p->setWeapon(Weapon(wt, stars));
        } else {
            p->clearWeapon();
        }
        QJsonArray artArr = obj["artifacts"].toArray();
        for (int i = 0; i < artArr.size() && i < MAX_ARTIFACT_SLOTS; ++i) {
            QJsonObject artObj = artArr[i].toObject();
            if (artObj["hasArtifact"].toBool(false)) {
                auto slot = static_cast<::ArtifactSlot>(artObj["slot"].toInt());
                auto stat = static_cast<::ArtifactMainStat>(artObj["mainStat"].toInt());
                p->setArtifact(slot, Artifact(slot, stat));
            }
        }
        engine->m_storage.append(p);
        engine->registerCharacter(p);
    }

    // Restore board pieces
    QJsonArray boardArr = json["boardPieces"].toArray();
    for (auto val : boardArr) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();
        auto *proto = findInPool(name);
        if (!proto) continue;
        auto cloned = proto->clone();
        auto *p = cloned.release();
        p->setConstellation(obj["constellation"].toInt(0));
        p->setCurrentHp(obj["currentHp"].toInt(p->maxHp()));
        p->setCurrentEnergy(obj["currentEnergy"].toInt(0));
        p->setSide(static_cast<TeamSide>(obj["side"].toInt(0)));
        if (obj["hasWeapon"].toBool(false)) {
            ::WeaponType wt = static_cast<::WeaponType>(obj["weaponType"].toInt());
            int stars = obj["weaponStars"].toInt(2);
            p->setWeapon(Weapon(wt, stars));
        } else {
            p->clearWeapon();
        }
        QJsonArray artArr = obj["artifacts"].toArray();
        for (int i = 0; i < artArr.size() && i < MAX_ARTIFACT_SLOTS; ++i) {
            QJsonObject artObj = artArr[i].toObject();
            if (artObj["hasArtifact"].toBool(false)) {
                auto slot = static_cast<::ArtifactSlot>(artObj["slot"].toInt());
                auto stat = static_cast<::ArtifactMainStat>(artObj["mainStat"].toInt());
                p->setArtifact(slot, Artifact(slot, stat));
            }
        }
        int row = obj["row"].toInt(-1);
        int col = obj["col"].toInt(-1);
        if (row >= 0 && col >= 0) {
            engine->m_board->placePiece(row, col, p);
        } else {
            engine->m_storage.append(p);
        }
        engine->registerCharacter(p);
    }

    // Restore weapon backpack
    QJsonArray wpnArr = json["weaponBackpack"].toArray();
    for (auto val : wpnArr) {
        QJsonObject obj = val.toObject();
        auto wt = static_cast<::WeaponType>(obj["type"].toInt());
        int stars = obj["stars"].toInt(2);
        engine->m_weaponBackpack.append(Weapon(wt, stars));
    }

    // Restore artifact backpack
    QJsonArray artBackArr = json["artifactBackpack"].toArray();
    for (auto val : artBackArr) {
        QJsonObject obj = val.toObject();
        auto slot = static_cast<::ArtifactSlot>(obj["slot"].toInt());
        auto stat = static_cast<::ArtifactMainStat>(obj["mainStat"].toInt());
        engine->m_artifactBackpack.append(Artifact(slot, stat));
    }

    return true;
}
