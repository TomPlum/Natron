//
//  OfxHost.h
//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
//  Created by Frédéric Devernay on 03/09/13.
//
//

#ifndef NATRON_ENGINE_OFXHOST_H_
#define NATRON_ENGINE_OFXHOST_H_

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include <list>
#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/shared_ptr.hpp>
#endif

#include "Global/Macros.h"
// ofxhPropertySuite.h:565:37: warning: 'this' pointer cannot be null in well-defined C++ code; comparison may be assumed to always evaluate to true [-Wtautological-undefined-compare]
CLANG_DIAG_OFF(unknown-pragmas)
CLANG_DIAG_OFF(tautological-undefined-compare) // appeared in clang 3.5
#include <ofxhPluginCache.h>
CLANG_DIAG_ON(tautological-undefined-compare)
CLANG_DIAG_ON(unknown-pragmas)
#include <ofxhImageEffectAPI.h>

#include "Global/Macros.h"
#include "Global/Enums.h"

//#define MULTI_THREAD_SUITE_USES_THREAD_SAFE_MUTEX_ALLOCATION

class AbstractOfxEffectInstance;
class AppInstance;
class QMutex;
class NodeSerialization;
class KnobSerialization;
namespace Natron {
class Node;
class Plugin;
class OfxHost
    : public OFX::Host::ImageEffect::Host
{
public:

    OfxHost();

    virtual ~OfxHost();

    void setProperties();

    /// Create a new instance of an image effect plug-in.
    ///
    /// It is called by ImageEffectPlugin::createInstance which the
    /// client code calls when it wants to make a new instance.
    ///
    ///   \arg clientData - the clientData passed into the ImageEffectPlugin::createInstance
    ///   \arg plugin - the plugin being created
    ///   \arg desc - the descriptor for that plugin
    ///   \arg context - the context to be created in
    virtual OFX::Host::ImageEffect::Instance* newInstance(void* clientData,
                                                          OFX::Host::ImageEffect::ImageEffectPlugin* plugin,
                                                          OFX::Host::ImageEffect::Descriptor & desc,
                                                          const std::string & context) OVERRIDE;

    /// Override this to create a descriptor, this makes the 'root' descriptor
    virtual OFX::Host::ImageEffect::Descriptor * makeDescriptor(OFX::Host::ImageEffect::ImageEffectPlugin* plugin) OVERRIDE;

    /// used to construct a context description, rootContext is the main context
    virtual OFX::Host::ImageEffect::Descriptor * makeDescriptor(const OFX::Host::ImageEffect::Descriptor &rootContext,
                                                                OFX::Host::ImageEffect::ImageEffectPlugin *plug) OVERRIDE;

    /// used to construct populate the cache
    virtual OFX::Host::ImageEffect::Descriptor * makeDescriptor(const std::string &bundlePath,
                                                                OFX::Host::ImageEffect::ImageEffectPlugin *plug) OVERRIDE;

    /// vmessage
    virtual OfxStatus vmessage(const char* type,
                               const char* id,
                               const char* format,
                               va_list args) OVERRIDE;

    /// setPersistentMessage
    virtual OfxStatus setPersistentMessage(const char* type,
                                           const char* id,
                                           const char* format,
                                           va_list args) OVERRIDE;
    /// clearPersistentMessage
    virtual OfxStatus clearPersistentMessage() OVERRIDE;
    virtual void loadingStatus(const std::string &) OVERRIDE;
    virtual bool pluginSupported(OFX::Host::ImageEffect::ImageEffectPlugin *plugin, std::string &reason) const OVERRIDE;

    ///fetch the parametric parameters suite or returns the base class version
    virtual const void* fetchSuite(const char *suiteName, int suiteVersion) OVERRIDE;

#ifdef OFX_SUPPORTS_MULTITHREAD
    virtual OfxStatus multiThread(OfxThreadFunctionV1 func,unsigned int nThreads, void *customArg) OVERRIDE;
    virtual OfxStatus multiThreadNumCPUS(unsigned int *nCPUs) const OVERRIDE;
    virtual OfxStatus multiThreadIndex(unsigned int *threadIndex) const OVERRIDE;
    virtual int multiThreadIsSpawnedThread() const OVERRIDE;
    virtual OfxStatus mutexCreate(OfxMutexHandle *mutex, int lockCount) OVERRIDE;
    virtual OfxStatus mutexDestroy(const OfxMutexHandle mutex) OVERRIDE;
    virtual OfxStatus mutexLock(const OfxMutexHandle mutex) OVERRIDE;
    virtual OfxStatus mutexUnLock(const OfxMutexHandle mutex) OVERRIDE;
    virtual OfxStatus mutexTryLock(const OfxMutexHandle mutex) OVERRIDE;
#endif
#ifdef OFX_SUPPORTS_OPENGLRENDER
    /// @see OfxImageEffectOpenGLRenderSuiteV1.flushResources()
    virtual OfxStatus flushOpenGLResources() const OVERRIDE;
#endif

    virtual OFX::Host::Memory::Instance* newMemoryInstance(size_t nBytes) OVERRIDE FINAL WARN_UNUSED_RETURN;
    
    boost::shared_ptr<AbstractOfxEffectInstance> createOfxEffect(boost::shared_ptr<Node> node,
                                               const NodeSerialization* serialization,
                                                const std::list<boost::shared_ptr<KnobSerialization> >& paramValues,
                                                bool allowFileDialogs,
                                                bool disableRenderScaleSupport);

    void addPathToLoadOFXPlugins(const std::string path);

    /*Reads OFX plugin cache and scan plugins directories
       to load them all.*/
    void loadOFXPlugins(std::map<std::string,std::vector< std::pair<std::string,double> > >* readersMap,
                        std::map<std::string,std::vector< std::pair<std::string,double> > >* writersMap);

    void clearPluginsLoadedCache();

    void setThreadAsActionCaller(bool actionCaller);
    
    static OFX::Host::ImageEffect::Descriptor* getPluginContextAndDescribe(OFX::Host::ImageEffect::ImageEffectPlugin* plugin,
                                                                           Natron::ContextEnum* ctx);
private:

    

    /*Writes all plugins loaded and their descriptors to
       the OFX plugin cache. (called by the destructor) */
    void writeOFXCache();

    OFX::Host::ImageEffect::PluginCache* _imageEffectPluginCache;


    /*plugin name -> pair< plugin id , plugin grouping >
       The name of the plugin is followed by the first part of the grouping in brackets
       to help identify two distinct plugins with the same name. e.g :
       1)Invert [OFX]  with plugin id net.sourceforge.openfx.invert and grouping OFX/
       2)Invert [Toto] with plugin id com.toto.invert and grouping Toto/SuperPlugins/OFX/
     */
    struct OFXPluginEntry
    {
        std::string openfxId;
        std::string grouping;
        OFXPluginEntry(const std::string &ofxid,
                       const std::string &grp)
            : openfxId(ofxid)
              , grouping(grp)
        {
        }

        OFXPluginEntry()
        {
        }
    };
    
#ifdef MULTI_THREAD_SUITE_USES_THREAD_SAFE_MUTEX_ALLOCATION
    std::list<QMutex*> _pluginsMutexes;
    QMutex* _pluginsMutexesLock; //<protects _pluginsMutexes
#endif
};
} // namespace Natron

#endif // ifndef NATRON_ENGINE_OFXHOST_H_
