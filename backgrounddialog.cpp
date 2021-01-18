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
#include "imagebutton.h"
#include "mainwindow.h"
#include "ui_backgrounddialog.h"
#include <QButtonGroup>
#include <QColor>
#include <QColorDialog>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QFileDialog>
#include <QBuffer>

/**
 * @brief BackgroundDialog::BackgroundDialog
 * @param parent
 */
BackgroundDialog::BackgroundDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::BackgroundDialog ) {
    this->ui->setupUi( this );

    this->setWindowIcon( DesktopIconModel::loadPixmapFromLibrary( 151, 16, "imageres" ));
    this->setWindowTitle( IconView::tr( "Personalize" ));

    QComboBox::connect( this->ui->comboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), [ this ]( int current ) {
        if ( current >=0 && current < this->ui->stackedWidget->count()) {
            this->ui->stackedWidget->setCurrentIndex( current );
        }
    } );

    this->setupColourPage();
    this->rows = 0;
    this->currentColumn = 0;
    this->setupImagePage();
}

/**
 * @brief BackgroundDialog::~BackgroundDialog
 */
BackgroundDialog::~BackgroundDialog() {
    delete this->ui;
}

/**
 * @brief BackgroundDialog::currentIndex
 * @return
 */
int BackgroundDialog::currentIndex() const {
    return this->ui->stackedWidget->currentIndex();
}

/**
 * @brief BackgroundDialog::setupColourPage
 */
void BackgroundDialog::setupColourPage() {
    QSettings settings;
    const QColor currentColour( settings.value( "currentColour", QColor::fromRgb( 128, 128, 128 )).value<QColor>());

    this->colours <<
                     QColor::fromRgb( 47, 174, 64 ) <<
                     QColor::fromRgb( 232, 17, 35 ) <<
                     QColor::fromRgb( 209, 52, 56 ) <<
                     QColor::fromRgb( 195, 0, 82 ) <<
                     QColor::fromRgb( 191, 0, 119 ) <<
                     QColor::fromRgb( 154, 0, 137 ) <<
                     QColor::fromRgb( 136, 23, 152 ) <<
                     QColor::fromRgb( 116, 77, 169 ) <<
                     QColor::fromRgb( 16, 137, 62 ) <<
                     QColor::fromRgb( 16, 124, 16 ) <<
                     QColor::fromRgb( 1, 133, 16 ) <<
                     QColor::fromRgb( 45, 125, 154 ) <<
                     QColor::fromRgb( 0, 99, 177 ) <<
                     QColor::fromRgb( 107, 105, 214 ) <<
                     QColor::fromRgb( 142, 140, 216 ) <<
                     QColor::fromRgb( 135, 100, 184 ) <<
                     QColor::fromRgb( 3, 131, 135 ) <<
                     QColor::fromRgb( 72, 104, 96 ) <<
                     QColor::fromRgb( 82, 94, 84 ) <<
                     QColor::fromRgb( 126, 115, 95 ) <<
                     QColor::fromRgb( 76, 74, 72 ) <<
                     QColor::fromRgb( 81, 92, 107 ) <<
                     QColor::fromRgb( 74, 84, 89 ) <<
                     QColor::fromRgb( 0, 0, 0 );

    QGridLayout *grid( new QGridLayout );

    grid->setMargin( 0 );
    grid->setSpacing( Ui::Spacing );

    QButtonGroup *group( new  QButtonGroup());
    group->setExclusive( true );

    if ( !this->colours.contains( currentColour ))
        this->colours << currentColour;

    auto addButton = [ this, currentColour, grid, group ]( const QColor &colour, const QColor &current ) {
        if ( this->currentColumn >= Ui::ColourColumnCount ) {
            this->currentColumn = 0;
            this->rows++;
        }

        ImageButton *colourButton( new ImageButton());
        colourButton->setFixedSize( QSize( Ui::ColourTileSize, Ui::ColourTileSize ));
        //colourButton->setStyleSheet( QString( "QPushButton { color: black; background-color: %1; border: none; }"
        //                                      "QPushButton::checked { border-image: url(:/icons/selected.png); }" ).arg( colour.name()));
        colourButton->setColour( colour );

        if ( current == colour )
            colourButton->setChecked( true );

        QPushButton::connect( colourButton, &QPushButton::toggled, [ this, colour ]( bool enabled ) {
            if ( enabled ) {
                QSettings().setValue( "currentColour", colour );
                qobject_cast<MainWindow*>( qobject_cast<IconView*>( this->parent())->windowParent )->updateWallpaper( MainWindow::SolidColour );
            }
        } );

        grid->addWidget( colourButton, this->rows, this->currentColumn++, Qt::AlignCenter );
        group->addButton( colourButton );

        this->buttonMap[colour.name()] = colourButton;

        return colourButton;
    };

    for ( const QColor &colour : this->colours )
        addButton( colour, currentColour );

    this->ui->colourFrame->setFixedSize( Ui::ColourTileSize * Ui::ColourColumnCount + ( Ui::ColourColumnCount - 1 ) * Ui::Spacing, Ui::ColourTileSize * ( this->rows + 1 ) + Ui::Spacing * this->rows );
    this->ui->colourFrame->setLayout( grid );

    QPushButton::connect( this->ui->customColourButton, &QPushButton::clicked, [ this, currentColour, addButton ]() {
        QColorDialog dialog( this );
        dialog.setCurrentColor( currentColour );

        QColorDialog::connect( &dialog, &QDialog::accepted, [ this, &dialog, addButton ]() {
            const QColor colour( dialog.currentColor());

            QSettings().setValue( "currentColour", colour );

            if ( !this->colours.contains( colour )) {
                addButton( colour, colour );
                this->ui->colourFrame->setFixedSize( Ui::ColourTileSize * Ui::ColourColumnCount + ( Ui::ColourColumnCount - 1 ) * Ui::Spacing, Ui::ColourTileSize * ( this->rows + 1 ) + Ui::Spacing * this->rows );
                qobject_cast<MainWindow*>( qobject_cast<IconView*>( this->parent())->windowParent )->updateWallpaper( MainWindow::SolidColour );
            }
        } );

        dialog.exec();
    } );
}

