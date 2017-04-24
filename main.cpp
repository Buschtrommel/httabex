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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QUrl>
#include <QRegularExpression>

#include "download.h"
#include "process.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setOrganizationName(QStringLiteral("Buschtrommel"));
    a.setOrganizationDomain(QStringLiteral("buschmann23.de"));
    a.setApplicationVersion(QStringLiteral("0.0.1"));

    QCommandLineParser clp;

    clp.addPositionalArgument(QStringLiteral("URL"), QCoreApplication::translate("main", "The URL to the local HTML file or web page to extract the table from."));

    QCommandLineOption tableNo(QStringList({QStringLiteral("t"), QStringLiteral("table")}), QCoreApplication::translate("main", "The number of the tabel to extract, starting at 1."), QStringLiteral("table number"), QStringLiteral("1"));
    clp.addOption(tableNo);

    QCommandLineOption column(QStringList({QStringLiteral("c"), QStringLiteral("column")}), QCoreApplication::translate("main", "The number of the column to extract, starting at 1."), QStringLiteral("column number"), QStringLiteral("1"));
    clp.addOption(column);

    QCommandLineOption refresh(QStringList({QStringLiteral("r"), QStringLiteral("refresh")}), QCoreApplication::translate("main", "Forces a redownload of remote data."));
    clp.addOption(refresh);

    clp.addHelpOption();
    clp.addVersionOption();

    clp.process(a);

    const QStringList posOptions = clp.positionalArguments();
    if (posOptions.empty()) {
        clp.showHelp();
    } else {
        QString url = posOptions.at(0);
        if (url.contains(QRegularExpression(QStringLiteral("^(http|ftp)s?"), QRegularExpression::CaseInsensitiveOption))) {
            qInfo("%s", QCoreApplication::translate("main", "Downloading HTML file: %1").arg(url).toUtf8().constData());
            Download download(url);
            url = download.get(clp.isSet(refresh));
            if (Q_UNLIKELY(url.isEmpty())) {
                return 1;
            }
        } else {
            qInfo("%s", QCoreApplication::translate("main", "Opening local file: %1").arg(url).toUtf8().constData());
        }
        Process process(url, clp.value(tableNo).toInt(), clp.value(column).toInt());
        qDebug() << process.exec();
    }

    return 0;
}
