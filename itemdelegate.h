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
#include <QStyledItemDelegate>
#include <QListView>
#include <QMap>

/**
 * @brief The ListItem class
 */
class ListItem {
public:
    QStringList lines;
    QList<int> lineWidths;
    int textHeight;
};
Q_DECLARE_METATYPE( ListItem )

/**
 * @brief The ItemDelegate class
 */
class ItemDelegate : public QStyledItemDelegate {
    Q_DISABLE_COPY( ItemDelegate )

public:
    explicit ItemDelegate( QListView *parent = nullptr );
    ~ItemDelegate() {}
    int textLineCount() const { return this->m_textLineCount; }
    bool isSelectionVisible() const { return this->m_selectionVisible; }
    int topMargin() const { return this->m_textLineCount; }
    int sideMargin() const { return this->m_sideMargin; }
    int textMargin() const { return this->m_textMargin; }
    int bottomMargin() const { return this->m_bottomMargin; }

public slots:
    void clearCache() { this->cache.clear(); }
    void setTextLineCount( int count ) { this->m_textLineCount = count; }
    void setSelectionVisible( bool enable ) { this->m_selectionVisible = enable; }
    void setTopMargin( int margin ) { this->m_topMargin = margin; }
    void setBottomMargin( int margin ) { this->m_bottomMargin = margin; }
    void setSideMargin( int margin ) { this->m_sideMargin = margin; }
    void setTextMargin( int margin ) { this->m_textMargin = margin; }

protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
    ListItem textItemForIndex( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    mutable QMap<QString, ListItem> cache;
    int m_textLineCount = 3;
    bool m_selectionVisible;
    int m_topMargin = 4;
    int m_bottomMargin = 4;
    int m_sideMargin = 16;
    int m_textMargin = 4;
};
