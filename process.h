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

#ifndef PROCESS_H
#define PROCESS_H

#include <QString>
#include <QCoreApplication>

class Process
{
    Q_DECLARE_TR_FUNCTIONS(Process)
public:
    explicit Process(const QString &filePath, int tableNo, int colNo);

    bool exec() const;

private:
    QString m_filePath;
    int m_tableNo = 0;
    int m_colNo = 0;

    QString tidyUp(const QByteArray &htmlData) const;
};

#endif // PROCESS_H
