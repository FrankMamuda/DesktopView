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
#include "mainwindow.h"
#include <QApplication>

//
// TODO:
//  fix slowdown after system context menu
//  menus:
//     icon size: small, medium, large
//     icons: pc, documents, trash
//     thumbnails
//     delete dialog
//     move/copy dialogs on drag/drop
//     proper icon scaling (set to 48 in models)
//     proper desktop icon selection (menu actions are just dummies)
//
//  port to other OSs...or not
//

int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );

    QCoreApplication::setOrganizationName( "factory12" );
    QCoreApplication::setOrganizationDomain( "factory12.org" );
    QCoreApplication::setApplicationName( "desktopview" );

    MainWindow w;
    w.showMaximized();

    return a.exec();
}
