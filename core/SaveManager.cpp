#include "SaveManager.h"
#include "GameEngine.h"
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
    // Save core game state
    root["primogems"] = engine->primogems();
    root["mora"] = engine->mora();
    root["currentRound"] = engine->currentRound();
    root["playerWins"] = engine->playerWins();
    root["enemyWins"] = engine->enemyWins();
    root["populationLevel"] = engine->populationLevel();
    root["phase"] = static_cast<int>(engine->phase());

    // Board state, storage, shop, etc. would be saved here
    // For the test version, we save the essential state
    QJsonArray storageArr;
    // ... serialize storage pieces
    root["storage"] = storageArr;

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
    // phase restoration handled by GameEngine

    return true;
}
