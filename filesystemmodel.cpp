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
#include "filesystemmodel.h"
#include <QDebug>
#include <QMimeDatabase>
#ifdef Q_OS_WIN
#include <QtWin>
#include <Windows.h>
#include <CommCtrl.h>
#include <commoncontrols.h>
#include <shellapi.h>
#include <WinUser.h>
#endif
#include <QOperatingSystemVersion>

/**
 * @brief FileSystemModel::FileSystemModel
 * @param parent
 */
FileSystemModel::FileSystemModel( const QString &path, QObject *parent ) : QFileSystemModel( parent ) {
    this->setRootPath( path );

    const QDir dir( "cache/" );
    if ( !dir.exists())
        dir.mkpath( "." );
}

/**
 * @brief FileSystemModel::fileIcon
 * @param index
 * @return
 */
QIcon FileSystemModel::fileIcon( const QModelIndex &index ) const {
    const QFileInfo info( this->fileInfo( index ));
    const auto identifier( qMakePair( info.absoluteFilePath(), info.isSymLink() ? info.symLinkTarget().size() : info.size()));
    if ( this->iconCache.contains( identifier ))
        return this->iconCache[identifier];

    QPixmap pixmap;
    if ( pixmap.load( QString( "cache/%1_%2_%3.png" ).arg( QString( QByteArray( info.absoluteFilePath().toUtf8().constData()).toBase64())).arg( info.size()).arg( QString::number( this->scale()))))
        return QIcon( pixmap );

    const QIcon icon( FileSystemModel::getIcon( info ));
    if ( icon.isNull())
        return QFileSystemModel::fileIcon( index );

    return icon;
}

/**
 * @brief FileSystemModel::index
 * @param row
 * @param column
 * @param parent
 * @return
 */
QModelIndex FileSystemModel::index( int row, int column, const QModelIndex & ) const {
    return QFileSystemModel::index( row, column, QFileSystemModel::index( this->rootPath()));
}

/**
 * @brief FileSystemModel::getIconPixmap
 * @param info
 * @param size
 * @return
 */
QPixmap FileSystemModel::getIconPixmap( const QFileInfo &info, int scale ) {
    SHFILEINFO fileInfo;
    QPixmap pixmap, cache;
    QImage image;
    int flags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_LARGEICON;
    int y, k;
    bool ok = false;

    memset( &fileInfo, 0, sizeof( SHFILEINFO ));

    if ( !info.isDir())
        flags |= SHGFI_USEFILEATTRIBUTES;

    // win32 icon cache
    const QMimeDatabase db;
    const QMimeType mime( db.mimeTypeForFile( info.absoluteFilePath(), QMimeDatabase::MatchContent ));
    const int hrFileInfo = static_cast<const int>( SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( info.absoluteFilePath()).utf16()), 0, &fileInfo, sizeof( SHFILEINFO ), static_cast<UINT>( flags )));

    // for some reason this always fails on msvc
#ifndef Q_CC_MSVC
    if ( static_cast<const int>( hrFileInfo ) >= 0 )
#else
    Q_UNUSED( hrFileInfo )
