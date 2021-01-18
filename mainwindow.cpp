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
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QDebug>
#include <QScreen>
#include <QSettings>
#include "ui_mainwindow.h"
#include "multidirmodel.h"
#include "desktopiconmodel.h"
#include "mainwindow.h"
#include "backgrounddialog.h"
#include "sortmodel.h"
#ifdef Q_OS_WIN
#include <QPainter>
#include <ShlObj.h>
#endif

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
    // set appropriate window flags and attributes
    this->setAttribute( Qt::WA_TranslucentBackground );
    this->setAttribute( Qt::WA_NoSystemBackground );
    this->setAttribute( Qt::WA_Hover );
    this->setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint );
    this->setWindowState( Qt::WindowMaximized );

    this->setWindowIcon( DesktopIconModel::loadPixmapFromLibrary( 151, 256, "imageres" ));
    this->setWindowTitle( IconView::tr( "Alternate desktop" ));

    // setup widgets
    this->ui->setupUi( this );

    this->ui->listView->parentHWND = reinterpret_cast<HWND>( this->winId());
    this->ui->listView->windowParent = this;

    // load background image (temporary)
    this->updateWallpaper( Image );
    //this->pixmap.load( Ui::DefaultWallpaper );
    this->setAutoFillBackground( false );

    // set it as a background brush
    //QPalette p( this->palette());
    //p.setBrush( QPalette::Base, pixmap );
    //this->setPalette( p );

    // initialize multi-directory model
    auto *model( new MultiDirModel());

    // add user desktop
    auto *desktopModel( new FileSystemModel( QStandardPaths::standardLocations( QStandardPaths::DesktopLocation ).first()));
    desktopModel->setResolveSymlinks( false );
    model->add( desktopModel );
    FileSystemModel::connect( desktopModel, &FileSystemModel::directoryLoaded, model, &MultiDirModel::reset );

    // add public desktop
    // FIXME::!!!
    auto *publicDesktopModel( new FileSystemModel( "C:/Users/Public/Desktop" ));
    publicDesktopModel->setResolveSymlinks( false );
    model->add( publicDesktopModel );
    FileSystemModel::connect( publicDesktopModel, &FileSystemModel::directoryLoaded, model, &MultiDirModel::reset );

    // add special icons (PC, documents, etc.)
#ifdef Q_OS_WIN
    auto *desktopIconModel( new DesktopIconModel());
    model->add( desktopIconModel );
#endif

    // reload model
    model->reset();

    // restore item positions
    MultiDirModel::connect( model, &MultiDirModel::loaded, this->ui->listView, &IconView::restorePositions );

    // initialize proxy model
    auto *sortModel( new SortModel());
    sortModel->setSourceModel( model );
    this->ui->listView->setModel( sortModel );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    // save item positions
    this->ui->listView->savePositions();

    // clear widgets
    delete this->ui;
}

/**
 * @brief MainWindow::updateWallpaper
 * @param type
 */
void MainWindow::updateWallpaper( MainWindow::UpdateType type ) {
    /*switch ( type ) {
    case Image:
        qDebug() << "update image" << QSettings().value( "currentImage", Ui::DefaultWallpaper ).toString();
    {
        QPixmap pixmap;
        if ( pixmap.load( QSettings().value( "currentImage", Ui::DefaultWallpaper ).toString())) {
            QPalette p( this->palette());
            p.setBrush( QPalette::Base, pixmap );
            this->setPalette( p );
        }
    }
        break;

    case SolidColour:
        qDebug() << "update colour" << QSettings().value( "currentColour", QColor::fromRgb( 128, 128, 128 )).value<QColor>();
    {
        QPalette p( this->palette());
        p.setBrush( QPalette::Base, QSettings().value( "currentColour", QColor::fromRgb( 128, 128, 128 )).value<QColor>());
        this->setPalette( p );
    }
        break;

    default:
        ;
    }*/
    if ( type == MainWindow::Image )
        this->pixmap.load( QSettings().value( "currentImage", Ui::DefaultWallpaper ).toString());
    else
        this->pixmap = QPixmap();

    this->repaint();
}

/**
 * @brief MainWindow::paintEvent
 * @param event
 */
void MainWindow::paintEvent( QPaintEvent *event ) {
    const QRect rect( qApp->primaryScreen()->geometry());
    QPainter painter( this );
    const QColor currentColour( QSettings().value( "currentColour", QColor::fromRgb( 128, 128, 128 )).value<QColor>());
    painter.fillRect( rect, currentColour );

    const QPoint center( rect.center());
    if ( !this->pixmap.isNull()) {
        const FillMode fillMode( QSettings().value( "fillMode", 0 ).value<FillMode>());
        switch ( fillMode ) {
        case Center:
            painter.drawPixmap( QRect( center.x() - this->pixmap.width() / 2, center.y() - this->pixmap.height() / 2, this->pixmap.width(), this->pixmap.height()), this->pixmap );
            break;

        case Fit:
        {
            const int height = qMin( pixmap.height(), qApp->primaryScreen()->geometry().height());
            const int width = static_cast<int>( height * static_cast<qreal>( this->pixmap.width()) / static_cast<qreal>( this->pixmap.height()));

            painter.drawPixmap( QRect( center.x() - width / 2, center.y() - height / 2, width, height ), this->pixmap );

        }
            break;

        case Tile:
            painter.drawTiledPixmap( rect, this->pixmap );
            break;

        case Stretch:
            painter.drawPixmap( rect, this->pixmap );
            break;

        case Fill:
        {

            int height = pixmap.height();
            int width = this->pixmap.width();

            if ( width < height ) {
                width = qMax( pixmap.width(), qApp->primaryScreen()->geometry().width());
                height = static_cast<int>( width * static_cast<qreal>( this->pixmap.height()) / static_cast<qreal>( this->pixmap.width()));
            } else {
                height = qMax( pixmap.height(), qApp->primaryScreen()->geometry().height());
                width = static_cast<int>( height * static_cast<qreal>( this->pixmap.width()) / static_cast<qreal>( this->pixmap.height()));
            }

            painter.drawPixmap( QRect( center.x() - width / 2, center.y() - height / 2, width, height ), this->pixmap );

        }
            break;

        default:
            ;
        }
    }

    // unnecessary
    // QMainWindow::paintEvent( event );
}
