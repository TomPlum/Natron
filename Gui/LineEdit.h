/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2015 INRIA and Alexandre Gauthier
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#ifndef NATRON_GUI_LINEEDIT_H_
#define NATRON_GUI_LINEEDIT_H_

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/Macros.h"
CLANG_DIAG_OFF(deprecated)
CLANG_DIAG_OFF(uninitialized)
#include <QLineEdit>
CLANG_DIAG_ON(deprecated)
CLANG_DIAG_ON(uninitialized)

#include "Global/Macros.h"

class QPaintEvent;
class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;

class LineEdit
    : public QLineEdit
{
GCC_DIAG_SUGGEST_OVERRIDE_OFF
    Q_OBJECT
GCC_DIAG_SUGGEST_OVERRIDE_ON

    Q_PROPERTY( int animation READ getAnimation WRITE setAnimation)
    Q_PROPERTY(bool dirty READ getDirty WRITE setDirty)
    Q_PROPERTY(bool altered READ getAltered WRITE setAltered)
    
public:
    explicit LineEdit(QWidget* parent = 0);
    virtual ~LineEdit() OVERRIDE;

    int getAnimation() const
    {
        return animation;
    }

    void setAnimation(int v);

    bool getDirty() const
    {
        return dirty;
    }

    void setDirty(bool b);
    
    void setAltered(bool b);
    bool getAltered() const
    {
        return altered;
    }

Q_SIGNALS:
    
    void textDropped();
    
    void textPasted();
    
public Q_SLOTS:

    void onEditingFinished();

protected:
    
    virtual void paintEvent(QPaintEvent* e) OVERRIDE;

private:
    
    virtual void dropEvent(QDropEvent* e) OVERRIDE FINAL;

    virtual void dragEnterEvent(QDragEnterEvent* e) OVERRIDE FINAL;

    virtual void dragMoveEvent(QDragMoveEvent* e) OVERRIDE FINAL;

    virtual void dragLeaveEvent(QDragLeaveEvent* e) OVERRIDE FINAL;
    
    virtual void keyPressEvent(QKeyEvent* e) OVERRIDE;
    
    int animation;
    bool dirty;
    bool altered;
};


#endif // ifndef NATRON_GUI_LINEEDIT_H_