#endif
    {
        if ( QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows7 && fileInfo.hIcon ) {
            /**
             * @brief pixmapFromImageList
             * @param index
             * @param pixmap
             */
            auto pixmapFromImageList = [ fileInfo ]( int index, QPixmap &pixmap ) {
                IImageList *imageList = nullptr;
                HICON hIcon = nullptr;

                if ( static_cast<int>( SHGetImageList( index, IID_PPV_ARGS( &imageList ))) >= 0 ) {
                    if ( static_cast<int>( imageList->GetIcon( fileInfo.iIcon, ILD_TRANSPARENT, &hIcon )) >=0 ) {
                        pixmap = QtWin::fromHICON( hIcon );
                        DestroyIcon( hIcon );
                        imageList->Release();
                    }
                }
            };

            // first try to get the jumbo icon
            pixmapFromImageList( 0x4, pixmap );

            // test if most of the image is blank
            // (invalid jumbo with 48x48 on top left)
            if ( pixmap.width() >= 64 && pixmap.height() >= 64 ) {
                image = pixmap.toImage();
                for ( y = 64; y < pixmap.width(); y++ ) {
                    for ( k = 64; k < pixmap.height(); k++ ) {
                        if ( image.pixelColor( y, k ).alphaF() > 0.0 )
                            ok = true;
                    }
                }
            }

            // check if icon is really a small, but centered one
            if ( ok ) {
                enum Sides { Top = 0, Bottom, Right, Left };
                auto checkSide = [ image ]( Sides side, int scale ) {
                    if ( image.width() != 256 || image.height() != 256 )
                        return false;

                    if ( scale <= 8 || scale >= 128 )
                        return false;

                    int hOffset = 0, vOffset = 0;
                    int hBound = image.width();
                    int vBound = image.height();

                    switch ( side ) {
                    case Top:
                        vBound = scale;
                        break;

                    case Bottom:
                        vOffset = 256 - scale;
                        break;

                    case Left:
                        hBound = scale;
                        break;

                    case Right:
                        hOffset = 256 - scale;
                        break;
                    }

                    bool blank = true;
                    for ( int x = hOffset; x < hBound; x++ ) {
                        for ( int y = vOffset; y < vBound; y++ ) {
                            if ( image.pixelColor( x, y ).alphaF() > 0.0 )
                                blank = false;
                        }
                    }
                    return blank;
                };

                const int cropScale = 64;
                if ( checkSide( Top, cropScale ) && checkSide( Bottom, cropScale ) && checkSide( Left, cropScale ) && checkSide( Right, cropScale ))
                    ok = false;
            }

            // then try to get the large icon
            if ( pixmap.isNull() || !ok )
                pixmapFromImageList( 0x2, pixmap );
        }

        // if everything fails, get icon the old way
        if ( pixmap.isNull() && fileInfo.hIcon != nullptr ) {
            pixmap = QtWin::fromHICON( fileInfo.hIcon );
            DestroyIcon( fileInfo.hIcon );
        }
    }

    /**
     * @brief downscale
     * @param pixmap
     * @param scale
     * @return
     */
    auto downscale = []( const QPixmap &pixmap, int scale ) {
        QPixmap downScaled( pixmap );

        if ( pixmap.isNull() || scale <= 0 )
            return QPixmap();

        if ( downScaled.width() >= scale * 2 )
            downScaled = downScaled.scaled( scale * 2, scale * 2, Qt::IgnoreAspectRatio, Qt::FastTransformation );

        return downScaled.scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    };

    pixmap = downscale( pixmap, scale );
    return pixmap;
}

/**
 * @brief FileSystemModel::getIcon
 * @param info
 * @return
 */
QIcon FileSystemModel::getIcon( const QFileInfo &info ) const {
    const QPixmap pixmap( FileSystemModel::getIconPixmap( info, this->scale()));
    const QIcon icon( pixmap );
    this->iconCache[qMakePair( info.absoluteFilePath(), info.isSymLink() ? info.symLinkTarget().size() : info.size())] = icon;

    if ( !icon.isNull()) {
        qDebug() << "FileSystemModel: write cache";
        pixmap.save( QString( "cache/%1_%2_%3.png" ).arg( QString( QByteArray( info.absoluteFilePath().toUtf8().constData()).toBase64())).arg( info.size()).arg( QString::number( this->scale())));
    }

    return icon;
}

/**
 * @brief FileSystemModel::rowCount
 * @return
 */
int FileSystemModel::rowCount( const QModelIndex & ) const {
    return QFileSystemModel::rowCount( QFileSystemModel::index( this->rootPath()));
}

/**
 * @brief FileSystemModel::data
 * @param index
 * @param role
 * @return
 */
QVariant FileSystemModel::data( const QModelIndex &index, int role ) const {
    if ( role == Qt::DisplayRole ) {
        const QString fileName( QFileSystemModel::data( index, Qt::DisplayRole ).toString());
        return ( fileName.endsWith( ".lnk" )) ? QString( fileName ).left( fileName.length() - 4 ) : fileName;
    }

    if ( role == Qt::DecorationRole )
        return this->fileIcon( index );

    return QFileSystemModel::data( index, role );
}

/**
 * @brief FileSystemModel::mimeTypeName
 * @param index
 * @return
 */
QString FileSystemModel::mimeTypeName( const QModelIndex &index ) const {
    return QMimeDatabase().mimeTypeForFile( this->filePath( index )/*, QMimeDatabase::MatchContent*/ ).name();
}

/**
 * @brief FileSystemModel::setScale
 * @param scale
 */
void FileSystemModel::setScale( int scale ) {
    this->beginResetModel();
    this->iconCache.clear();
    this->m_scale = scale;
    this->endResetModel();
}
