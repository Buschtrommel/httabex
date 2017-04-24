/*
 * httabex - HTML table extractor
 * Copyright (C) 2017 - Matthias Fehring (https://www.buschmann23.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "download.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QUrl>

Download::Download(const QString &url) :
    m_url(url)
{

}


QString Download::get(bool refresh) const
{
    QString localUrl;

    Q_ASSERT(!m_url.isEmpty());

    const QString urlHash = QCryptographicHash::hash(m_url.toUtf8(), QCryptographicHash::Md5).toHex();
    const QString tempFileName = QDir::temp().absoluteFilePath(QLatin1String("httabex-") + urlHash);
    QFile tempFile(tempFileName);

    if (tempFile.exists() && !refresh) {
        qInfo("%s", tr("Found cached data. (Use the --refresh option to force a redownload of the HTML data.)").toUtf8().constData());
        localUrl = tempFileName;
        return localUrl;
    } else if (tempFile.exists() && refresh) {
        qInfo("%s", tr("Redownloading remote data.").toUtf8().constData());
    }

    if (Q_UNLIKELY(!tempFile.open(QIODevice::WriteOnly|QIODevice::Text))) {
        qCritical("%s", tr("Can not open temporary file: %1").arg(tempFileName).toUtf8().constData());
        return localUrl;
    }

    QNetworkAccessManager nam;
    QEventLoop loop;
    const QUrl url(m_url);
    QNetworkRequest request(url);
    QNetworkReply *reply = nam.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (Q_UNLIKELY(tempFile.write(reply->readAll()) < 0)) {
        qCritical("%s", tr("Failed to write to temporary file at %1.").arg(tempFileName).toUtf8().constData());
        qDebug("%s", tempFile.errorString().toUtf8().constData());
        tempFile.close();
        return localUrl;
    }

    tempFile.close();
    reply->deleteLater();

    localUrl = tempFileName;

    return localUrl;
}
