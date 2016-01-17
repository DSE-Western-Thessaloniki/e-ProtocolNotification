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
  enum Error {NoError, DownloadFailed, HashCheckFailed};

public:
  explicit FileDownloader(QObject *parent = 0);
  virtual ~FileDownloader();
  QStringList downloadedFiles(void) const;
  QMap<int, QByteArray> downloadedData(void) const;
  void getFiles(QVariantList filelist, QUrl baseUrl);
  Error error(void);

 signals:
  void downloaded();
  void hashChecked();

 private slots:
  void fileDownloaded(QNetworkReply* reply);
  void checkHash();

 private:
  QString hash;
  int algorithm;
  QNetworkAccessManager netManager;
  QMap<int,QByteArray> downloadedDataMap;
  QStringList filenameList;
  QStringList hashList;
  QList<bool> hashOK;
  QList<QCryptographicHash::Algorithm> hashTypeList;
  QList<QUrl> urlList;
  int remainingRequests;
  Error err;
};

#endif // FILEDOWNLOADER_H

