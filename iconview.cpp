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
#include "backgrounddialog.h"
#include "desktopiconmodel.h"
#include "iconview.h"
#include "mainwindow.h"
#include "multidirmodel.h"
#include "sortmodel.h"
#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QApplication>
#include <QDropEvent>
#include <QMenu>
#include <QDesktopServices>
#include <QSortFilterProxyModel>
#include <QtWin>
#include <QInputDialog>
#include <QLineEdit>
#include <QScreen>
#ifdef Q_OS_WIN
#include <QSettings>
#include <ShlObj.h>
#endif

/**
 * @brief IconView::IconView
 * @param parent
 */
IconView::IconView( QWidget *parent ) : QListView( parent ) {
    this->setItemDelegate( this->delegate );
    this->setAutoFillBackground( false );
    this->setMovement( QSettings().value( "snap", true ).toBool() ? QListView::Snap : QListView::Free );
    //this->setViewMode( QSettings().value( "iconMode", true ).toBool() ? QListView::IconMode : QListView::ListMode );
    const bool iconMode = QSettings().value( "iconMode", true ).toBool();
    if ( iconMode ) {
        this->setViewMode( QListView::IconMode );
        this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    } else {
        this->setViewMode( QListView::ListMode );
        this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    }

    // set it as a background brush
    QPalette p( this->palette());
    p.setBrush( QPalette::Base, Qt::transparent );
    this->setPalette( p );

    // FIXME: double click only works
    IconView::connect( this, &QAbstractItemView::activated, [ this ]( const QModelIndex &index ) {
        if ( index.isValid()) {
            const QString path( this->getFilePath( index ));

            if ( !path.isEmpty())
                QDesktopServices::openUrl( QUrl::fromLocalFile( path ));
        }
    } );

    const int size = QSettings().value( "icons/size", 48 ).toInt();
    this->setIconSize( QSize( size, size ));


    // TODO: can have a scroll bar (vertical)


}

/**
 * @brief IconView::getFilePath
 * @param index
 * @return
 */
QString IconView::getFilePath( const QModelIndex &index ) const {
    if ( index.isValid()) {
        const QSortFilterProxyModel *proxyModel( qobject_cast<const QSortFilterProxyModel *>( index.model()));
        const MultiDirModel *dirModel( qobject_cast<const MultiDirModel *>( proxyModel->sourceModel()));

        if ( dirModel != nullptr ) {
            const QModelIndex sourceIndex( dirModel->mapToSource( proxyModel->mapToSource( index )));
            const FileSystemModel *model( qobject_cast<const FileSystemModel*>( sourceIndex.model()));

            if ( model != nullptr )
                return model->filePath( sourceIndex );

            const DesktopIconModel *desktopIconModel( qobject_cast<const DesktopIconModel*>( sourceIndex.model()));
            if ( desktopIconModel != nullptr )
                return desktopIconModel->filePath( sourceIndex );
        }
    }

    return QString();
}

/**
 * @brief IconView::savePositions
 */
void IconView::savePositions() {
    if ( this->movement() == Static || this->viewMode() == QListView::ListMode )
        return;

    qDebug() << "SAVE";


    const QSortFilterProxyModel *proxyModel( qobject_cast<const QSortFilterProxyModel *>( this->model()));
    const MultiDirModel *model( qobject_cast<MultiDirModel*>( proxyModel->sourceModel()));
    QFile file( "positions.dat" );
    if ( file.open( QIODevice::WriteOnly )) {
        QDataStream out( &file );

        for ( int y = 0; y < model->rowCount(); y++ ) {
            if ( model == nullptr )
                continue;

            const QModelIndex proxyIndex( proxyModel->index( y, 0 ));
            const QModelIndex index( proxyModel->mapToSource( proxyIndex ));
            const QPoint position( this->rectForIndex( proxyIndex ).topLeft());

            out << model->filePath( index );
            out << position;
        }

        file.close();
    }
}

/**
 * @brief IconView::restorePositions
 */
