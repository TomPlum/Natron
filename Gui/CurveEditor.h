//  Natron
//
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */

#ifndef CURVEEDITOR_H
#define CURVEEDITOR_H

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#endif
#include "Global/Macros.h"
CLANG_DIAG_OFF(deprecated)
CLANG_DIAG_OFF(uninitialized)
#include <QWidget>
CLANG_DIAG_ON(deprecated)
CLANG_DIAG_ON(uninitialized)

#include "Global/GlobalDefines.h"
#include "Global/Macros.h"

#include "Engine/ScriptObject.h"

#include "Gui/CurveSelection.h"
#include "Gui/CurveEditorUndoRedo.h"

class RectD;
class NodeGui;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;
class CurveWidget;
class CurveEditor;
class Curve;
class CurveGui;
class QHBoxLayout;
class QSplitter;
class KnobGui;
class KnobI;
class BezierCP;
class Bezier;
class RotoDrawableItem;
class RotoStrokeItem;
class LineEdit;
class RotoItem;
class RotoContext;
class KeyFrame;
class Variant;
class Gui;
class QAction;
class TimeLine;

/**
 * All nodes are tracked in the CurveEditor and they all have a NodeCurveEditorContext.
 * Each node context owns a list of NodeCurveEditorElement which corresponds to the animation
 * for one parameter (knob). You can show/hide the parameter's animation by calling
 * checkVisibleState() which will automatically show/hide the curve from the tree if it has (or hasn't) an animation.
 **/

class NodeCurveEditorElement
    : public QObject
{
    Q_OBJECT

public:

    NodeCurveEditorElement(QTreeWidget* tree,
                           CurveEditor* curveWidget,
                           KnobGui* knob,
                           int dimension,
                           QTreeWidgetItem* item,
                           const boost::shared_ptr<CurveGui>& curve);
    
    NodeCurveEditorElement(QTreeWidget* tree,
                           CurveEditor* curveWidget,
                           const boost::shared_ptr<KnobI>& internalKnob,
                           int dimension,
                           QTreeWidgetItem* item,
                           const boost::shared_ptr<CurveGui>& curve);

    NodeCurveEditorElement()
        : _treeItem(),_curve(),_curveDisplayed(false),_curveWidget(NULL)
    {
    }

    virtual ~NodeCurveEditorElement() OVERRIDE;

    QTreeWidgetItem* getTreeItem() const WARN_UNUSED_RETURN
    {
        return _treeItem;
    }

    boost::shared_ptr<CurveGui>  getCurve() const WARN_UNUSED_RETURN
    {
        return _curve;
    }

    bool isCurveVisible() const WARN_UNUSED_RETURN
    {
        return _curveDisplayed;
    }
    
    void setVisible(bool visible);

    int getDimension() const WARN_UNUSED_RETURN
    {
        return _dimension;
    }

    KnobGui* getKnobGui() const WARN_UNUSED_RETURN
    {
        return _knob;
    }
    
    boost::shared_ptr<KnobI> getInternalKnob() const WARN_UNUSED_RETURN;
    
    void checkVisibleState(bool autoSelectOnShow);

public Q_SLOTS:
    
    
    /**
     * @brief This is invoked everytimes the knob has a keyframe set or removed, to determine whether we need
     * to keep this element in the tree or not.
     **/
    void checkVisibleState();

    void onExpressionChanged();
    
private:


    QTreeWidgetItem* _treeItem;
    boost::shared_ptr<CurveGui> _curve;
    bool _curveDisplayed;
    CurveEditor* _curveWidget;
    QTreeWidget* _treeWidget;
    KnobGui* _knob;
    boost::shared_ptr<KnobI> _internalKnob;
    int _dimension;
};

class NodeCurveEditorContext
    : public QObject
{
    Q_OBJECT

public:

    typedef std::list< NodeCurveEditorElement* > Elements;

    NodeCurveEditorContext(QTreeWidget *tree,
                           CurveEditor* curveWidget,
                           const boost::shared_ptr<NodeGui> &node);

    virtual ~NodeCurveEditorContext() OVERRIDE;

    boost::shared_ptr<NodeGui> getNode() const WARN_UNUSED_RETURN
    {
        return _node;
    }
    
    QTreeWidgetItem* getItem() const
    {
        return _nameItem;
    }

    const Elements & getElements() const WARN_UNUSED_RETURN
    {
        return _nodeElements;
    }
    
    bool isVisible() const;
    
    void setVisible(bool visible);

    NodeCurveEditorElement* findElement(CurveGui* curve) const WARN_UNUSED_RETURN;
    NodeCurveEditorElement* findElement(KnobGui* knob,int dimension) const WARN_UNUSED_RETURN;
    NodeCurveEditorElement* findElement(QTreeWidgetItem* item) const WARN_UNUSED_RETURN;

public Q_SLOTS:

    void onNameChanged(const QString & name);

private:
    // FIXME: PIMPL
    boost::shared_ptr<NodeGui> _node;
    Elements _nodeElements;
    QTreeWidgetItem* _nameItem;
};


class RotoCurveEditorContext;

