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
#include <QApplication>
#include "itemdelegate.h"
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

/**
 * @brief ItemDelegate::ItemDelegate
 * @param parent
 */
ItemDelegate::ItemDelegate( QListView *parent ) : m_selectionVisible( true ) {
    // set parent
    this->setParent( qobject_cast<QObject*>( parent ));
}

/**
 * @brief ItemDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize ItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    ListItem item;
    QListView *view( qobject_cast<QListView*>( this->parent()));
    QSize size( QStyledItemDelegate::sizeHint( option, index ));
    QStyleOptionViewItem customOption( option );

    // get parent listView
    if ( view == nullptr )
        return QStyledItemDelegate::sizeHint( option, index );

    // get display text
    const QString text( view->model()->data( index, Qt::DisplayRole ).toString());

    // only icon mode has custom placement
   // if ( view->viewMode() == QListView::IconMode ) {
        customOption.rect.setWidth( option.decorationSize.width() + this->sideMargin() * 2 );

        if ( this->cache.contains( text )) {
            item = this->cache[text];
        } else {
            item = this->textItemForIndex( customOption, index );
            this->cache[text] = item;
        }

        size.setWidth( customOption.rect.width());
        size.setHeight( view->viewMode() == QListView::ListMode ? ( this->topMargin() + option.decorationSize.height() + this->bottomMargin()) : ( this->topMargin() + option.decorationSize.height() + item.lines.count() * item.textHeight + this->bottomMargin()));
  //  }

    return size;
}

/**
 * @brief ItemDelegate::textItemForIndex
 * @param option
 * @param index
 * @return
 */
ListItem ItemDelegate::textItemForIndex( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QVector<QString> line( this->textLineCount());
    int y, numLines = 0;
    QListView *view( qobject_cast<QListView*>( this->parent()));
    ListItem item;

    // get parent listView
    if ( view == nullptr )
        return item;

    // get display text and height
    QString text( view->model()->data( index, Qt::DisplayRole ).toString());
    const int textHeight = option.fontMetrics.height();

    if ( view->viewMode() == QListView::IconMode ) {
        // split text into lines
        while ( this->textLineCount() - numLines ) {
            for ( y = 0; y < text.length(); y++ ) {
                if ( option.fontMetrics.horizontalAdvance( text.left( y + 1 )) > option.rect.width() - this->textMargin() * 2 )
                    break;
            }

            if ( y > 0 ) {
                if ( numLines < this->textLineCount() - 1 ) {
                    line[numLines] = text.left( y );
                    text = text.mid( y, text.length() - y );
                } else
                    line[numLines] = option.fontMetrics.elidedText( text, Qt::ElideRight, option.rect.width() - this->textMargin() * 2 );
            } else
                break;

            numLines++;
        }

        // store data as a new display item
        for ( y = 0; y < numLines; y++ ) {
            item.lines << line[y];
            item.lineWidths << option.fontMetrics.horizontalAdvance( line[y] ) + 1;
        }
    } else {
        item.lines << text;
        item.lineWidths << option.fontMetrics.horizontalAdvance( text ) + 1;
    }

    item.textHeight = textHeight;
    return item;
}

/**
 * @brief ItemDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void ItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QListView *view( qobject_cast<QListView*>( this->parent()));
    QBrush hilightBrush;

    // get parent listView
    if ( view == nullptr )
        return;

    // save painter state & get hilight brush
    painter->save();
    hilightBrush = option.palette.highlight();
    painter->setPen( Qt::NoPen );

    // mouseOver/hover item
    if (( option.state & QStyle::State_MouseOver || option.state & QStyle::State_Selected ) && this->isSelectionVisible()) {
        hilightBrush.setColor( QColor::fromRgbF( hilightBrush.color().redF(), hilightBrush.color().greenF(), hilightBrush.color().blueF(), 0.25 ));
        painter->fillRect( option.rect, hilightBrush );
    }

    // get display text
    const QString text( view->model()->data( index, Qt::DisplayRole ).toString());

    // restore painter state
    painter->restore();

    // handle icon view
    //if ( view->viewMode() == QListView::IconMode ) {
        ListItem item;
        QRect rect;
        int y;
        QTextOption to;

        // get pixmap and its dimensions
        const QIcon icon( qvariant_cast<QIcon>( view->model()->data( index, Qt::DecorationRole )));
        rect = option.rect;
        rect.setY( rect.y() + this->topMargin());
        const int width = option.decorationSize.width();
        const int height = option.decorationSize.height();

        // properly position pixmap
        if ( width < rect.width()) {
            const int offset = rect.width() - width;
            rect.setX( view->viewMode() == QListView::ListMode ? 0 : rect.x() + offset / 2 );
            rect.setWidth( width );
        }

        // draw pixmap
        rect.setHeight( height );
        const QPixmap pixmap( icon.pixmap( rect.size()));
        painter->drawPixmap( rect, pixmap );

        // split text into multiple lines
        if ( cache.contains( text )) {
            item = cache[text];
        } else {
            item = this->textItemForIndex( option, index );
            cache[text] = item;
        }

        //item = this->textItemForIndex( option, index );
        to.setAlignment( Qt::AlignHCenter );

        // init text rectangle
        rect = option.rect;
        rect.setY( view->viewMode() == QListView::ListMode ? ( rect.y() + option.rect.height() / 2 - static_cast<int>( item.textHeight * 1.5 )) : ( this->topMargin() + rect.y() + height - item.textHeight ));

        // display multi-line text
        for ( y = 0; y < item.lines.count(); y++ ) {
            rect.setX( view->viewMode() == QListView::ListMode ? option.decorationSize.width() + this->textMargin() : ( rect.x() + ( rect.width() - item.lineWidths.at( y )) / 2 ));
            rect.setY( rect.y() + item.textHeight );
            rect.setHeight( item.textHeight );
            rect.setWidth( item.lineWidths.at( y ));

            painter->save();
            painter->setPen( QColor( 0, 0, 0, 196 ));
            painter->drawText( rect.adjusted( 1, 1, 1, 1 ), item.lines.at( y ), to );
            painter->setPen( QColor( 0, 0, 0, 128 ));
            painter->drawText( rect.adjusted( 2, 2, 2, 2 ), item.lines.at( y ), to );
            painter->setPen( Qt::white );
            painter->drawText( rect, item.lines.at( y ), to );
            painter->restore();
        }
    /*} else {
        QStyleOptionViewItem optionNoSelection( option );
        QStyle::State state;

        // remove hover/selection flags
        state = option.state;
        state = state & ( ~QStyle::State_MouseOver );
        state = state & ( ~QStyle::State_Selected );
        state = state & ( ~QStyle::State_HasFocus );
        state = state & ( ~QStyle::State_Active );

        optionNoSelection.state = state;

        // paint it exactly the same as before, yet ignoring selections
        QStyledItemDelegate::paint( painter, optionNoSelection, index );
    }*/
}
