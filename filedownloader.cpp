#include "filedownloader.h"

FileDownloader::FileDownloader(QObject *parent) :
 QObject(parent)
{
    connect(&netManager, SIGNAL (finished(QNetworkReply*)), this, SLOT (fileDownloaded(QNetworkReply*)));
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* reply) {
    int key;

    if (reply->error() == QNetworkReply::NoError) { // All OK
        key = urlList.indexOf(reply->request().url());
        downloadedDataMap[key] = reply->readAll();
    }
    else {
        qDebug() << "Download Error: " << reply->errorString();
    }
    reply->deleteLater();
    remainingRequests--;
    if (!remainingRequests)
        emit downloaded();
}

QMap<int, QByteArray> FileDownloader::downloadedData() const {
    return downloadedDataMap;
}

void FileDownloader::getFiles(QVariantList filelist, QUrl baseUrl)
{
    if (!remainingRequests) { // Accept new list only when the previous is done
        int size = filelist.size();
        if ((size % 3) == 0) {
            for (int i=0; i<size; i+=3) {
                filenameList << filelist[0].toString();
                hashList << filelist[1].toString();
                hashTypeList << filelist[2].toInt();
                urlList << QUrl(baseUrl.toString() + filelist[0].toString());
                netManager.get(QNetworkRequest(QUrl(baseUrl.toString() + filelist[0].toString())));
                remainingRequests++;
            }
        }
        else {
            qDebug() << "Download error: Invalid file list.";
        }
    }
    qDebug() << "Download error: Downloads still pending. Not queuing.";
}
