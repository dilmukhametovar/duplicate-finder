#include "mainwindow.h"
#include "core/DuplicateScanner.h"
#include "model/DuplicateGroupsModel.h"
#include "utils/fileutils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QTreeView>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* root = new QVBoxLayout(central);

    // Select directory
    auto* pathRow = new QHBoxLayout;
    pathEdit_  = new QLineEdit(this);
    browseBtn_ = new QPushButton("Select…", this);
    scanBtn_   = new QPushButton("Scan", this);
    cancelBtn_ = new QPushButton("Cancel", this);
    cancelBtn_->setEnabled(false);

    pathRow->addWidget(new QLabel("Directory:", this));
    pathRow->addWidget(pathEdit_, 1);
    pathRow->addWidget(browseBtn_);
    pathRow->addWidget(scanBtn_);
    pathRow->addWidget(cancelBtn_);
    root->addLayout(pathRow);

    // progress bar
    auto* progRow = new QHBoxLayout;
    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    statusLabel_ = new QLabel("Ready to start", this);
    progRow->addWidget(progressBar_, 1);
    progRow->addWidget(statusLabel_);
    root->addLayout(progRow);

    // tree
    model_   = new DuplicateGroupsModel(this);
    treeView_ = new QTreeView(this);
    treeView_->setModel(model_);
    treeView_->setAlternatingRowColors(true);
    treeView_->setUniformRowHeights(true);
    treeView_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    treeView_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    treeView_->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    root->addWidget(treeView_, 1);

    scanner_ = new DuplicateScanner(this);

    uiTick_ = new QTimer(this);
    uiTick_->setInterval(100); // ms

    connect(browseBtn_, &QPushButton::clicked,
            this,       &MainWindow::chooseFolder);
    connect(scanBtn_,   &QPushButton::clicked,
            this,       &MainWindow::startScan);
    connect(cancelBtn_, &QPushButton::clicked,
            scanner_,   &DuplicateScanner::cancel);


    connect(scanner_, &DuplicateScanner::progress,
            this,     &MainWindow::onProgress);
    connect(scanner_, &DuplicateScanner::groupFound,
            this,     &MainWindow::onGroupFound);
    connect(scanner_, &DuplicateScanner::finished,
            this,     &MainWindow::onFinished);
    connect(uiTick_,  &QTimer::timeout,
            this,     &MainWindow::refreshStatusLabel);
}

MainWindow::~MainWindow()
{
}

void MainWindow::chooseFolder() {
    const QString dir = QFileDialog::getExistingDirectory(this, "Choose directory");
    if (!dir.isEmpty()) pathEdit_->setText(dir);
}

void MainWindow::startScan() {
    const QString dir = pathEdit_->text().trimmed();
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "Error", "Choose a directory");
        return;
    }

    model_->clear();
    progressBar_->setValue(0);
    lastScanned_ = 0;
    lastTotal_   = 0;
    scanning_    = true;

    statusLabel_->setText("Scanning");

    scanBtn_->setEnabled(false);
    cancelBtn_->setEnabled(true);

    scanTimer_.start();
    uiTick_->start();
    scanner_->start(dir);
}

void MainWindow::onProgress(int scanned, int total, qint64 /*bytes*/) {
    lastScanned_ = scanned;
    lastTotal_   = total;
    if (total > 0)
        progressBar_->setValue(int(100.0 * scanned / total));
}

void MainWindow::onGroupFound(const QVector<FileEntry>& group) {
    model_->addGroup(group);
}

void MainWindow::onFinished(int groups, qint64 wasted) {
    scanning_ = false;

    uiTick_->stop();
    const qint64 elapsed = scanTimer_.elapsed();

    scanBtn_->setEnabled(true);
    cancelBtn_->setEnabled(false);
    progressBar_->setValue(100);
    statusLabel_->setText(
        QString("Done in %1 s: groups — %2, can be cleaned %3")
            .arg(elapsed / 1000.0)
            .arg(groups)
            .arg(FileUtils::humanReadableSize(wasted)));
}

void MainWindow::refreshStatusLabel() {
    if (!scanTimer_.isValid() || !scanning_)
        return;

    const double seconds = scanTimer_.elapsed() / 1000.0;

    QString progressPart;
    if (lastTotal_ > 0) {
        const int percent = int(100.0 * lastScanned_ / lastTotal_);
        progressPart = QString("%1 / %2 files (%3%)")
                           .arg(lastScanned_)
                           .arg(lastTotal_)
                           .arg(percent);
    } else {
        progressPart = "...";
    }

    statusLabel_->setText(QString("Scanning: %1 - %2 s")
                              .arg(progressPart)
                              .arg(seconds, 0, 'f', 1));

}
