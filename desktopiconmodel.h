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
#include "filesystemmodel.h"
#include <QFileInfo>
#include <QIcon>
#include <QTime>

/**
 * @brief The DesktopIcons namespace
 */
namespace DesktopIcons {
[[maybe_unused]] constexpr static const char *PCID = "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";
[[maybe_unused]] constexpr static const char *TrashID = "::{645FF040-5081-101B-9F08-00AA002F954E}";
};

/**
 * @brief The DesktopIconModel class
 */
class DesktopIconModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit DesktopIconModel( QObject *parent = nullptr );
    ~DesktopIconModel() override = default;

    int columnCount( const QModelIndex & ) const override { return 1; }
    int rowCount( const QModelIndex & = QModelIndex()) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex & ) const override { return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled; }
    QString fileName( const QModelIndex & ) const;
    QString filePath( const QModelIndex & ) const;
    QString mimeTypeName( const QModelIndex & ) const;
    QIcon fileIcon( const QModelIndex & ) const;
    int scale() const { return this->m_scale; }
    QIcon getIcon( int iconId ) const;
    static QPixmap getIconPixmap( int iconId, int scale = 48 );
    static QPixmap loadPixmapFromLibrary( int resourceId, int scale, const QString &name = "shell32" );
    qint64 size( const QModelIndex &index ) const;
    QDateTime lastModified( const QModelIndex & ) const { return QDateTime(); }

public slots:
    void setScale( int scale );

private:
    mutable QMap<int,QIcon> iconCache;
    int m_scale = 48; // TODO: copy from ListView
};
