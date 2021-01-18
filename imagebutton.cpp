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
#include "imagebutton.h"

#include <QPainter>
#include <QPainterPath>
#include <QSettings>

/**
 * @brief ImageButton::ImageButton
 * @param parent
 */
ImageButton::ImageButton( QWidget *parent ) : QPushButton( parent ) {
    this->setFixedSize( 64, 64 );
    this->setFlat( true );
    this->setCheckable( true );
    this->setStyleSheet( "border: none" );
}

/**
 * @brief ImageButton::setPixmap
 * @param pixmap
 */
void ImageButton::setPixmap( const QPixmap &pixmap ) {
    QPixmap processed( pixmap );

    // fast downscale
    if ( processed.width() > this->width() * 2 || processed.height() > this->height() * 2 )
        processed = pixmap.width() > pixmap.height() ? pixmap.scaledToWidth( this->width() * 2 ) : pixmap.scaledToHeight( this->height() * 2 );

    // smooth downscale
    processed = processed.width() > processed.height() ? processed.scaledToWidth( this->width(), Qt::SmoothTransformation ) : processed.scaledToHeight( this->height(), Qt::SmoothTransformation );

    this->pixmap = processed;
}

/**
 * @brief ImageButton::paintEvent
 * @param event
 */
void ImageButton::paintEvent( QPaintEvent *event ) {
    const QRect rect( 0, 0, this->width(), this->height());

    QPainter painter( this );

    const QColor currentColour( this->colour == Qt::transparent ? QSettings().value( "currentColour", QColor::fromRgb( 128, 128, 128 )).value<QColor>() : this->colour );
    painter.fillRect( rect, currentColour );

    const QRect pixmapRect( rect.center().x() - pixmap.width() / 2, rect.center().y() - pixmap.height() / 2, pixmap.width(), pixmap.height());
    if ( !this->pixmap.isNull())
        painter.drawPixmap( pixmapRect, this->pixmap );

    if ( this->isChecked()) {
        painter.setRenderHint( QPainter::Antialiasing, true );
        painter.setPen( { Qt::black, 6.0 } );
        painter.drawRect( rect );
        painter.fillRect( rect.width() - 24, 0, 24, 24, Qt::black );
        QPainterPath path( QPoint( rect.width() - 6, 8 ));
        path.lineTo( QPoint( rect.width() - 14, 18 ));
        path.lineTo( QPoint( rect.width() - 18, 14 ));
        painter.setPen( { Qt::white, 1.8 } );
        painter.drawPath( path );
    }

    QPushButton::paintEvent( event );
}
