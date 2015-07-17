
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

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include "KnobSerialization.h"

#include <algorithm> // min, max

#include <QDebug>

#include "Engine/Knob.h"
#include "Engine/Curve.h"
#include "Engine/Node.h"
#include "Engine/EffectInstance.h"
#include "Engine/AppInstance.h"
#include "Engine/KnobTypes.h"


ValueSerialization::ValueSerialization(const boost::shared_ptr<KnobI> & knob,
                                       int dimension)
    : _knob(knob)
    , _dimension(dimension)
    , _master()
    , _expression()
    , _exprHasRetVar(false)
{
}

ValueSerialization::ValueSerialization(const boost::shared_ptr<KnobI> & knob,
                   int dimension,
                   bool exprHasRetVar,
                   const std::string& expr)
: _knob(knob)
, _dimension(dimension)
, _master()
, _expression(expr)
, _exprHasRetVar(exprHasRetVar)
{
    std::pair< int, boost::shared_ptr<KnobI> > m = knob->getMaster(dimension);
    if ( m.second && !knob->isMastersPersistenceIgnored() ) {
        _master.masterDimension = m.first;
        NamedKnobHolder* holder = dynamic_cast<NamedKnobHolder*>( m.second->getHolder() );
        
        assert(holder);
        // coverity[dead_error_line]
        _master.masterNodeName = holder ? holder->getScriptName_mt_safe() : "";
        _master.masterKnobName = m.second->getName();
    } else {
        _master.masterDimension = -1;
    }
}

boost::shared_ptr<KnobI> KnobSerialization::createKnob(const std::string & typeName,
                                                       int dimension)
{
    boost::shared_ptr<KnobI> ret;

    if ( typeName == Int_Knob::typeNameStatic() ) {
        ret.reset( new Int_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Bool_Knob::typeNameStatic() ) {
        ret.reset( new Bool_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Double_Knob::typeNameStatic() ) {
        ret.reset( new Double_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Choice_Knob::typeNameStatic() ) {
        ret.reset( new Choice_Knob(NULL,"",dimension,false) );
    } else if ( typeName == String_Knob::typeNameStatic() ) {
        ret.reset( new String_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Parametric_Knob::typeNameStatic() ) {
        ret.reset( new Parametric_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Color_Knob::typeNameStatic() ) {
        ret.reset( new Color_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Path_Knob::typeNameStatic() ) {
        ret.reset( new Path_Knob(NULL,"",dimension,false) );
    } else if ( typeName == File_Knob::typeNameStatic() ) {
        ret.reset( new File_Knob(NULL,"",dimension,false) );
    } else if ( typeName == OutputFile_Knob::typeNameStatic() ) {
        ret.reset( new OutputFile_Knob(NULL,"",dimension,false) );
    } else if ( typeName == Button_Knob::typeNameStatic() ) {
        ret.reset(new Button_Knob(NULL,"",dimension,false));
    }
    if (ret) {
        ret->populate();
    }

    return ret;
}

void
KnobSerialization::restoreKnobLinks(const boost::shared_ptr<KnobI> & knob,
                                    const std::list<boost::shared_ptr<Natron::Node> > & allNodes)
{
    int i = 0;

    for (std::list<MasterSerialization>::iterator it = _masters.begin(); it != _masters.end(); ++it) {
        if (it->masterDimension != -1) {
            ///we need to cycle through all the nodes of the project to find the real master
            boost::shared_ptr<Natron::Node> masterNode;
            for (std::list<boost::shared_ptr<Natron::Node> >::const_iterator it2 = allNodes.begin(); it2 != allNodes.end(); ++it2) {
                if ((*it2)->getScriptName() == it->masterNodeName) {
                    masterNode = *it2;
                    break;
                }
            }
            if (!masterNode) {
                qDebug() << "Link slave/master for " << knob->getName().c_str() <<   " failed to restore the following linkage: " << it->masterNodeName.c_str();
                ++i;
                continue;
            }

            ///now that we have the master node, find the corresponding knob
            const std::vector< boost::shared_ptr<KnobI> > & otherKnobs = masterNode->getKnobs();
            bool found = false;
            for (U32 j = 0; j < otherKnobs.size(); ++j) {
                if ( (otherKnobs[j]->getName() == it->masterKnobName) && otherKnobs[j]->getIsPersistant() ) {
                    knob->slaveTo(i, otherKnobs[j], it->masterDimension);
                    found = true;
                    break;
                }
            }
            if (!found) {
                qDebug() << "Link slave/master for " << knob->getName().c_str() <<   " failed to restore the following linkage: " << it->masterNodeName.c_str();
            }
        }
        ++i;
    }
}

void
KnobSerialization::restoreTracks(const boost::shared_ptr<KnobI> & knob,
                                 const std::list<boost::shared_ptr<Natron::Node> > & allNodes)
{
    Double_Knob* isDouble = dynamic_cast<Double_Knob*>( knob.get() );

    if ( isDouble && (isDouble->getName() == "center") && (isDouble->getDimension() == 2) ) {
        isDouble->restoreTracks(slavedTracks,allNodes);
    }
}

void
KnobSerialization::restoreExpressions(const boost::shared_ptr<KnobI> & knob)
{
    int dims = std::min(knob->getDimension(), _knob->getDimension());
    try {
        for (int i = 0; i < dims; ++i) {
            (void)knob->restoreExpression(i, _expressions[i].first, _expressions[i].second);
        }
    } catch (const std::exception& e) {
        QString err = QString("Failed to restore expression on %1: %2").arg(knob->getName().c_str()).arg(e.what());
        appPTR->writeToOfxLog_mt_safe(err);
    }
    
}
