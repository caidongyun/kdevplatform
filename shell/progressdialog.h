/* -*- c++ -*-
 * progressdialog.h
 *
 *  Copyright (c) 2004 Till Adam <adam@kde.org>
 *  based on imapprogressdialog.cpp ,which is
 *  Copyright (c) 2002-2003 Klar�vdalens Datakonsult AB
 *  Copyright (c) 2009 Manuel Breugelmans <mbr.nxi@gmail.com>
 *      copied from pimlibs
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef KDEVELOP_PROGRESSDIALOG_H
#define KDEVELOP_PROGRESSDIALOG_H

#include "overlaywidget.h"

#include <QScrollArea>
#include <QMap>
#include <KVBox>

class QProgressBar;
class QFrame;
class QLabel;
class QPushButton;

namespace KDevelop
{

class ProgressItem;
class TransactionItem;
class ProgressManager;

class TransactionItemView : public QScrollArea
{
Q_OBJECT
public:
  explicit TransactionItemView( QWidget * parent = 0, const char * name = 0 );
  virtual ~TransactionItemView();

  TransactionItem* addTransactionItem( ProgressItem *item, bool first );
  QSize sizeHint() const;
  QSize minimumSizeHint() const;

public Q_SLOTS:
  void slotLayoutFirstItem();

protected:
  virtual void resizeEvent ( QResizeEvent *event );

private:
  KVBox *mBigBox;
};

/**
 * A single progress & status message widget shown in a ProgressDialog 
 */
class TransactionItem : public KVBox
{
Q_OBJECT
public:
  TransactionItem( QWidget * parent,
                   ProgressItem* item, bool first );
  ~TransactionItem();

  void hideHLine();
  void setProgress( int progress );
  void setLabel( const QString& );
  void setStatus( const QString& );
  ProgressItem* item() const { return mItem; }
  void addSubTransaction( ProgressItem *item);

  // The progressitem is deleted immediately, we take 5s to go out,
  // so better not use mItem during this time.
  void setItemComplete() { mItem = 0; }

public Q_SLOTS:
  void slotItemCanceled();

protected:
  QProgressBar* mProgress;
  QPushButton*  mCancelButton;
  QLabel*       mItemLabel;
  QLabel*       mItemStatus;
  QFrame*       mFrame;
  ProgressItem* mItem;
};


/**
 * Aggregates progressbars & status information for multiple items.
 * The widget is shown on top of an existing widget.
 */
class ProgressDialog : public OverlayWidget
{
Q_OBJECT

public:
  ProgressDialog( ProgressManager* progressManager, QWidget* alignWidget, QWidget* parent, const char* name = 0 );
  ~ProgressDialog();
  void setVisible( bool b );

public Q_SLOTS:
  void slotToggleVisibility();

protected Q_SLOTS:
  void slotTransactionAdded( KDevelop::ProgressItem *item );
  void slotTransactionCompleted( KDevelop::ProgressItem *item );
  void slotTransactionCanceled( KDevelop::ProgressItem *item );
  void slotTransactionProgress( KDevelop::ProgressItem *item, unsigned int progress );
  void slotTransactionStatus( KDevelop::ProgressItem *item, const QString& );
  void slotTransactionLabel( KDevelop::ProgressItem *item, const QString& );

  void slotClose();
  void slotShow();
  void slotHide();

Q_SIGNALS:
  void visibilityChanged( bool );

protected:
  virtual void closeEvent( QCloseEvent* );

protected:
  TransactionItemView* mScrollView;
  TransactionItem* mPreviousItem;
  QMap< const ProgressItem*, TransactionItem* > mTransactionsToListviewItems;
  bool mWasLastShown;
};

} // namespace KDevelop

#endif
