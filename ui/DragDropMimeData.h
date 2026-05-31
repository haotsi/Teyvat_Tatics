#ifndef DRAGDROPMIMEDATA_H
#define DRAGDROPMIMEDATA_H

#include <QMimeData>
#include <QPoint>

class CharacterBase;

class DragDropMimeData : public QMimeData {
    Q_OBJECT
public:
    inline static const QString MIME_TYPE = QStringLiteral("teyvat/piece-drag");

    explicit DragDropMimeData() {
        setData(MIME_TYPE, QByteArray());
    }

    void setSourceType(const QString &type) { m_sourceType = type; }
    QString sourceType() const { return m_sourceType; }

    void setSourceIndex(int index) { m_sourceIndex = index; }
    int sourceIndex() const { return m_sourceIndex; }

    void setGridPos(int row, int col) { m_row = row; m_col = col; }
    int gridRow() const { return m_row; }
    int gridCol() const { return m_col; }

    void setBackpackIndex(int index) { m_backpackIndex = index; }
    int backpackIndex() const { return m_backpackIndex; }

    void setArtifactTargetSlot(int slot) { m_artifactSlot = slot; }
    int artifactTargetSlot() const { return m_artifactSlot; }

    static bool isPieceDrag(const QMimeData *data) {
        return data && data->hasFormat(MIME_TYPE);
    }

    static DragDropMimeData* fromMimeData(const QMimeData *data) {
        return const_cast<DragDropMimeData*>(
            qobject_cast<const DragDropMimeData*>(data));
    }

private:
    QString m_sourceType;
    int m_sourceIndex = -1;
    int m_row = -1;
    int m_col = -1;
    int m_backpackIndex = -1;
    int m_artifactSlot = -1;
};

#endif // DRAGDROPMIMEDATA_H
