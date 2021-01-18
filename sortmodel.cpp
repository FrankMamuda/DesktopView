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
#include "desktopiconmodel.h"
#include "filesystemmodel.h"
#include "multidirmodel.h"
#include "sortmodel.h"
#include <QMimeDatabase>
#include <QDebug>
#include <QDateTime>

/**
 * @brief SortModel::lessThan
 * @param left
 * @param right
 * @return
 */
bool SortModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const {


    switch ( this->sortMode()) {
    case Name:
        return this->sortByName( left, right );

    case Type:
        return this->sortByType( left, right );

    case Size:
        return this->sortBySize( left, right );

    case Date:
        return this->sortByDate( left, right );

    default:
        ;
    }

    return QSortFilterProxyModel::lessThan( left, right );
}

/**
 * @brief SortModel::sortByName
 * @param left
 * @param right
 * @return
 */
bool SortModel::sortByName( const QModelIndex &left, const QModelIndex &right ) const {
    auto getFileName = []( const QModelIndex &index ) {
        const MultiDirModel *model( qobject_cast<const MultiDirModel*>( index.model()));
        return model != nullptr ?  model->fileName( index ) : QString();
    };

    const QString leftName( getFileName( left ));
    const QString rightName( getFileName( right ));

    return QString::localeAwareCompare( leftName, rightName ) < 0;
}

/**
 * @brief SortModel::sortByType
 * @param left
 * @param right
 * @return
 */
bool SortModel::sortByType( const QModelIndex &left, const QModelIndex &right ) const {
    auto getMimeTypeName = []( const QModelIndex &index ) {
        const MultiDirModel *model( qobject_cast<const MultiDirModel*>( index.model()));
        return model != nullptr ?  model->mimeTypeName( index ) : QString();
    };

    const QString leftName( getMimeTypeName( left ));
    const QString rightName( getMimeTypeName( right ));

    qDebug() << "less than called" << this->sortMode() << leftName << rightName << left << right;

    return QString::compare( leftName, rightName ) < 0;
}

/**
 * @brief SortModel::sortBySize
 * @param left
 * @param right
 * @return
 */
bool SortModel::sortBySize( const QModelIndex &left, const QModelIndex &right ) const {
    auto getFileSize = []( const QModelIndex &index ) {
        const MultiDirModel *model( qobject_cast<const MultiDirModel*>( index.model()));
        return model != nullptr ?  model->size( index ) : -1;
    };

    const qint64 leftSize( getFileSize( left ));
    const qint64 rightSize( getFileSize( right ));

    return leftSize < rightSize;
}

/**
 * @brief SortModel::sortByDate
 * @param left
 * @param right
 * @return
 */
bool SortModel::sortByDate( const QModelIndex &left, const QModelIndex &right ) const {
    auto getFileDate = []( const QModelIndex &index ) {
        const MultiDirModel *model( qobject_cast<const MultiDirModel*>( index.model()));
        return model != nullptr ?  model->lastModified( index ) : QDateTime();
    };

    const QDateTime leftDate( getFileDate( left ));
    const QDateTime rightDate( getFileDate( right ));

    return leftDate < rightDate;
}
