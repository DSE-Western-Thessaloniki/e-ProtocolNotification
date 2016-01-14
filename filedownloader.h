#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCryptographicHash>

class FileDownloader : public QObject
{
 Q_OBJECT
 public:
  explicit FileDownloader(QObject *parent = 0);
  virtual ~FileDownloader();
  QMap<int, QByteArray> downloadedData() const;
  void getFiles(QVariantList filelist, QUrl baseUrl);

 signals:
  void downloaded();

 private slots:
  void fileDownloaded(QNetworkReply* reply);

 private:
  QString hash;
  int algorithm;
  QNetworkAccessManager netManager;
  QMap<int,QByteArray> downloadedDataMap;
  QStringList filenameList;
  QStringList hashList;
  QList<int> hashTypeList;
  QList<QUrl> urlList;
  int remainingRequests;
};

#endif // FILEDOWNLOADER_H

