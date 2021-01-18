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
#include <QAbstractItemModel>
#include <QIcon>

/**
 * @brief The MultiDirModel class
 */
class MultiDirModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit MultiDirModel( QObject *parent = nullptr ) : QAbstractListModel( parent ) {}
    ~MultiDirModel() override = default;

    int columnCount( const QModelIndex & ) const override { return 1; }
    int rowCount( const QModelIndex &parent = QModelIndex()) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QModelIndex mapToSource( const QModelIndex &index ) const;
    QString fileName( const QModelIndex &index ) const;
    QString filePath( const QModelIndex &index ) const;
    QFileInfo fileInfo( const QModelIndex &index ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QString mimeTypeName( const QModelIndex &index ) const;
    qint64 size( const QModelIndex &index ) const;
    QDateTime lastModified( const QModelIndex &index ) const;

public slots:
    void add( QAbstractItemModel *model ) { this->models << model; }
    void reset();

signals:
    void loaded();

private:
    QList<QAbstractItemModel*> models;
    mutable QList<QModelIndex> cache;
};
