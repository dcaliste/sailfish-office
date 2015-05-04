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

#include "filtermodel.h"

#include "documentlistmodel.h"

FilterModel::FilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
  this->setFilterRole(DocumentListModel::Roles::FileNameRole);
}

FilterModel::~FilterModel()
{
}

void FilterModel::setSourceModel(DocumentListModel *model)
{
  QSortFilterProxyModel::setSourceModel(static_cast<QAbstractItemModel*>(model));
}

DocumentListModel* FilterModel::sourceModel() const
{
  return static_cast<DocumentListModel*>(QSortFilterProxyModel::sourceModel());
}
