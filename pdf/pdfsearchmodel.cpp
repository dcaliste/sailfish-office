/*
 * Copyright (C) 2015 Caliste Damien.
 * Contact: Damien Caliste <dcaliste@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "pdfsearchmodel.h"

#include <QRectF>

PDFSearchModel::PDFSearchModel(const QList< QPair<int, QRectF> > &matches, QObject* parent)
  : QAbstractListModel(parent), m_matches(matches)
{
}

PDFSearchModel::~PDFSearchModel()
{
}

QHash< int, QByteArray > PDFSearchModel::roleNames() const
{
    QHash< int, QByteArray > names;
    names[Page] = "page";
    names[Rect] = "rect";
    return names;
}

QVariant PDFSearchModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if(index.isValid())
    {
        int row = index.row();
        if(row > -1 && row < m_matches.count())
        {
            const QPair<int, QRectF> &match = m_matches.at(row);
            switch(role)
            {
                case Page:
                    result.setValue<int>(match.first);
                    break;
                case Rect:
                    result.setValue<QRectF>(match.second);
                    break;
                default:
                    result.setValue<QString>(QString("Unknown role: %1").arg(role));
                    break;
            }
        }
    }
    return result;
}

int PDFSearchModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;
    return m_matches.count();
}

int PDFSearchModel::count() const
{
  return m_matches.count();
}
