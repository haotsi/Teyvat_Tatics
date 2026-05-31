#ifndef BOARD_H
#define BOARD_H

#include "GameTypes.h"
#include <memory>
#include <QVector>
#include <QObject>

class CharacterBase;

class Board : public QObject {
    Q_OBJECT
public:
    explicit Board(QObject *parent = nullptr);

    void clear();
    void resetBoard();

    // Access cells
    CharacterBase* pieceAt(int row, int col) const;
    CharacterBase* pieceAt(GridPos pos) const;
    void placePiece(int row, int col, CharacterBase *piece);
    void placePiece(GridPos pos, CharacterBase *piece);
    CharacterBase* removePiece(int row, int col);
    CharacterBase* removePiece(GridPos pos);
    bool isOccupied(int row, int col) const;
    bool isOccupied(GridPos pos) const;
    bool isEmpty(int row, int col) const;
    bool isEmpty(GridPos pos) const;

    // Check if position is in player deploy zone (rows 4-7, 0-indexed)
    bool isPlayerDeployZone(GridPos pos) const;
    bool isEnemyDeployZone(GridPos pos) const;

    // Get all player/enemy pieces on board
    QVector<CharacterBase*> playerPieces() const;
    QVector<CharacterBase*> enemyPieces() const;

    // Get all pieces
    QVector<CharacterBase*> allPieces() const;

    // Clear a side
    void clearSide(TeamSide side);

    // Movement helpers for battle
    QVector<GridPos> getValidMoves(CharacterBase *piece) const;
    QVector<GridPos> getValidAttackTargets(CharacterBase *piece) const;

    // Move piece from one cell to another
    void movePiece(GridPos from, GridPos to);

    // Get adjacent positions (4-direction)
    QVector<GridPos> adjacentPositions4(GridPos pos) const;
    // Get adjacent positions (8-direction)
    QVector<GridPos> adjacentPositions8(GridPos pos) const;

signals:
    void boardChanged();

private:
    CharacterBase* m_grid[BOARD_ROWS][BOARD_COLS] = {};
};

#endif // BOARD_H
