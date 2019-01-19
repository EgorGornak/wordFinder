#include <indexer.h>

#include <QDirIterator>
#include <QDebug>
#include <QFile>




void Indexer::scanDirectory(QString const &directory) {
    totalSize = 0;
    totalTextSize = 0;
    isCancel = false;
    filesInfo.clear();
    trigramByFile.clear();
    currentDirectory = directory;

    QDirIterator it(directory, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        if (isCancel) {
            return;
        }

        QFileInfo temp = it.next();
        if (temp.isFile()) {
            filesInfo.push_back(temp);
            totalSize += temp.size();
        }
    }
    calculateTrigram();
}

void Indexer::calculateTrigram() {
    qint64 currentSize = 0;
    emit setProgressBar(0);
    for (auto const &fileInfo: filesInfo) {
        if (isCancel) {
            emit setProgressBar(100);
            return;
        }

        QFile file(fileInfo.filePath());

        bool isTextFile = false;
        char last[2];
        char buffer[1024];
        QSet<uint32_t> trigrams;

        if (file.open(QIODevice::ReadOnly)) {
            isTextFile = true;
            while(!file.atEnd()) {
                qint64 bufferSize = file.read(buffer, sizeof(buffer));

                if (!trigrams.empty()) {
                    trigrams.insert(getTrigram(last[0], last[1], buffer[0]));
                    if (bufferSize > 1) {
                        trigrams.insert(getTrigram(last[1], buffer[0], buffer[1]));
                    }
                }

                for (int i = 2; i < bufferSize; i++) {
                    trigrams.insert(getTrigram(buffer[i - 2], buffer[i - 1], buffer[i]));
                }
                if (bufferSize > 1) {
                    last[0] = buffer[bufferSize - 2];
                    last[1] = buffer[bufferSize - 1];
                }
                if (trigrams.size() > 50000) {
                    isTextFile = false;
                    break;
                }
            }

        }


        if (isTextFile) {
            trigramByFile.push_back(qMakePair(fileInfo, std::move(trigrams)));
            totalTextSize += fileInfo.size();
        }

        currentSize += file.size();
        emit setProgressBar(static_cast<int>(100 * currentSize / totalSize));
    }
    emit lineSetDisabled(false);
}


void Indexer::findWord(std::string word) {
    emit setProgressBar(0);
    qint64 currentSize = 0;

    // Knuth-Morris-Pratt ке5
    word += '\0';
    std::vector <size_t> pi(word.size());
    for (size_t i = 1; i < word.size(); i++) {
        size_t j = pi[i - 1];
        while (j > 0 && word[i] != word[j]) {
            j = pi[j - 1];
        }
        if (word[i] == word[j]) {
            j++;
        }
        pi[i] = j;
    }

    for (auto const &info: trigramByFile) {
        if (isCancel) {
            emit setProgressBar(100);
            break;
        }

        bool allTrigrams = true;
        for (unsigned int i = 2; i < word.size() - 1; i++) {
            uint32_t trigram = getTrigram(word[i - 2], word[i - 1], word[i]);
            if (info.second.find(trigram) == info.second.end()) {
                allTrigrams = false;
                break;
            }
        }

        if (!allTrigrams) {
            currentSize += info.first.size();
            emit setProgressBar(static_cast<int>(100 * currentSize / totalTextSize));
            continue;
        }

        QFile file(info.first.filePath());
        file.open(QIODevice::ReadOnly);

        char buffer[1024];
        size_t last = pi.back();
        bool wordFound = false;
        while(!file.atEnd()) {
            qint64 bufferSize = file.read(buffer, sizeof(buffer));
            for (qint64 i = 0; i < bufferSize; i++) {
                size_t j = last;
                while (j > 0 && buffer[i] != word[j]) {
                    j = pi[j - 1];
                }
                if (buffer[i] == word[j]) {
                    j++;
                }
                last = j;
                if (last + 1 == word.size()) {
                    wordFound = true;
                    emit addNewFile(info.first.filePath());
                    break;
                }
            }
            if (wordFound) {
                break;
            }
        }
        currentSize += info.first.size();
        emit setProgressBar(static_cast<int>(100 * currentSize / totalTextSize));
    }

    emit searchFinished();
}

void Indexer::setCancel(bool value) {
    isCancel = value;
}

uint32_t Indexer::getTrigram(char first, char second, char third) {
    return  (static_cast<uint32_t>(first) +
             static_cast<uint32_t>(second << 8) +
             static_cast<uint32_t>(third << 16));
}
