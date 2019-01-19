#ifndef INDEXER_H
#define INDEXER_H

#endif // INDEXER_H

#include <QObject>
#include <QFileInfo>
#include <QTreeWidget>
#include <QMap>

class Indexer : public QObject {
    Q_OBJECT

signals:
    void addNewFile(QString path);
    void setProgressBar(int value);
    void lineSetDisabled(bool value);
    void searchFinished(bool found);

public slots:
    void scanDirectory(QString const &dir);
    void calculateTrigram();
    void setCancel(bool value);
    void findWord(std::string word);


private:
    bool isCancel;
    qint64 totalSize;
    qint64 totalTextSize;
    QFileInfoList filesInfo;
    QString currentDirectory;
    QVector<QPair<QFileInfo, QSet<uint32_t>>> trigramByFile;
    uint32_t getTrigram(char first, char second, char third);

};
