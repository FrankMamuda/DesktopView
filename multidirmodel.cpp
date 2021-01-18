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

/*
 * includes
 */
#include "desktopiconmodel.h"
#include "multidirmodel.h"
#include <QDebug>

/**
 * @brief MultiDirModel::reset
 */
void MultiDirModel::reset() {
    this->beginResetModel();
    this->cache.clear();

    for ( const QAbstractItemModel *model : this->models ) {
        for ( int y = 0; y < model->rowCount(); y++ )
            this->cache << model->index( y, 0 );
    }

    this->endResetModel();
    emit this->loaded();
}

/**
 * @brief MultiDirModel::rowCount
 * @return
 */
int MultiDirModel::rowCount( const QModelIndex & ) const {
    return this->cache.count();
}

/**
 * @brief MultiDirModel::data
 * @param index
 * @param role
 * @return
 */
QVariant MultiDirModel::data( const QModelIndex &index, int role ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        if ( sourceIndex.isValid())
            return sourceIndex.data( role );
    }

    return QVariant();
}

/**
 * @brief MultiDirModel::mapToSource
 * @param index
 * @return
 */
QModelIndex MultiDirModel::mapToSource( const QModelIndex &index ) const {
    if ( this->cache.isEmpty() || !index.isValid() || index.row() < 0 || index.row() >= this->rowCount())
        return QModelIndex();

    return this->cache.at( index.row());
}

/**
 * @brief MultiDirModel::fileName
 * @param index
 * @return
 */
QString MultiDirModel::fileName( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->fileName( sourceIndex );

        const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
        if ( desktopIconModel != nullptr )
            return desktopIconModel->fileName( sourceIndex );
    }
    return QString();
}

/**
 * @brief MultiDirModel::filePath
 * @param index
 * @return
 */
QString MultiDirModel::filePath( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->filePath( sourceIndex );

        const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
        if ( desktopIconModel != nullptr )
            return desktopIconModel->filePath( sourceIndex );
    }
    return QString();
}

/**
 * @brief MultiDirModel::fileInfo
 * @param index
 * @return
 */
QFileInfo MultiDirModel::fileInfo( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->fileInfo( sourceIndex );

    }
    return QFileInfo();
}

/**
 * @brief MultiDirModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags MultiDirModel::flags( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->flags( sourceIndex );

        const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
        if ( desktopIconModel != nullptr )
            return desktopIconModel->flags( sourceIndex );
    }
    return QAbstractListModel::flags( index );
}

/**
 * @brief MultiDirModel::mimeTypeName
 * @param index
 * @return
 */
QString MultiDirModel::mimeTypeName( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->mimeTypeName( sourceIndex );

        const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
        if ( desktopIconModel != nullptr )
            return desktopIconModel->mimeTypeName( sourceIndex );
    }
    return QString();
}

/**
 * @brief MultiDirModel::size
 * @param index
 * @return
 */
qint64 MultiDirModel::size( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->size( sourceIndex );

        const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
        if ( desktopIconModel != nullptr )
            return desktopIconModel->size( sourceIndex );
    }
    return -1;
}

/**
 * @brief MultiDirModel::lastModified
 * @param index
 * @return
 */
QDateTime MultiDirModel::lastModified( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QModelIndex sourceIndex( this->mapToSource( index ));
        const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

        if ( model != nullptr )
            return model->fileInfo( sourceIndex ).lastModified();

        const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
        if ( desktopIconModel != nullptr )
            return desktopIconModel->lastModified( sourceIndex );
    }
    return QDateTime();
}
