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
#include <QDialog>
#include <QMap>
#include <QPushButton>

/**
 * @brief The Ui namespace
 */
namespace Ui {
class BackgroundDialog;
[[maybe_unused]] static constexpr const int ColourColumnCount = 8;
[[maybe_unused]] static constexpr const int ImageColumnCount = 5;
[[maybe_unused]] static constexpr const int ColourTileSize = 45;
[[maybe_unused]] static constexpr const int ImageTileSize = 75;
[[maybe_unused]] static constexpr const int Spacing = 4;
[[maybe_unused]] static constexpr const char *DefaultWallpaper = "C:/Windows/Web/Wallpaper/Windows/img0.jpg";// "C:/Home/Development/FishWallPaper/fish.jpg";
}

/**
 * @brief The BackgroundDialog class
 */
class BackgroundDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackgroundDialog( QWidget *parent = nullptr );
    ~BackgroundDialog();
    [[nodiscard]] int currentIndex() const;

private slots:
    void setupColourPage();
    void setupImagePage();

private:
    Ui::BackgroundDialog *ui;
    QList<QColor> colours;
    QList<QString> images;
    QMap<QString, QPushButton*> buttonMap;
    int currentColumn = 0;
    int rows = 0;
};