/**
 * @brief BackgroundDialog::setupImagePage
 */
void BackgroundDialog::setupImagePage() {
    QSettings settings;    

    const QString currentImage( settings.value( "currentImage", Ui::DefaultWallpaper ).toString());
    this->images << Ui::DefaultWallpaper;

    QGridLayout *grid( new QGridLayout );

    grid->setMargin( 0 );
    grid->setSpacing( Ui::Spacing );

    QButtonGroup *group( new  QButtonGroup());
    group->setExclusive( true );

    qDebug() << currentImage;
    if ( !this->images.contains( currentImage ))
        this->images << currentImage;

    QStringList previousImages( settings.value( "previousImages" ).toStringList());
    previousImages.removeAll( currentImage );
    previousImages.removeDuplicates();
    while ( previousImages.count() > 9 )
        previousImages.takeFirst();

    this->images << previousImages;

    auto addButton = [ this, currentImage, grid, group ]( const QString &image, const QString &current ) {
        if ( this->currentColumn >= Ui::ImageColumnCount ) {
            this->currentColumn = 0;
            this->rows++;
        }

        QPixmap pixmap;
        if ( !pixmap.load( image ))
            return;

        ImageButton *imageButton( new ImageButton());
        imageButton->setFixedSize( QSize( Ui::ImageTileSize, Ui::ImageTileSize ));
        imageButton->setPixmap( pixmap );


        if ( current == image )
            imageButton->setChecked( true );

        QPushButton::connect( imageButton, &QPushButton::toggled, [ this, image ]( bool enabled ) {
            if ( enabled ) {
                QSettings().setValue( "currentImage", image );
                qDebug() << "toggle";
                qobject_cast<MainWindow*>( qobject_cast<IconView*>( this->parent())->windowParent )->updateWallpaper( MainWindow::Image );
            }
        } );

        grid->addWidget( imageButton, this->rows, this->currentColumn++, Qt::AlignLeft | Qt::AlignTop );
        group->addButton( imageButton );

        this->buttonMap[image] = imageButton;
    };

    for ( const QString &image : this->images )
        addButton( image, currentImage );

    this->ui->imageFrame->setFixedSize( grid->columnCount() * Ui::ImageTileSize + ( grid->columnCount() - 1 ) * Ui::Spacing, Ui::ImageTileSize * grid->rowCount() + Ui::Spacing * ( grid->rowCount() - 1 ));
    this->ui->imageFrame->setLayout( grid );

    QPushButton::connect( this->ui->browseButton, &QPushButton::clicked, [ this, currentImage, addButton, grid ]() {
        const QString fileName( QFileDialog::getOpenFileName( this, BackgroundDialog::tr( "Open image" ), "", BackgroundDialog::tr( "Images (*.png *.jpg)" )));
        if ( !fileName.isEmpty()) {

            if ( !this->buttonMap.contains( fileName ))
                addButton( fileName, fileName );
            else {
                qDebug() << "button found" << fileName;
                this->buttonMap[fileName]->setChecked( true );
            }

            QSettings().setValue( "currentImage", fileName );
            QStringList previousImages( QSettings().value( "previousImages" ).toStringList());
            if ( !previousImages.contains( fileName ))
                previousImages.append( fileName );
            QSettings().setValue( "previousImages", previousImages );

            this->ui->imageFrame->setFixedSize( grid->columnCount() * Ui::ImageTileSize + ( grid->columnCount() - 1 ) * Ui::Spacing, Ui::ImageTileSize * grid->rowCount() + Ui::Spacing * ( grid->rowCount() - 1 ));
        }
    } );

    this->ui->backgroundButton->setFixedSize( 16, 16 );
    QPushButton::connect(  this->ui->backgroundButton, &QPushButton::clicked, [ this ]() {
        QColorDialog dialog( this );
        const QColor currentColour( QSettings().value( "currentColour", QColor::fromRgb( 128, 128, 128 )).value<QColor>());
        dialog.setCurrentColor( currentColour );

        QColorDialog::connect( &dialog, &QDialog::accepted, [ this, &dialog ]() {
            const QColor colour( dialog.currentColor());
            QSettings().setValue( "currentColour", colour );
            qobject_cast<MainWindow*>( qobject_cast<IconView*>( this->parent())->windowParent )->updateWallpaper( MainWindow::Image );
        } );

        dialog.exec();
    } );


    const int fillMode = QSettings().value( "fillMode", 0 ).toInt();
    this->ui->fillCombo->setCurrentIndex( fillMode );

    QComboBox::connect( this->ui->fillCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), [ this ]( int current ) {
        QSettings().setValue( "fillMode", current );
        qobject_cast<MainWindow*>( qobject_cast<IconView*>( this->parent())->windowParent )->updateWallpaper( MainWindow::Image );
    } );    
}
