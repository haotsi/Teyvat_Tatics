#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QString>
#include <QJsonObject>

class GameEngine;

class SaveManager {
public:
    static bool saveToFile(const QString &filePath, const GameEngine *engine);
    static QJsonObject serialize(const GameEngine *engine);
    static bool loadFromFile(const QString &filePath, GameEngine *engine);
    static bool deserialize(const QJsonObject &json, GameEngine *engine);
};

#endif // SAVEMANAGER_H
