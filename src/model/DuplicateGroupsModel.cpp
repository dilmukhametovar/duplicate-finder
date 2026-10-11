#include "DuplicateGroupsModel.h"
#include "utils/fileutils.h"
#include <QColor>
#include <QFileInfo>

DuplicateGroupsModel::DuplicateGroupsModel(QObject* parent)
        : QAbstractItemModel(parent) {
}

void DuplicateGroupsModel::addGroup(const QVector<FileEntry>& group) {
    if (group.size() < 2)
        return;

    const int row = groups_.size();
    beginInsertRows(QModelIndex(), row, row);

    Group g;
    g.files = group,
    g.wasted = group.front().size * (group.size() - 1);
    groups_.append(g);

    endInsertRows();
}

void DuplicateGroupsModel::clear() {
    beginResetModel();
    groups_.clear();
    endResetModel();
}

QModelIndex DuplicateGroupsModel::index(int row, int col, const QModelIndex& parent) const {
    if (row < 0 || col < 0 || col >= 3)
        return {};

    if (!parent.isValid()) {
        if (row >= groups_.size())
            return {};

        return createIndex(row, col, quintptr(0));
    }

    const int gid = parent.row();
    if (gid < 0 || gid >= groups_.size())
        return {};
    if (row >= groups_[gid].files.size())
        return {};

    return createIndex(row, col, quintptr(gid + 1));
}

QModelIndex DuplicateGroupsModel::parent(const QModelIndex& child) const {
    if (!child.isValid())
        return {};

    const quintptr id = child.internalId();
    if (id == 0)
        return {};

    return createIndex(static_cast<int>(id) - 1, 0, quintptr(0));
}

int DuplicateGroupsModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid())
        return groups_.size();

    if (parent.internalId() != 0)
        return 0;

    const int gid = parent.row();
    if (gid < 0 || gid >= groups_.size())
        return 0;

    return groups_[gid].files.size();
}

int DuplicateGroupsModel::columnCount(const QModelIndex& parent) const {
    return 3;
}

QVariant DuplicateGroupsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    const quintptr id = index.internalId();

    // group
    if (id == 0) {
        const Group& g = groups_[index.row()];
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0: return QString("Group of %1 files").arg(g.files.size());
            case 1: return FileUtils::humanReadableSize(g.wasted);
            case 2: return QFileInfo(g.files.front().path).absolutePath();
            }
        }

        if (role == Qt::ForegroundRole) {
            return QColor(Qt::darkRed);
        }

        return {};
    }

    // file
    const int gid = static_cast<int>(id) - 1;
    if (gid < 0 || gid >= groups_.size())
        return {};

    const FileEntry& e = groups_[gid].files[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return QFileInfo(e.path).fileName();
        case 1: return FileUtils::humanReadableSize(e.size);
        case 2: return e.path;
        }
    }

    return {};
}

QVariant DuplicateGroupsModel::headerData(int section, Qt::Orientation o, int role) const {
    switch (section) {
    case 0: return "Name";
    case 1: return "Size";
    case 2: return "Path";
    }
    return {};
}
