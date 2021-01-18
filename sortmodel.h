/*
 * Copyright (C) 2020 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

#pragma once

/*
 * includes
 */
#include <QSortFilterProxyModel>

/**
 * @brief The SortModel class
 */
class SortModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    SortModel( QObject *parent = nullptr ) : QSortFilterProxyModel( parent ) {}

    enum SortMode {
        NoMode = -1,
        Name,
        Type,
        Date,
        Size
    };
    Q_ENUM( SortMode )

    SortMode sortMode() const { return this->m_sortMode; }

public slots:
    void setSortMode( SortMode mode ) { this->m_sortMode = mode; }

protected:
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

private:
    SortMode m_sortMode = Name;
    bool sortByName( const QModelIndex &left, const QModelIndex &right ) const;
    bool sortByType( const QModelIndex &left, const QModelIndex &right ) const;
    bool sortBySize( const QModelIndex &left, const QModelIndex &right ) const;
    bool sortByDate( const QModelIndex &left, const QModelIndex &right ) const;
};
