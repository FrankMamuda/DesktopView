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
#include <QListView>
#include "itemdelegate.h"
#include <QMainWindow>
#include <Windows.h>

/**
 * @brief The IconView class
 */
class IconView : public QListView {
    Q_OBJECT

public:
    explicit IconView( QWidget *parent = nullptr );
    QSize internalGridSize() const { return this->m_internalGridSize; }
    HWND parentHWND;
    QMainWindow *windowParent = nullptr;
    [[nodiscard]] QString getFilePath( const QModelIndex &index ) const;

public slots:
    void savePositions();
    void restorePositions();
    void setInternalGridSize( const QSize &size ) { this->m_internalGridSize = size; }

protected:
    void dropEvent( QDropEvent *event ) override;
    void showEvent( QShowEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

private:
    ItemDelegate *delegate = new ItemDelegate( this );
    QSize m_internalGridSize = QSize( 128, 96 );
};
