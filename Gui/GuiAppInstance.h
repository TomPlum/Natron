//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */

#ifndef GUIAPPINSTANCE_H
#define GUIAPPINSTANCE_H

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include <map>

#include "Engine/AppInstance.h"

#include "Global/Macros.h"

class NodeGui;

class Gui;
class ViewerTab;
class Format;
class KnobHolder;
/**
 * @brief This little struct contains what enables file dialogs to show previews.
 * It is shared by all dialogs so that we don't have to recreate the nodes everytimes
 **/
struct FileDialogPreviewProvider
{
    ViewerTab* viewerUI;
    boost::shared_ptr<Natron::Node> viewerNodeInternal;
    boost::shared_ptr<NodeGui> viewerNode;
    std::map<std::string,std::pair< boost::shared_ptr<Natron::Node>, boost::shared_ptr<NodeGui> > > readerNodes;
    
    FileDialogPreviewProvider()
    : viewerUI(0)
    , viewerNodeInternal()
    , viewerNode()
    , readerNodes()
    {}
};


struct GuiAppInstancePrivate;
class GuiAppInstance
    : public AppInstance
{
    Q_OBJECT

public:

    GuiAppInstance(int appID);

    virtual ~GuiAppInstance();

    void resetPreviewProvider();
private:
    
    
    void deletePreviewProvider();
    
    
    /** @brief Attemps to find an untitled autosave. If found one, prompts the user
     * whether he/she wants to load it. If something was loaded this function
     * returns true,otherwise false.
     **/
    bool findAndTryLoadUntitledAutoSave() WARN_UNUSED_RETURN;
    
public:
    
    virtual void aboutToQuit() OVERRIDE FINAL;
    virtual void load(const CLArgs& cl) OVERRIDE FINAL;
    
    Gui* getGui() const WARN_UNUSED_RETURN;

    /**
     * @brief Remove the node n from the mapping in GuiAppInstance and from the project so the pointer is no longer
     * referenced anywhere. This function is called on nodes that were already deleted by the user but were kept into
     * the undo/redo stack. That means this node is no longer references by any other node and can be safely deleted.
     * The first thing this function does is to assert that the node n is not active.
     **/
    void deleteNode(const boost::shared_ptr<NodeGui> & n);
    //////////

    virtual bool shouldRefreshPreview() const OVERRIDE FINAL;
    virtual void errorDialog(const std::string & title,const std::string & message, bool useHtml) const OVERRIDE FINAL;
    virtual void errorDialog(const std::string & title,const std::string & message,bool* stopAsking,bool useHtml) const OVERRIDE FINAL;
    virtual void warningDialog(const std::string & title,const std::string & message,bool useHtml) const OVERRIDE FINAL;
    virtual void warningDialog(const std::string & title,const std::string & message,bool* stopAsking,bool useHtml) const OVERRIDE FINAL;
    virtual void informationDialog(const std::string & title,const std::string & message,bool useHtml) const OVERRIDE FINAL;
    virtual void informationDialog(const std::string & title,const std::string & message,bool* stopAsking,bool useHtml) const OVERRIDE FINAL;
    virtual Natron::StandardButtonEnum questionDialog(const std::string & title,
                                                      const std::string & message,
                                                      bool useHtml,
                                                      Natron::StandardButtons buttons = Natron::StandardButtons(Natron::eStandardButtonYes | Natron::eStandardButtonNo),
                                                      Natron::StandardButtonEnum defaultButton = Natron::eStandardButtonNoButton) const OVERRIDE FINAL WARN_UNUSED_RETURN;
    
    virtual Natron::StandardButtonEnum questionDialog(const std::string & title,
                                                      const std::string & message,
                                                      bool useHtml,
                                                      Natron::StandardButtons buttons,
                                                      Natron::StandardButtonEnum defaultButton,
                                                      bool* stopAsking) OVERRIDE FINAL WARN_UNUSED_RETURN;
    
    virtual void loadProjectGui(boost::archive::xml_iarchive & archive) const OVERRIDE FINAL;
    virtual void saveProjectGui(boost::archive::xml_oarchive & archive) OVERRIDE FINAL;
    virtual void notifyRenderProcessHandlerStarted(const QString & sequenceName,
                                                   int firstFrame,int lastFrame,
                                                   const boost::shared_ptr<ProcessHandler> & process) OVERRIDE FINAL;
    virtual void setupViewersForViews(int viewsCount) OVERRIDE FINAL;

    void setViewersCurrentView(int view);

    void setUndoRedoStackLimit(int limit);

    bool isClosing() const;
    
    virtual bool isGuiFrozen() const OVERRIDE FINAL;

    virtual bool isShowingDialog() const OVERRIDE FINAL;
    virtual void startProgress(KnobHolder* effect,const std::string & message,bool canCancel = true) OVERRIDE FINAL;
    virtual void endProgress(KnobHolder* effect) OVERRIDE FINAL;
    virtual bool progressUpdate(KnobHolder* effect,double t) OVERRIDE FINAL;
    virtual void onMaxPanelsOpenedChanged(int maxPanels) OVERRIDE FINAL;
    virtual void connectViewersToViewerCache() OVERRIDE FINAL;
    virtual void disconnectViewersFromViewerCache() OVERRIDE FINAL;


    boost::shared_ptr<FileDialogPreviewProvider> getPreviewProvider() const;

    virtual std::string openImageFileDialog() OVERRIDE FINAL;
    virtual std::string saveImageFileDialog() OVERRIDE FINAL;

    virtual void startRenderingFullSequence(const AppInstance::RenderWork& w,bool renderInSeparateProcess,const QString& savePath) OVERRIDE FINAL;

    virtual void clearViewersLastRenderedTexture() OVERRIDE FINAL;
    
    virtual void appendToScriptEditor(const std::string& str) OVERRIDE FINAL;
    
    virtual void printAutoDeclaredVariable(const std::string& str) OVERRIDE FINAL;
    
    virtual void toggleAutoHideGraphInputs() OVERRIDE FINAL;
    virtual void setLastViewerUsingTimeline(const boost::shared_ptr<Natron::Node>& node) OVERRIDE FINAL;
    
    virtual ViewerInstance* getLastViewerUsingTimeline() const OVERRIDE FINAL;
    
    void discardLastViewerUsingTimeline();
    

    virtual void declareCurrentAppVariable_Python();

    virtual void createLoadProjectSplashScreen(const QString& projectFile) OVERRIDE FINAL;
    
    virtual void updateProjectLoadStatus(const QString& str) OVERRIDE FINAL;
    
    virtual void closeLoadPRojectSplashScreen() OVERRIDE FINAL;
    

    virtual void renderAllViewers() OVERRIDE FINAL;
    
    
    virtual void queueRedrawForAllViewers() OVERRIDE FINAL;
    
    int getOverlayRedrawRequestsCount() const;
    
    void clearOverlayRedrawRequests();
    
    public Q_SLOTS:
    

    void reloadStylesheet();

    virtual void redrawAllViewers() OVERRIDE FINAL;

    void onProcessFinished();

    void projectFormatChanged(const Format& f);
    
    virtual bool isDraftRenderEnabled() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    
    virtual void setUserIsPainting(const boost::shared_ptr<Natron::Node>& rotopaintNode) OVERRIDE FINAL;
    virtual boost::shared_ptr<Natron::Node> getIsUserPainting() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    
private:

    virtual void onGroupCreationFinished(const boost::shared_ptr<Natron::Node>& node) OVERRIDE FINAL;
    
    virtual void createNodeGui(const boost::shared_ptr<Natron::Node> &node,
                               const boost::shared_ptr<Natron::Node>&  parentMultiInstance,
                               bool loadRequest,
                               bool autoConnect,
                               double xPosHint,double yPosHint,
                               bool pushUndoRedoCommand) OVERRIDE FINAL;
    

    boost::scoped_ptr<GuiAppInstancePrivate> _imp;
};

#endif // GUIAPPINSTANCE_H
