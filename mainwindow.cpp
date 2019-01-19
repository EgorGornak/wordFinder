#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QtConcurrent/QtConcurrent>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);
    ui->lineEdit->setDisabled(true);
    ui->treeWidget->setUniformRowHeights(true);


    connect(ui->actionSelectDirectory, &QAction::triggered, this, &MainWindow::selectDirectory);
    connect(ui->actionCancel, &QAction::triggered, this, &MainWindow::Cancel);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::findWord);
    connect(&indexer, &Indexer::setProgressBar, this, &MainWindow::setProgressBar);
    connect(&indexer, &Indexer::addNewFile, this, &MainWindow::addNewFile);
    connect(&indexer, &Indexer::lineSetDisabled, this, &MainWindow::lineSetDisabled);
    connect(&indexer, &Indexer::searchFinished, this, &MainWindow::searchFinished);
}


void MainWindow::selectDirectory() {
    ui->treeWidget->clear();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->statusBar->showMessage("Current directory = " + dir);
    currentDirectory = dir;
    QtConcurrent::run(&indexer, &Indexer::scanDirectory, dir);
    ui->lineEdit->setDisabled(true);
}

void MainWindow::Cancel() {
    indexer.setCancel(true);
}

void MainWindow::setProgressBar(int value) {
    ui->progressBar->setValue(value);
}

void MainWindow::addNewFile(QString path) {
    QTreeWidgetItem* group = new QTreeWidgetItem();

    QString restPath;
    for (int i = currentDirectory.size() + 1; i < path.size(); i++) {
        restPath += path[i];
    }
    group->setText(0, restPath);
    ui->treeWidget->addTopLevelItem(group);
}

void MainWindow::findWord() {
    QString input = ui->lineEdit->text();
    ui->treeWidget->clear();
    QtConcurrent::run(&indexer, &Indexer::findWord, input.toStdString());
}

void MainWindow::lineSetDisabled(bool value) {
    ui->lineEdit->setDisabled(value);
}

void MainWindow::searchFinished(bool found) {
    if (!found) {
        QTreeWidgetItem* notFound = new QTreeWidgetItem();
        notFound->setText(0, "Word not found");
        ui->treeWidget->addTopLevelItem(notFound);
    }
}

MainWindow::~MainWindow() {
    delete ui;
}