void IconView::restorePositions() {
    if ( this->movement() == Static || this->viewMode() == QListView::ListMode )
        return;

    qDebug() << "RESTORE";

    const QSortFilterProxyModel *proxyModel( qobject_cast<const QSortFilterProxyModel *>( this->model()));
    const MultiDirModel *model( qobject_cast<MultiDirModel*>( proxyModel->sourceModel()));
    QMap<QString, QPoint> positions;

    QFile file( "positions.dat" );
    if ( file.open( QIODevice::ReadOnly )) {
        QDataStream in( &file );

        while ( !in.atEnd()) {
            QString fileName;
            QPoint position;

            in >> fileName >> position;
            positions[fileName] = position;
        }

        file.close();
    }

    // first pass
    for ( int y = 0; y < model->rowCount(); y++ ) {
        if ( model == nullptr )
            continue;

        const QModelIndex index( model->index( y, 0 ));
        const QString fileName( model->filePath( index ));

        if ( positions.contains( fileName ))
            this->setPositionForIndex( positions[fileName], proxyModel->mapFromSource( index ));
    }

    // second pass
    // DETECT OVERLAPS
    // might be simpler to create internal grid and map items to grid?
    // but that wont work in free placement mode
    for ( int y = 0; y < model->rowCount(); y++ ) {
        if ( model == nullptr )
            continue;

        const QModelIndex index( model->index( y, 0 ));
        const QString fileName( model->filePath( index ));


        const QPoint pos( positions.contains( fileName ) ? positions[fileName] : this->visualRect( index ).topLeft());

        const QModelIndex under( this->indexAt( pos ));
        if ( under.isValid() && QString::compare( fileName, model->filePath( under ))) {
            qDebug() << "overlap" << model->fileName( index ) << model->fileName( under ) << pos << this->rectForIndex( under ).topLeft();

            // find a spot to place the icon
            int posx = 0;
            int posy = 0;
            while ( 1 ) {
                if ( this->indexAt( QPoint( posx, posy )).isValid()) {
                    posx += this->internalGridSize().width();

                    if ( posx > qApp->primaryScreen()->geometry().width() - this->internalGridSize().width()) {
                        posx = 0;
                        posy += this->internalGridSize().height();
                    }

                    continue;
                }

                if ( posy > qApp->primaryScreen()->geometry().height()) {
                    qDebug() << "could not find a spot";
                    break;
                }

                qDebug() << "found a spot at" << posx << posy;
                this->setPositionForIndex( QPoint( posx, posy ), proxyModel->mapFromSource( index ));
                break;
            }
        }
    }
}

/**
 * @brief IconView::dropEvent
 * @param event
 */
void IconView::dropEvent( QDropEvent *event ) {
    QListView::dropEvent( event );

    if ( this->movement() == QListView::Snap ) {
        const QModelIndexList list( this->selectedIndexes());

        //const QModelIndex index( this->indexAt( event->pos()));

        for ( const QModelIndex &index : list ) {
            if ( index.isValid()) {
                auto round = []( int numToRound, const int multiple, bool down = false ) {
                    if ( multiple == 0 )
                        return numToRound;

                    const int remainder = numToRound % multiple;
                    if ( remainder == 0 )
                        return numToRound;

                    return numToRound + ( down ? 0 : multiple ) - remainder;
                };

                const QPoint pos( this->visualRect( index ).topLeft());
                const QPoint minPos( round( pos.x(), this->internalGridSize().width(), true ), round( pos.y(), this->internalGridSize().height(), true ));
                const QPoint maxPos( minPos.x() + this->internalGridSize().width(), minPos.y() + this->internalGridSize().height());
                this->setPositionForIndex( QPoint( maxPos.x() - pos.x() < pos.x() - minPos.x() ? maxPos.x() : minPos.x(),
                                                   maxPos.y() - pos.y() < pos.y() - minPos.y() ? maxPos.y() : minPos.y()), index );
            }
        }
    }
}

/**
 * @brief IconView::showEvent
 * @param event
 */
