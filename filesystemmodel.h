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
#include <QFileSystemModel>
#include <QFileInfo>
#include <QIcon>

/**
 * @brief The FileSystemModel class
 */
class FileSystemModel : public QFileSystemModel {
    Q_OBJECT

public:
    explicit FileSystemModel( const QString &path, QObject *parent = nullptr );
    ~FileSystemModel() override = default;
    QIcon fileIcon( const QModelIndex & ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    int scale() const { return this->m_scale; }
    static QPixmap getIconPixmap( const QFileInfo &info, int size = 48 );
    QIcon getIcon( const QFileInfo &info ) const;
    int rowCount( const QModelIndex & ) const override;
    int columnCount( const QModelIndex & ) const override { return 1; }
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QString mimeTypeName( const QModelIndex &index ) const;

public slots:
    void setScale( int scale );

private:
    mutable QMap<QPair<QString,qint64>,QIcon> iconCache;
    int m_scale = 48; // TODO: copy from ListView
};