struct RotoItemEditorContextPrivate;
class RotoItemEditorContext
: public QObject
{
    Q_OBJECT
    
public:
    
    RotoItemEditorContext(QTreeWidget* tree,
                        CurveEditor* widget,
                        const boost::shared_ptr<RotoDrawableItem>& curve,
                        RotoCurveEditorContext* context);
    
    virtual ~RotoItemEditorContext();

    //Called when the destr. of RotoCurveEditorContext is called to prevent
    //the tree items to be deleted twice due to Qt's parenting
    void preventItemDeletion();
    
    QTreeWidgetItem* getItem() const;
    
    boost::shared_ptr<RotoDrawableItem> getRotoItem() const;
    
    QString getName() const;
    
    boost::shared_ptr<RotoContext> getContext() const;
    
    const std::list<NodeCurveEditorElement*>& getElements() const;
    
    NodeCurveEditorElement* findElement(KnobGui* knob,int dimension) const;
    
    void recursiveSelect(QTreeWidgetItem* cur,bool mustSelect,
                               std::vector<boost::shared_ptr<CurveGui> > *curves);
    
    CurveEditor* getWidget() const;

public Q_SLOTS:
    
    void onNameChanged(const QString & name);
    
    void onKeyframeAdded();
    
    void onKeyframeRemoved();
    
    
protected:
    
    virtual void getAnimCurveAndItem(QTreeWidgetItem** /*item*/,boost::shared_ptr<CurveGui>* /*curve*/) const {}
    
private:
    
    boost::scoped_ptr<RotoItemEditorContextPrivate> _imp;
};

struct BezierEditorContextPrivate;
class BezierEditorContext
: public RotoItemEditorContext
{
    Q_OBJECT
    
public:
    
    BezierEditorContext(QTreeWidget* tree,
                        CurveEditor* widget,
                        const boost::shared_ptr<Bezier>& curve,
                        RotoCurveEditorContext* context);
    
    virtual ~BezierEditorContext() OVERRIDE;
    
    virtual void getAnimCurveAndItem(QTreeWidgetItem** item,boost::shared_ptr<CurveGui>* curve) const OVERRIDE FINAL;
    
    

private:
       
    boost::scoped_ptr<BezierEditorContextPrivate> _imp;
    
};


struct RotoCurveEditorContextPrivate;
class RotoCurveEditorContext
: public QObject
{
    Q_OBJECT
    
public:
    
    RotoCurveEditorContext(CurveEditor* widget,
                           QTreeWidget *tree,
                           const boost::shared_ptr<NodeGui> &node);
    
    virtual ~RotoCurveEditorContext() OVERRIDE;
    
    boost::shared_ptr<NodeGui> getNode() const WARN_UNUSED_RETURN;
    
    QTreeWidgetItem* getItem() const;
    
    void recursiveSelectRoto(QTreeWidgetItem* cur,
                             std::vector<boost::shared_ptr<CurveGui> > *curves);
    
    void setVisible(bool visible);
    
    const std::list<RotoItemEditorContext*>& getElements() const;

    std::list<NodeCurveEditorElement*> findElement(KnobGui* knob,int dimension) const;
    
public Q_SLOTS:
    
    void onNameChanged(const QString & name);
    
    void onItemNameChanged(const boost::shared_ptr<RotoItem>& item);
    
    void itemInserted(int);
    
    void onItemRemoved(const boost::shared_ptr<RotoItem>& item, int);

private:

    boost::scoped_ptr<RotoCurveEditorContextPrivate> _imp;
    
};

struct CurveEditorPrivate;
class CurveEditor
    : public QWidget
    , public CurveSelection
    , public ScriptObject
{
    Q_OBJECT

public:

    CurveEditor(Gui* gui,
                boost::shared_ptr<TimeLine> timeline,
                QWidget* parent = 0);

    virtual ~CurveEditor() OVERRIDE;

    /**
     * @brief Creates a new NodeCurveEditorContext and stores it until the CurveEditor is destroyed.
     **/
    void addNode(boost::shared_ptr<NodeGui> node);

    void removeNode(NodeGui* node);
    
    void setTreeWidgetWidth(int width);

    void centerOn(const std::vector<boost::shared_ptr<Curve> > & curves);

    std::pair<QAction*,QAction*> getUndoRedoActions() const WARN_UNUSED_RETURN;
    std::list<boost::shared_ptr<CurveGui> > findCurve(KnobGui* knob,int dimension) const WARN_UNUSED_RETURN;

    void hideCurves(KnobGui* knob);

    void hideCurve(KnobGui* knob,int dimension);

    void showCurves(KnobGui* knob);

    void showCurve(KnobGui* knob,int dimension);

    CurveWidget* getCurveWidget() const WARN_UNUSED_RETURN;

    virtual void getSelectedCurves(std::vector<boost::shared_ptr<CurveGui> >* selection) OVERRIDE FINAL;
    
    void setSelectedCurve(const boost::shared_ptr<CurveGui>& curve);
    
    boost::shared_ptr<CurveGui> getSelectedCurve() const;

    void refreshCurrentExpression();
    
    void setSelectedCurveExpression(const QString& expression);
    
public Q_SLOTS:

    void onFilterTextChanged(const QString& filter);
    
    void onItemSelectionChanged();
        
    void onItemDoubleClicked(QTreeWidgetItem* item,int);
    
    void onExprLineEditFinished();
    
private:

    virtual void keyPressEvent(QKeyEvent* e) OVERRIDE FINAL;
    
    void recursiveSelect(QTreeWidgetItem* cur,std::vector<boost::shared_ptr<CurveGui> > *curves,bool inspectRotos = true);

    boost::scoped_ptr<CurveEditorPrivate> _imp;
};



#endif // CURVEEDITOR_H
