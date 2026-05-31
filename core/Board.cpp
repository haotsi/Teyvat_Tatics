#include "Board.h"
#include "CharacterBase.h"

Board::Board(QObject *parent) : QObject(parent)
{
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            m_grid[r][c] = nullptr;
}

void Board::clear()
{
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            m_grid[r][c] = nullptr;
    emit boardChanged();
}

void Board::resetBoard()
{
    clear();
}

CharacterBase* Board::pieceAt(int row, int col) const
{
    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) return nullptr;
    return m_grid[row][col];
}

CharacterBase* Board::pieceAt(GridPos pos) const
{
    return pieceAt(pos.row, pos.col);
}

void Board::placePiece(int row, int col, CharacterBase *piece)
{
    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) return;
    m_grid[row][col] = piece;
    if (piece) piece->setGridPos({row, col});
    emit boardChanged();
}

void Board::placePiece(GridPos pos, CharacterBase *piece)
{
    placePiece(pos.row, pos.col, piece);
}

CharacterBase* Board::removePiece(int row, int col)
{
    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) return nullptr;
    auto *p = m_grid[row][col];
    m_grid[row][col] = nullptr;
    if (p) p->setGridPos({-1, -1});
    emit boardChanged();
    return p;
}

CharacterBase* Board::removePiece(GridPos pos)
{
    return removePiece(pos.row, pos.col);
}

bool Board::isOccupied(int row, int col) const { return pieceAt(row, col) != nullptr; }
bool Board::isOccupied(GridPos pos) const { return isOccupied(pos.row, pos.col); }
bool Board::isEmpty(int row, int col) const { return !isOccupied(row, col); }
bool Board::isEmpty(GridPos pos) const { return isEmpty(pos.row, pos.col); }

bool Board::isPlayerDeployZone(GridPos pos) const
{
    return pos.row >= PLAYER_START_ROW - 1 && pos.row < BOARD_ROWS;
}

bool Board::isEnemyDeployZone(GridPos pos) const
{
    return pos.row < PLAYER_START_ROW - 1;
}

QVector<CharacterBase*> Board::playerPieces() const
{
    QVector<CharacterBase*> result;
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (m_grid[r][c] && m_grid[r][c]->side() == TeamSide::Player)
                result.append(m_grid[r][c]);
    return result;
}

QVector<CharacterBase*> Board::enemyPieces() const
{
    QVector<CharacterBase*> result;
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (m_grid[r][c] && m_grid[r][c]->side() == TeamSide::Enemy)
                result.append(m_grid[r][c]);
    return result;
}

QVector<CharacterBase*> Board::allPieces() const
{
    QVector<CharacterBase*> result;
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (m_grid[r][c])
                result.append(m_grid[r][c]);
    return result;
}

void Board::clearSide(TeamSide side)
{
    for (int r = 0; r < BOARD_ROWS; ++r)
        for (int c = 0; c < BOARD_COLS; ++c)
            if (m_grid[r][c] && m_grid[r][c]->side() == side)
                m_grid[r][c] = nullptr;
    emit boardChanged();
}

void Board::movePiece(GridPos from, GridPos to)
{
    auto *p = removePiece(from);
    if (p) placePiece(to, p);
}

QVector<GridPos> Board::getValidMoves(CharacterBase *piece) const
{
    QVector<GridPos> result;
    if (!piece || !piece->isAlive()) return result;

    GridPos pos = piece->gridPos();
    // Can move to any adjacent cell (8 directions) that's empty
    auto neighbors = pos.neighbors8();
    for (auto &n : neighbors) {
        if (isEmpty(n))
            result.append(n);
    }
    return result;
}

QVector<GridPos> Board::getValidAttackTargets(CharacterBase *piece) const
{
    QVector<GridPos> result;
    if (!piece || !piece->isAlive()) return result;

    GridPos pos = piece->gridPos();
    WeaponType wt = piece->weaponType();
    int range = (wt == WeaponType::Bow || wt == WeaponType::Catalyst || wt == WeaponType::Polearm) ? 2 : 1;

    switch (wt) {
        case WeaponType::Sword:
        case WeaponType::Bow:
        case WeaponType::Polearm: {
            // 4 directions: up, down, left, right
            static const int dirs4[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
            for (auto &d : dirs4) {
                for (int dist = 1; dist <= range; ++dist) {
                    GridPos t{pos.row + d[0]*dist, pos.col + d[1]*dist};
                    if (!t.isValid()) break;
                    CharacterBase *target = pieceAt(t);
                    if (target && target->side() != piece->side() && target->isAlive()) {
                        result.append(t);
                        // For polearm, continue in line (pierce)
                        if (wt != WeaponType::Polearm) break;
                    }
                    if (target && wt != WeaponType::Polearm) break;
                }
            }
            break;
        }
        case WeaponType::Claymore: {
            // 8 directions, range 1
            auto neighbors = pos.neighbors8();
            for (auto &n : neighbors) {
                CharacterBase *target = pieceAt(n);
                if (target && target->side() != piece->side() && target->isAlive())
                    result.append(n);
            }
            break;
        }
        case WeaponType::Catalyst: {
            // All cells within range 2 (24 cells)
            for (int dr = -2; dr <= 2; ++dr)
                for (int dc = -2; dc <= 2; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    GridPos t{pos.row + dr, pos.col + dc};
                    if (!t.isValid()) continue;
                    CharacterBase *target = pieceAt(t);
                    if (target && target->side() != piece->side() && target->isAlive())
                        result.append(t);
                }
            break;
        }
    }
    return result;
}

QVector<GridPos> Board::adjacentPositions4(GridPos pos) const
{
    QVector<GridPos> result;
    auto neighbors = pos.neighbors4();
    for (auto &n : neighbors) result.append(n);
    return result;
}

QVector<GridPos> Board::adjacentPositions8(GridPos pos) const
{
    QVector<GridPos> result;
    auto neighbors = pos.neighbors8();
    for (auto &n : neighbors) result.append(n);
    return result;
}
