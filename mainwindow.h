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
#include <QMainWindow>
#include <QPixmap>

/*
 * namespaces
 */
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = nullptr );
    ~MainWindow() override;
    enum UpdateType {
        NoType = -1,
        Image,
        SolidColour
    };
    Q_ENUM( UpdateType )

    enum FillMode {
        NoMode = -1,
        Fill,
        Fit,
        Stretch,
        Tile,
        Center
    };
    Q_ENUM( FillMode )

public slots:
    void updateWallpaper( UpdateType type );

protected:
    void paintEvent( QPaintEvent *event ) override;

private:
    Ui::MainWindow *ui;
    QPixmap pixmap;
};
