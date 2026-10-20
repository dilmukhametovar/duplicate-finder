#include "HashWorker.h"
#include <QCryptographicHash>
#include <QFile>

constexpr qint64 kMiniHashBytes = 4096;     // 4 KiB
constexpr qint64 kChunkSize     = 1 << 20;  // 1 MiB

HashWorker::HashWorker(FileEntry entry,
                       bool fullHash,
                       const CancellationToken* token,
                       Callback onDone)
        : entry_(entry),
        fullHash_(fullHash),
        token_(token),
        onDone_(std::move(onDone)) {
    setAutoDelete(true);

}

void HashWorker::run() {
    QFile f(entry_.path);
    if (!f.open(QIODevice::ReadOnly))
    {
        if (onDone_)
            onDone_(std::move(entry_), fullHash_);
        return;
    }

    const qint64 toRead = fullHash_
                            ? entry_.size
                            : std::min<qint64>(kMiniHashBytes, entry_.size);


    QCryptographicHash hasher(QCryptographicHash::Sha256);
    QByteArray buffer;
    buffer.resize(std::min(kChunkSize, toRead));

    qint64 read = 0;
    while (read < toRead) {
        if (token_ && token_->isCancelled()) {
            if (onDone_) onDone_(std::move(entry_), fullHash_);
            return;
        }

        const qint64 want = std::min(buffer.size(), toRead - read);
        const qint64 got = f.read(buffer.data(), want);
        if (got <= 0)
            break;

        hasher.addData(buffer);
        read += got;
    }

    if (fullHash_)
        entry_.fullHash = hasher.result();
    else
        entry_.miniHash = hasher.result();

    if (onDone_)
        onDone_(std::move(entry_), fullHash_);
}
