#include "DuplicateScanner.h"
#include "FileEntry.h"
#include "HashWorker.h"
#include <QDirIterator>
#include <QHash>
#include <QList>
#include <QMutexLocker>
#include <QThreadPool>
#include <QVector>

DuplicateScanner::DuplicateScanner(QObject* parent)
    : QObject(parent) {

}

void DuplicateScanner::cancel() {
    token_.cancel();
}

void DuplicateScanner::start(const QString& rootDir, int threadCount) {
    token_.reset();

    {
        QMutexLocker l(&mutex_);

        //byMiniHash_.clear();
        //byFullHash_.clear();

        pending_ = 0;
        scannedFiles_ = 0;
        totalFiles_ = 0;

        stage_ = IDLE;
    }

    if (threadCount <= 0)
        threadCount = QThread::idealThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(threadCount);

    // Stage 1 - visit tree
    QHash<qint64, QVector<FileEntry>> bySize;
    int total = 0;

    QDirIterator dit(rootDir,
                    QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (dit.hasNext()) {
        if (token_.isCancelled())
        {
            emit finished(0, 0);
            return;
        }

        dit.next();

        const QFileInfo info = dit.fileInfo();
        if (info.isSymLink())
            continue;

        if (info.size() == 0)
            continue;

        FileEntry e;
        e.path = info.absoluteFilePath();
        e.size = info.size();

        bySize[e.size].append(e);
        ++total;
    }

    if (token_.isCancelled())
    {
        emit finished(0, 0);
        return;
    }

    QVector<FileEntry> candidates;
    for (auto& files: bySize) {
        if (files.size() >= 2) {
            for (const auto& f: files) {
                candidates.append(f);
            }
        }
    }

    {
        QMutexLocker l(&mutex_);
        totalFiles_ = total;
    }
    emit progress(0, total, 0);

    if (candidates.isEmpty()) {
        emit finished(0, 0);
        return;
    }

    // Stage 2 - mini hash
    {
        QMutexLocker l(&mutex_);
        stage_   = MINI;
        pending_ = candidates.size();
    }

    for (const auto& e : candidates) {
        auto* w = new HashWorker(e, /*fullHash=*/false, &token_,
                                 [this](FileEntry en, bool fh) { onHashed(std::move(en), fh); });
        QThreadPool::globalInstance()->start(w);
    }
}

void DuplicateScanner::onHashed(FileEntry entry, bool fullHash) {
    enum Action { None, StartStage3, Finish };
    Action action = None;

    QVector<FileEntry>          nextStage;
    QVector<QVector<FileEntry>> groups;
    int     groupsCount = 0;
    qint64  wastedBytes = 0;
    int     scanned     = 0;
    int     total       = 0;

    {
        QMutexLocker l(&mutex_);

        if (fullHash) {
            if (!entry.fullHash.isEmpty())
                byFullHash_[entry.fullHash].append(entry);
        } else {
            if (!entry.miniHash.isEmpty())
                byMiniHash_[entry.miniHash].append(entry);
        }

        ++scannedFiles_;
        scanned = scannedFiles_;
        total   = totalFiles_;

        if (--pending_ == 0) {
            if (stage_ == MINI) {
                // Select groups where mini hash is equal
                for (auto it = byMiniHash_.cbegin(); it != byMiniHash_.cend(); ++it) {
                    if (it.value().size() >= 2)
                        for (const auto& f : it.value())
                            nextStage.append(f);
                }
                byMiniHash_.clear();

                if (nextStage.isEmpty()) {
                    stage_ = IDLE;
                    action = Finish;
                } else {
                    stage_ = FULL;
                    pending_ = nextStage.size();
                    action = StartStage3;
                }
            } else if (stage_ == FULL) {
                for (auto it = byFullHash_.cbegin(); it != byFullHash_.cend(); ++it) {
                    if (it.value().size() >= 2) {
                        groups.append(it.value());
                        ++groupsCount;
                        wastedBytes += it.value().front().size
                                       * (it.value().size() - 1);
                    }
                }
                byFullHash_.clear();
                stage_ = IDLE;
                action = Finish;
            }
        }
    }

    emit progress(scanned, total, 0);

    if (action == StartStage3) {
        for (const auto& f : nextStage) {
            auto* w = new HashWorker(f, true, &token_,
                                        [this](FileEntry e, bool fh) { onHashed(std::move(e), fh); });
            QThreadPool::globalInstance()->start(w);
        }
    } else if (action == Finish) {
        for (const auto& g : groups)
            emit groupFound(g);
        emit finished(groupsCount, wastedBytes);
    }
}