void IconView::showEvent( QShowEvent *event ) {
    QListView::showEvent( event );

    // can only be used here since size is set after initialization
    const QSize size( this->iconSize().width() + this->delegate->sideMargin() * 2 + 2,
                      this->delegate->topMargin() + this->iconSize().height() + this->delegate->textLineCount() * QFontMetrics( this->font()).height() + this->delegate->bottomMargin() + 2 );
    this->setInternalGridSize( size );
}

/**
 * @brief IconView::mouseReleaseEvent
 * @param event
 */
void IconView::mouseReleaseEvent( QMouseEvent *event ) {
    QListView::mouseReleaseEvent( event );

    if ( event->button() == Qt::RightButton ) {
        qDebug() << "RB";
        const QList<QModelIndex> indexes( this->selectionModel()->selectedIndexes());

        if ( indexes.isEmpty()) {
            QMenu menu( this );

            QMenu *viewMenu( menu.addMenu( IconView::tr( "View" )));

            // TODO: or move to personalize???
            //       also add spacing and grid size options
            const int iconSize = QSettings().value( "icons/size", 48 ).toInt();
            QAction *actionLargeIcons( viewMenu->addAction( IconView::tr( "Large icons" ), [ this ]() {
                QSettings().setValue( "icons/size", 64 );
                this->setIconSize( QSize( 64, 64 ));
            } ));
            actionLargeIcons->setCheckable( true );
            actionLargeIcons->setChecked( iconSize == 64 );

            QAction *actionMediumIcons( viewMenu->addAction( IconView::tr( "Medium icons" ), [ this ]() {
                QSettings().setValue( "icons/size", 48 );
                this->setIconSize( QSize( 48, 48 ));
            } ));
            actionMediumIcons->setCheckable( true );
            actionMediumIcons->setChecked( iconSize == 48 );

            QAction *actionSmallIcons( viewMenu->addAction( IconView::tr( "Small icons" ), [ this ]() {
                QSettings().setValue( "icons/size", 32 );
                this->setIconSize( QSize( 32, 32 ));
            } ));
            actionSmallIcons->setCheckable( true );
            actionSmallIcons->setChecked( iconSize == 32 );

            viewMenu->addSeparator();

            QAction *actionIconMode( viewMenu->addAction( IconView::tr( "Icon mode" ), [ this ]() {
                // TODO: can have a scroll bar (vertical)

                QSettings().setValue( "iconMode", true );
                this->setViewMode( QListView::IconMode );
                this->delegate->clearCache();
                this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            } ));
            actionIconMode->setCheckable( true );
            actionIconMode->setChecked( QSettings().value( "iconMode", true ).toBool());

            QAction *actionListMode( viewMenu->addAction( IconView::tr( "List mode" ), [ this ]() {
                QSettings().setValue( "iconMode", false );
                this->setViewMode( QListView::ListMode );
                this->delegate->clearCache();
                this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
            } ));
            actionListMode->setCheckable( true );
            actionListMode->setChecked( !QSettings().value( "iconMode", true ).toBool());

            viewMenu->addSeparator();

            // TODO: if not list mode
            QAction *actionSnap( viewMenu->addAction( IconView::tr( "Align icons to grid" ), [ this ]( bool checked ) {
                this->savePositions();
                this->setMovement( checked ? QListView::Snap : QListView::Free );
                QSettings().setValue( "snap", checked );

                if ( !checked )
                    this->restorePositions();
            } ));
            actionSnap->setCheckable( true );
            actionSnap->setChecked( QSettings().value( "snap", true ).toBool());


            auto sort = [ this ]( SortModel::SortMode mode ) {
                //const Movement movement = this->movement();
                //this->setMovement( Static );

                SortModel *sortModel( qobject_cast<SortModel *>( this->model()));
                sortModel->setSortMode( mode );
                sortModel->sort( 0, Qt::DescendingOrder );
                sortModel->sort( 0, Qt::AscendingOrder );


                //this->setMovement( movement );
            };

            QMenu *sortMenu( menu.addMenu( IconView::tr( "Sort by" )));
            sortMenu->addAction( IconView::tr( "Name" ), [ sort ]() {
                sort( SortModel::Name );
            } );
            sortMenu->addAction( IconView::tr( "Size" ), [ sort ]() {
                sort( SortModel::Size );
            } );
            sortMenu->addAction( IconView::tr( "Item type" ), [ sort ]() {
                sort( SortModel::Type );
            } );
            sortMenu->addAction( IconView::tr( "Date modified" ), [ sort ]() {
                sort( SortModel::Date );
            } );

            QMenu *iconsMenu( menu.addMenu( IconView::tr( "Icons" )));
            QAction *iconPC( iconsMenu->addAction( IconView::tr( "This PC" ), [ this ]() {
                Q_UNUSED( this )
            } ));
            iconPC->setCheckable( true );
            iconPC->setChecked( QSettings().value( "icons/pc", true ).toBool());

            QAction *iconDocuments( iconsMenu->addAction( IconView::tr( "Documents" ), [ this ]() {
                Q_UNUSED( this )
            } ));
            iconDocuments->setCheckable( true );
            iconDocuments->setChecked( QSettings().value( "icons/documents", false ).toBool());

            QAction *iconTrash( iconsMenu->addAction( IconView::tr( "Trash" ), [ this ]() {
                Q_UNUSED( this )
            } ));
            iconTrash->setCheckable( true );
            iconTrash->setChecked( QSettings().value( "icons/trash", true ).toBool());

            QAction *iconDesktops( iconsMenu->addAction( IconView::tr( "Virtual desktops" ), [ this ]() {
                Q_UNUSED( this )
            } ));
            iconDesktops->setCheckable( true );
            iconDesktops->setChecked( QSettings().value( "icons/desktops", false ).toBool());

            menu.addSeparator();

            QMenu *desktopMenu( menu.addMenu( IconView::tr( "Switch desktop" )));
            desktopMenu->addAction( IconView::tr( "Desktop 1" ), [ this ]() {
                Q_UNUSED( this )
            } );
            desktopMenu->addAction( IconView::tr( "Desktop 2" ), [ this ]() {
                Q_UNUSED( this )
            } );
            menu.addAction( IconView::tr( "Rename desktop" ), [ this ]() {
                bool ok;
                const QString text( QInputDialog::getText( this, IconView::tr( "Rename virtual desktop" ),
                                                           IconView::tr( "Desktop name:" ), QLineEdit::Normal,
                                                           QSettings().value( "desktop/name", "Desktop 1" ).toString(), &ok ));
                if ( ok && !text.isEmpty())
                    QSettings().setValue( "desktop/name", text );
            } );

            menu.addAction( IconView::tr( "Refresh" ), [ this ]() {
                Q_UNUSED( this )
            } );

            menu.addSeparator();

            menu.addAction( IconView::tr( "Paste" ), [ this ]() {
                Q_UNUSED( this )
            } );

            menu.addAction( IconView::tr( "Paste shortcut" ), [ this ]() {
                Q_UNUSED( this )
            } );

            menu.addSeparator();

            // QPixmap DesktopIconModel::loadPixmapFromShell32( int resourceId, int scale )  {

            menu.addAction( DesktopIconModel::loadPixmapFromLibrary( 151, 16, "imageres" ), IconView::tr( "Personalize" ), [ this ]() {
                BackgroundDialog ( this ).exec();
                //if ( bd.exec() == QDialog::Accepted )
                //    qobject_cast<MainWindow*>( this->windowParent )->updateWallpaper( static_cast<MainWindow::UpdateType>( bd.currentIndex()));
            } );

             //for ( int x = 256; x < 512; x++ )
             //    menu.addAction( DesktopIconModel::loadPixmapFromLibrary( x, 16 , "imageres" ), "A" );

            menu.addSeparator();

            menu.addAction( IconView::tr( "New folder" ), [ this ]() {
                bool ok;
                const QString text( QInputDialog::getText( this, IconView::tr( "New folder" ),
                                                           IconView::tr( "Name:" ), QLineEdit::Normal,
                                                           IconView::tr( "Folder" ), &ok ));
                if ( ok && !text.isEmpty()) {
                    const QString path( QStandardPaths::standardLocations( QStandardPaths::DesktopLocation ).first() + "/" + text );
                    const QDir dir( path );
                    if ( !dir.exists()) {
                        this->savePositions();
                        dir.mkpath( "." );
                    }

                    // QSettings()

                    //this->restorePositions();
                    //const QSortFilterProxyModel *proxyModel( qobject_cast<const QSortFilterProxyModel *>( this->model()));
                    //MultiDirModel *model( qobject_cast<MultiDirModel*>( proxyModel->sourceModel()));
                    //model->reset();
                }
            } );

            menu.addSeparator();

            menu.addAction( "Exit", []() {
                qApp->quit();
            } );


            menu.exec( event->pos());
            return;
        }

        QStringList filePaths;
        for ( const QModelIndex &index : indexes )
            filePaths << this->getFilePath( index );

        filePaths.removeDuplicates();
        filePaths.removeAll( "" );

        if ( filePaths.count() > 1 && ( filePaths.contains( DesktopIcons::PCID ) || filePaths.contains( DesktopIcons::TrashID ))) {
            filePaths.removeAll( DesktopIcons::PCID );
            filePaths.removeAll( DesktopIcons::TrashID );
        }

        if ( filePaths.isEmpty())
            return;

        auto getItem = []( const QString &filePath ) {
            ITEMIDLIST *itemIdList = nullptr;
            SHParseDisplayName( reinterpret_cast<const wchar_t*>( QDir::toNativeSeparators( filePath ).utf16()), nullptr, &itemIdList, 0, nullptr );
            return itemIdList;
        };

        QList<ITEMIDLIST*> idList;
        for ( const QString &path : filePaths ) {
            ITEMIDLIST *id = getItem( path );
            if ( id != nullptr )
                idList << id;
        }

        LPCITEMIDLIST *list = new LPCITEMIDLIST[static_cast<unsigned long long>( idList.count())];
        for ( int y = 0; y < idList.count(); y++ )
            list[y] = idList.at( y );


        IShellFolder *desktop;
        SHGetDesktopFolder( &desktop );

        IContextMenu *menu;
        HMENU popupMenu = CreatePopupMenu();

        if ( !filePaths.contains( DesktopIcons::PCID ) && !filePaths.contains( DesktopIcons::TrashID )) {
            DEFCONTEXTMENU cm;
            memset( &cm, 0, sizeof( DEFCONTEXTMENU ));

            cm.hwnd = this->parentHWND;
            cm.psf =  desktop;
            cm.cidl = static_cast<unsigned int>( idList.count());
            cm.apidl = list;

            if ( FAILED( SHCreateDefaultContextMenu( &cm, IID_IContextMenu, reinterpret_cast<void**>( &menu ))))
                return;
        } else {
            if ( FAILED( desktop->GetUIObjectOf( this->parentHWND, 1, list, IID_IContextMenu, nullptr, reinterpret_cast<void**>( &menu ))))
                return;
        }

        if ( SUCCEEDED( menu->QueryContextMenu( popupMenu, 0, 0, SHRT_MAX, CMF_EXPLORE | CMF_CANRENAME ))) {
            CMINVOKECOMMANDINFO info;
            memset( &info, 0, sizeof( CMINVOKECOMMANDINFO ));
            info.cbSize = sizeof( CMINVOKECOMMANDINFO );
            info.hwnd = this->parentHWND;
            info.nShow = SW_SHOWNORMAL;
            info.lpVerb = MAKEINTRESOURCEA( static_cast<unsigned short>( TrackPopupMenu( popupMenu, static_cast<unsigned short>( TPM_RETURNCMD ), event->pos().x(), event->pos().y(), 0, this->parentHWND, nullptr )));

            menu->InvokeCommand( &info );
        }

        DestroyMenu( popupMenu );
    }
}

// TODO: fix off screen stuff?
