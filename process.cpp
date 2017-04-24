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

#include "process.h"
#include <QFile>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomElement>
#include <tidy.h>
#include <tidybuffio.h>
#include <errno.h>

Process::Process(const QString &filePath, int tableNo, int colNo) :
    m_filePath(filePath), m_tableNo(tableNo), m_colNo(colNo)
{
    if (m_tableNo > 0) {
        m_tableNo--;
    }

    if (m_tableNo < 0) {
        qWarning("%s", tr("Invalid table number. Using first table.").toUtf8().constData());
        m_tableNo = 0;
    }

    if (m_colNo > 0) {
        m_colNo--;
    }
}


bool Process::exec() const
{
    Q_ASSERT(!m_filePath.isEmpty());

    QFile localFile(m_filePath);
    if (Q_UNLIKELY(!localFile.open(QIODevice::ReadOnly|QIODevice::Text))) {
        qCritical("%s", tr("Can not read data from file!").toUtf8().constData());
        qDebug("%s", localFile.errorString().toUtf8().constData());
        return false;
    }

    const QByteArray htmlData = localFile.readAll();
    localFile.close();

    QDomDocument html;
    QString errorString;
    int errorLine = -1;
    int errorColumn = -1;
    if (!html.setContent(htmlData, &errorString, &errorLine, &errorColumn)) {
        qWarning("%s", tr("Failed to parse HTML at Line %1: %2").arg(QString::number(errorLine), errorString).toUtf8().constData());
        qInfo("%s", tr("Using HTML tidy to make it conform to XML.").toUtf8().constData());
        const QString tidied = tidyUp(htmlData);
        if (!html.setContent(tidied, &errorString, &errorLine, &errorColumn)) {
            qWarning("%s", tr("Failed to parse HTML at Line %1: %2").arg(QString::number(errorLine), errorString).toUtf8().constData());
            return false;
        }
    }

    QDomNodeList tables = html.elementsByTagName(QStringLiteral("table"));
    if (Q_UNLIKELY(tables.isEmpty())) {
        qCritical("%s", tr("No tables found.").toUtf8().constData());
        return false;
    }
    qInfo("%s", tr("Found %n table(s).", "", tables.size()).toUtf8().constData());

    if (m_tableNo > tables.size()) {
        qCritical("%s", tr("The selected table number is higher than the amount of tables.").toUtf8().constData());
        return false;
    }

    const QDomElement table = tables.at(m_tableNo).toElement();
    if (Q_UNLIKELY(table.isNull())) {
        qCritical("%s", tr("Table number %1 is not valid.").arg(m_tableNo + 1).toUtf8().constData());
        return false;
    }

    const QDomElement tbody = table.firstChildElement(QStringLiteral("tbody"));
    QDomNodeList rows;
    if (tbody.isNull()) {
        rows = table.childNodes();
    } else {
        rows = tbody.childNodes();
    }

    if (Q_UNLIKELY(rows.isEmpty())) {
        qCritical("%s", tr("Table number %1 does not contain any rows.").arg(m_tableNo + 1).toUtf8().constData());
        return false;
    }

    bool firstRowIsHeader = false;
    QDomNodeList cols = rows.at(0).toElement().elementsByTagName(QStringLiteral("td"));
    if (cols.isEmpty()) {
        cols = rows.at(0).toElement().elementsByTagName(QStringLiteral("th"));
        if (!cols.isEmpty()) {
            firstRowIsHeader = true;
        } else {
            qCritical("%s", tr("The selected table does not contain any column.").toUtf8().constData());
            return false;
        }
    }

    if (m_colNo >= cols.size()) {
        qCritical("%s", tr("The selected table only contains %n column(s).", "", cols.size()).toUtf8().constData());
        return false;
    }

    qInfo("%s", tr("The selected table contains %n column(s).", "", cols.size()).toUtf8().constData());

    for (int i = firstRowIsHeader ? 1 : 0; i < rows.size(); ++i) {
        const QDomElement td = rows.at(i).childNodes().at(m_colNo).toElement();
        qDebug("%s", td.toElement().text().toUtf8().constData());
    }

    return true;
}



QString Process::tidyUp(const QByteArray &htmlData) const
{
    QString retVal;

    const char *htmlDataRaw = htmlData.constData();
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    int rc = -1;
    Bool ok;

    TidyDoc tdoc = tidyCreate();

    ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);

    if (ok) {
        rc = tidySetErrorBuffer(tdoc, &errbuf);
    }

    if (rc >= 0) {
        rc = tidyParseString(tdoc, htmlDataRaw);
    }

    if (rc > 0) {
        rc = tidyCleanAndRepair(tdoc);
    }

    if (rc > 1) {
        rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
    }

    if (rc >= 0) {
        rc = tidySaveBuffer(tdoc, &output);
    }

    if (rc >= 0) {
        retVal = (char*) output.bp;
    }

    tidyBufFree(&output);
    tidyBufFree(&errbuf);
    tidyRelease(tdoc);

    return retVal;
}
