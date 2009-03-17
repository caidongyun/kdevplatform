/***************************************************************************
 *   This file was taken from KDevelop's git plugin                        *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Mercurial                                                 *
 *   Copyright 2009 Fabian Wiesel <fabian.wiesel@fu-berlin.de>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef MERCURIAL_INIT_H
#define MERCURIAL_INIT_H

#include <QtCore/QObject>

class MercurialPlugin;

namespace KDevelop {
class TestCore;
}

class MercurialInitTest: public QObject
{
    Q_OBJECT

private:
    void repoInit();
    void addFiles();
    void commitFiles();
    void cloneRepository();
    // void importTestData();
    // void checkoutTestData();

private slots:
    void initTestCase();
    void testInit();
    void testAdd();
    void testCommit();
    void testBranching();
    void revHistory();
    void cleanupTestCase();

private:
    MercurialPlugin* m_proxy;
    KDevelop::TestCore* m_testCore;
    void removeTempDirs();
};

#endif