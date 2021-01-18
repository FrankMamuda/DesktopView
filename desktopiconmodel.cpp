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
#ifdef Q_OS_WIN
#include <QtWin>
#include <Windows.h>
#include <CommCtrl.h>
#include <commoncontrols.h>
#include <shellapi.h>
#include <WinUser.h>
#include <QStandardPaths>
#endif

/**
 * @brief DesktopIconModel::DesktopIconModel
 * @param parent
 */
DesktopIconModel::DesktopIconModel( QObject *parent ) : QAbstractListModel( parent ) {
    const QDir dir( "cache/" );
    if ( !dir.exists())
        dir.mkpath( "." );
}

/**
 * @brief DesktopIconModel::setScale
 * @param scale
 */
void DesktopIconModel::setScale( int scale ) {
    this->beginResetModel();
    this->iconCache.clear();
    this->m_scale = scale;
    this->endResetModel();
}

/**
 * @brief DesktopIconModel::rowCount
 * @return
 */
int DesktopIconModel::rowCount( const QModelIndex & ) const {
    return 3;
}

/**
 * @brief DesktopIconModel::data
 * @param index
 * @param role
 * @return
 */
QVariant DesktopIconModel::data( const QModelIndex &index, int role ) const {
    if ( role == Qt::DisplayRole )
        return this->fileName( index );

    if ( role == Qt::DecorationRole )
        return this->fileIcon( index );

    return QVariant();
}

/**
 * @brief DesktopIconModel::fileName
 * @return
 */
QString DesktopIconModel::fileName( const QModelIndex &index ) const {
    switch ( index.row()) {
    case 0:
        return DesktopIconModel::tr( "This PC" );

    case 1:
        return DesktopIconModel::tr( "Trash" );

    case 2:
        return DesktopIconModel::tr( "Documents" );
    }

    return QString();
}

/**
 * @brief DesktopIconModel::filePath
 * @return
 */
QString DesktopIconModel::filePath( const QModelIndex &index ) const {
    switch ( index.row()) {
    case 0:
        return DesktopIcons::PCID;// "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";

    case 1:
        return DesktopIcons::TrashID;//"::{645FF040-5081-101B-9F08-00AA002F954E}";

    case 2:
        return QStandardPaths::standardLocations( QStandardPaths::DocumentsLocation ).first();// DesktopIcons::DocsID;//"::{645FF040-5081-101B-9F08-00AA002F954E}";
    }

    return QString();
}

/**
 * @brief DesktopIconModel::mimeTypeName
 * @return
 */
QString DesktopIconModel::mimeTypeName( const QModelIndex &index ) const {
    switch ( index.row()) {
    case 0:
        return "-100";// "x-windows-thispc";

    case 1:
        return "-80";//"x-windows-trash";

    case 2:
        return "-90";// "x-windows-thispc";
    }

    return QString();
}

/**
 * @brief DesktopIconModel::fileIcon
 * @return
 */
QIcon DesktopIconModel::fileIcon( const QModelIndex &index ) const {
    int iconId = -1;

    switch ( index.row()) {
    case 0:
        iconId = 16;
        break;

    case 1:
        iconId = 32;
        break;

    case 2:
        iconId = 267;
        break;
    }

    if ( this->iconCache.contains( iconId ))
        return this->iconCache[iconId];

    QPixmap pixmap;
    if ( pixmap.load( QString( "cache/%1_%2.png" ).arg( QString::number( iconId ), QString::number( this->scale()))))
        return QIcon( pixmap );

    return this->getIcon( iconId );
}

/**
 * @brief DesktopIconModel::getIconPixmap
 * @return
 */
QPixmap DesktopIconModel::getIconPixmap( int iconId, int scale ) {
    return DesktopIconModel::loadPixmapFromLibrary( iconId, scale );
}

/**
 * @brief DesktopIconModel::loadPixmapFromLibrary
 * @param resourceId
 * @param scale
 * @param name
 * @return
 */
QPixmap DesktopIconModel::loadPixmapFromLibrary( int resourceId, int scale, const QString &name )  {
    auto loadLibrary = []( const wchar_t *libraryName )  {
        QVarLengthArray<wchar_t, MAX_PATH> fullPath;

        UINT retLen = ::GetSystemDirectory( fullPath.data(), MAX_PATH );
        if ( retLen > MAX_PATH ) {
            fullPath.resize( static_cast<int>( retLen ));
            retLen = ::GetSystemDirectory( fullPath.data(), retLen );
        }

        const QString systemDirectory( QString::fromWCharArray( fullPath.constData(), static_cast<int>( retLen )));
        HINSTANCE inst = nullptr;
        if ( !systemDirectory.isEmpty()) {
            const QString fileName( QString::fromWCharArray( libraryName ).append( QLatin1String(".dll" )));
            const QString absolutePath( systemDirectory + ( !systemDirectory.endsWith( "\\" ) ? "\\" : "" ) + fileName );

            inst = ::LoadLibrary( reinterpret_cast<const wchar_t *>( absolutePath.utf16()));
            if ( inst != nullptr )
                return inst;
        }

        return inst;
    };

    if ( const HMODULE hmod = loadLibrary( reinterpret_cast<const wchar_t*>( QDir::toNativeSeparators( name ).utf16()))) {
        const HICON hIcon = static_cast<HICON>( LoadImage( hmod, MAKEINTRESOURCE( resourceId ), IMAGE_ICON, scale, scale, 0 ));
        if ( hIcon != nullptr ) {
            const QPixmap pixmap( QtWin::fromHICON( hIcon ));
            DestroyIcon( hIcon );
            return pixmap;
        }
    }
    return QPixmap();
}

/**
 * @brief DesktopIconModel::size
 * @param index
 * @return
 */
qint64 DesktopIconModel::size( const QModelIndex &index ) const {
    switch ( index.row()) {
    case 0:
        return -100;

    case 1:
        return -80;

    case 2:
        return -90;
    }

    return 0;
}

/**
 * @brief DesktopIconModel::getIcon
 * @param info
 * @return
 */
QIcon DesktopIconModel::getIcon( int iconId ) const {
    const QPixmap pixmap( DesktopIconModel::getIconPixmap( iconId, this->scale()));
    const QIcon icon( pixmap );
    this->iconCache[iconId] = icon;

    if ( !icon.isNull()) {
        qDebug() << "DesktopIconModel: write cache";
        pixmap.save( QString( "cache/%1_%2.png" ).arg( QString::number( iconId ), QString::number( this->scale())));
    }

    return icon;
}
