/*
    Interface description for KDevelop OutputView Filter strategies
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef IFILTERSTRATEGY_H
#define IFILTERSTRATEGY_H

#include "outputviewexport.h"

class QString;

namespace KDevelop
{

class FilteredItem;

/**
* Interface class for filtering output. Filtered output is divided into two catagories: Errors
* and Actions. Use this interface if you want to write a filter for the outputview.
*
* @author Morten Danielsen Volden
*/
class KDEVPLATFORMOUTPUTVIEW_EXPORT IFilterStrategy
{
public:

    IFilterStrategy();
    virtual ~IFilterStrategy();

    /**
     * Examine if a given line contains output that is defined as an error (E.g. from a script or from a compiler, or other).
     * @param line the line to examine
     * @param item Where all the metadata about the current line is put after the given filter is applied
     * @return true if an error is found, false otherwise
     **/
    virtual FilteredItem errorInLine(QString const& line) = 0;

    /**
     * Examine if a given line contains output that is defined as an action (E.g. from a script or from a compiler, or other).
     * @param line the line to examine
     * @param item Where all the metadata about the current line is put after the given filter is applied
     * @return true if an action is found, false otherwise
     **/
    virtual FilteredItem actionInLine(QString const& line) = 0;

};



} // namespace KDevelop

#endif // IFILTERSTRATEGY_H
