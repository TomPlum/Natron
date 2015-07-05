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

#ifndef MULTIINSTANCEPANEL_H
#define MULTIINSTANCEPANEL_H

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#endif
#include "Engine/Knob.h"

namespace Natron {
class Node;
}
class NodeGui;
class QVBoxLayout;
class QHBoxLayout;
class QItemSelection;
class TableItem;
class Button_Knob;
class Gui;
/**
 * @brief This class represents a multi-instance settings panel.
 **/
struct MultiInstancePanelPrivate;
class MultiInstancePanel
    :  public NamedKnobHolder
{
    Q_OBJECT

public:

    MultiInstancePanel(const boost::shared_ptr<NodeGui> & node);

    virtual ~MultiInstancePanel();

    void createMultiInstanceGui(QVBoxLayout* layout);

    bool isGuiCreated() const;

    void addRow(const boost::shared_ptr<Natron::Node> & node);

    void removeRow(int index);

    int getNodeIndex(const boost::shared_ptr<Natron::Node> & node) const;

    const std::list< std::pair<boost::weak_ptr<Natron::Node>,bool > > & getInstances() const;
    virtual std::string getScriptName_mt_safe() const OVERRIDE FINAL;
    boost::shared_ptr<Natron::Node> getMainInstance() const;
    
    boost::shared_ptr<NodeGui> getMainInstanceGui() const;

    void getSelectedInstances(std::list<Natron::Node*>* instances) const;

    void resetAllInstances();

    boost::shared_ptr<KnobI> getKnobForItem(TableItem* item,int* dimension) const;
    Gui* getGui() const;
    virtual void setIconForButton(Button_Knob* /*knob*/)
    {
    }

    boost::shared_ptr<Natron::Node> createNewInstance(bool useUndoRedoStack);

    void selectNode(const boost::shared_ptr<Natron::Node> & node,bool addToSelection);

    void selectNodes(const std::list<Natron::Node*> & nodes,bool addToSelection);

    void removeNodeFromSelection(const boost::shared_ptr<Natron::Node> & node);

    void clearSelection();

    bool isSettingsPanelVisible() const;
        
    void removeInstances(const std::list<boost::shared_ptr<Natron::Node> >& instances);
    void addInstances(const std::list<boost::shared_ptr<Natron::Node> >& instances);

    void onChildCreated(const boost::shared_ptr<Natron::Node>& node);
    
    void setRedrawOnSelectionChanged(bool redraw);
    
public Q_SLOTS:

    void onAddButtonClicked();

    void onRemoveButtonClicked();

    void onSelectAllButtonClicked();

    void onSelectionChanged(const QItemSelection & oldSelection,const QItemSelection & newSelection);

    void onItemDataChanged(TableItem* item);

    void onCheckBoxChecked(bool checked);

    void onDeleteKeyPressed();

    void onInstanceKnobValueChanged(int dim,int reason);

    void resetSelectedInstances();

    void onSettingsPanelClosed(bool closed);

    void onItemRightClicked(TableItem* item);

protected:

    virtual void appendExtraGui(QVBoxLayout* /*layout*/)
    {
    }

    virtual void appendButtons(QHBoxLayout* /*buttonLayout*/)
    {
    }

    boost::shared_ptr<Natron::Node> addInstanceInternal(bool useUndoRedoStack);
    virtual void initializeExtraKnobs()
    {
    }

    virtual void showMenuForInstance(Natron::Node* /*instance*/)
    {
    }

private:

    virtual void onButtonTriggered(Button_Knob* button);

    void resetInstances(const std::list<Natron::Node*> & instances);

    void removeInstancesInternal();

    virtual void evaluate(KnobI* knob,bool isSignificant,Natron::ValueChangedReasonEnum reason) OVERRIDE;
    virtual void initializeKnobs() OVERRIDE FINAL;
    virtual void onKnobValueChanged(KnobI* k,Natron::ValueChangedReasonEnum reason,SequenceTime time,
                                    bool originatedFromMainThread) OVERRIDE;
    boost::scoped_ptr<MultiInstancePanelPrivate> _imp;
};

struct TrackerPanelPrivate;
class TrackerPanel
    : public MultiInstancePanel
{
    Q_OBJECT

public:

    TrackerPanel(const boost::shared_ptr<NodeGui> & node);

    virtual ~TrackerPanel();

    ///Each function below returns true if there is a selection, false otherwise
    bool trackBackward();
    bool trackForward();
    bool trackPrevious();
    bool trackNext();
    void stopTracking();

    void clearAllAnimationForSelection();

    void clearBackwardAnimationForSelection();

    void clearForwardAnimationForSelection();

    void setUpdateViewerOnTracking(bool update);

    bool isUpdateViewerOnTrackingEnabled() const;
public Q_SLOTS:

    void onAverageTracksButtonClicked();
    void onExportButtonClicked();

    void onTrackingStarted();
    
    void onTrackingFinished();
    
    void onTrackingProgressUpdate(double progress);
    
Q_SIGNALS:
    
    void trackingEnded();
    
private:

    virtual void initializeExtraKnobs() OVERRIDE FINAL;
    virtual void appendExtraGui(QVBoxLayout* layout) OVERRIDE FINAL;
    virtual void appendButtons(QHBoxLayout* buttonLayout) OVERRIDE FINAL;
    virtual void setIconForButton(Button_Knob* knob) OVERRIDE FINAL;
    virtual void onButtonTriggered(Button_Knob* button) OVERRIDE FINAL;
    virtual void showMenuForInstance(Natron::Node* item) OVERRIDE FINAL;

    boost::scoped_ptr<TrackerPanelPrivate> _imp;
};



#endif // MULTIINSTANCEPANEL_H
