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

#ifndef GLOBAL_GUI_WRAPPER_H
#define GLOBAL_GUI_WRAPPER_H

#include "Engine/GlobalFunctionsWrapper.h"

#include <QKeyEvent>

#include "Gui/GuiAppWrapper.h"
#include "Gui/GuiApplicationManager.h"

class PyGuiApplication : public PyCoreApplication
{
public:
    
    PyGuiApplication()
    : PyCoreApplication()
    {
        
    }
    
    virtual ~PyGuiApplication()
    {
        
    }
    
    QPixmap getIcon(Natron::PixmapEnum val) const
    {
        QPixmap ret;
        appPTR->getIcon(val,&ret);
        return ret;
    }
    
    GuiApp* getGuiInstance(int idx) const
    {
        AppInstance* app = appPTR->getAppInstance(idx);
        if (!app) {
            return 0;
        }
        GuiAppInstance* guiApp = dynamic_cast<GuiAppInstance*>(app);
        if (!guiApp) {
            return 0;
        }
        return new GuiApp(app);
    }
    
    void informationDialog(const std::string& title,const std::string& message)
    {
        Natron::informationDialog(title, message);
    }
    
    void warningDialog(const std::string& title,const std::string& message)
    {
        Natron::warningDialog(title,message);
    }
    
    void errorDialog(const std::string& title,const std::string& message)
    {
        Natron::errorDialog(title,message);
    }
    
    Natron::StandardButtonEnum questionDialog(const std::string& title,const std::string& message)
    {
        return Natron::questionDialog(title, message, false);
    }
    
    void addMenuCommand(const std::string& grouping,const std::string& pythonFunctionName)
    {
        appPTR->addCommand(grouping.c_str(), pythonFunctionName, (Qt::Key)0, Qt::NoModifier);
    }
    
    
    void addMenuCommand(const std::string& grouping,const std::string& pythonFunctionName,
                        Qt::Key key, const Qt::KeyboardModifiers& modifiers)
    {
        appPTR->addCommand(grouping.c_str(), pythonFunctionName, key, modifiers);
    }
    
};



#endif // GLOBAL_GUI_WRAPPER_H
