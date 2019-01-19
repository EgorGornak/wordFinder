#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <indexer.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void Cancel();
    void findWord();
    void selectDirectory();
    void setProgressBar(int value);
    void addNewFile(QString path);
    void lineSetDisabled(bool value);
    void searchFinished(bool found);

private:
    Ui::MainWindow *ui;
    Indexer indexer;
    QString currentDirectory;
};

#endif // MAINWINDOW_H
