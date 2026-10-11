#pragma once
#include "core/FileEntry.h"
#include <QAbstractItemModel>
#include <QVector>

class DuplicateGroupsModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit DuplicateGroupsModel(QObject* parent = nullptr);

    void addGroup(const QVector<FileEntry>& group);
    void clear();

    QModelIndex index(int row, int col, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation o, int role) const override;

private:
    struct Group { QVector<FileEntry> files; qint64 wasted = 0; };
    QVector<Group> groups_;
};
