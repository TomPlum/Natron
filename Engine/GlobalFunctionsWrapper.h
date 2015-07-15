//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */
/**
* @brief Used to wrap all global functions that are in the Natron namespace so shiboken
* doesn't generate the Natron namespace
**/

#ifndef GLOBALFUNCTIONSWRAPPER_H
#define GLOBALFUNCTIONSWRAPPER_H

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include "Engine/AppManager.h"
#include "Engine/AppInstanceWrapper.h"
#include "Global/MemoryInfo.h"

class PyCoreApplication
{
    
public:
    
    PyCoreApplication() {}
    
    virtual ~PyCoreApplication() {}
    
    inline std::list<std::string>
    getPluginIDs() const
    {
        return appPTR->getPluginIDs();
    }
    
    inline std::list<std::string>
    getPluginIDs(const std::string& filter) const
    {
        return appPTR->getPluginIDs(filter);
    }
    
    inline int
    getNumInstances() const
    {
        return appPTR->getNumInstances();
    }
    
    inline std::list<std::string>
    getNatronPath() const
    {
        return appPTR->getNatronPath();
    }
    
    inline void
    appendToNatronPath(const std::string& path)
    {
        appPTR->appendToNatronPath(path);
    }
    
    inline bool isLinux() const
    {
#ifdef __NATRON_LINUX__
        return true;
#else
        return false;
#endif
    }
    
    inline bool isWindows() const
    {
#ifdef __NATRON_WIN32__
        return true;
#else
        return false;
#endif
    }
    
    inline bool isMacOSX() const
    {
#ifdef __NATRON_OSX__
        return true;
#else
        return false;
#endif
    }
    
    inline bool isUnix() const
    {
#ifdef __NATRON_UNIX__
        return true;
#else
        return false;
#endif
    }
    
    inline std::string getNatronVersionString() const
    {
        return NATRON_VERSION_STRING;
    }
    
    inline int getNatronVersionMajor() const
    {
        return NATRON_VERSION_MAJOR;
    }
    
    inline int getNatronVersionMinor() const
    {
        return NATRON_VERSION_MINOR;
    }
    
    inline int getNatronVersionRevision() const
    {
        return NATRON_VERSION_REVISION;
    }
    
    inline int getNatronVersionEncoded() const
    {
        return NATRON_VERSION_ENCODED;
    }
    
    inline std::string getNatronDevelopmentStatus() const
    {
        return std::string(NATRON_DEVELOPMENT_STATUS);
    }
    
    inline int getBuildNumber() const
    {
        return NATRON_BUILD_NUMBER;
    }
    
    inline bool is64Bit() const
    {
        return !isApplication32Bits();
    }
    
    bool isBackground() const
    {
        return appPTR->isBackground();
    }
    
    inline int getNumCpus() const
    {
        return appPTR->getHardwareIdealThreadCount();
    }
    
    inline App*
    getInstance(int idx) const
    {
        AppInstance* app = appPTR->getAppInstance(idx);
        if (!app) {
            return 0;
        }
        return new App(app);
    }
    
    
    inline AppSettings* getSettings() const
    {
        return new AppSettings(appPTR->getCurrentSettings());
    }
    
    inline void setOnProjectCreatedCallback(const std::string& pythonFunctionName)
    {
        appPTR->setOnProjectCreatedCallback(pythonFunctionName);
    }
    
    inline void setOnProjectLoadedCallback(const std::string& pythonFunctionName)
    {
        appPTR->setOnProjectLoadedCallback(pythonFunctionName);
    }
    
    

};

#if defined(PYSIDE_H) && defined(PYSIDE_OLD)
namespace PySide {
PYSIDE_API PyObject* getWrapperForQObject(QObject* cppSelf, SbkObjectType* sbk_type);
}
#endif



#endif // GLOBALFUNCTIONSWRAPPER_H
