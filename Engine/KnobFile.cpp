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

#include "KnobFile.h"

#include <utility>
#include <QtCore/QStringList>
#include <QtCore/QMutexLocker>
#include <QDebug>

#include "Engine/Transform.h"
#include "Engine/StringAnimationManager.h"
#include "Engine/KnobTypes.h"
#include "Engine/Project.h"
#include <SequenceParsing.h>
#include "Global/QtCompat.h"

using namespace Natron;
using std::make_pair;
using std::pair;

/***********************************FILE_KNOB*****************************************/

File_Knob::File_Knob(KnobHolder* holder,
                     const std::string &description,
                     int dimension,
                     bool declaredByPlugin)
    : AnimatingString_KnobHelper(holder, description, dimension,declaredByPlugin)
      , _isInputImage(false)
{
}

File_Knob::~File_Knob()
{
}

bool
File_Knob::canAnimate() const
{
    return true;
}

const std::string File_Knob::_typeNameStr("InputFile");
const std::string &
File_Knob::typeNameStatic()
{
    return _typeNameStr;
}

const std::string &
File_Knob::typeName() const
{
    return typeNameStatic();
}

int
File_Knob::firstFrame() const
{
    double time;
    bool foundKF = getFirstKeyFrameTime(0, &time);

    return foundKF ? (int)time : INT_MIN;
}

int
File_Knob::lastFrame() const
{
    double time;
    bool foundKF = getLastKeyFrameTime(0, &time);

    return foundKF ? (int)time : INT_MAX;
}

int
File_Knob::frameCount() const
{
    return getKeyFramesCount(0);
}

std::string
File_Knob::getFileName(int time) const
{
    int view = getHolder() ? getHolder()->getCurrentView() : 0;
    
    if (!_isInputImage) {
        return getValue();
    } else {
        ///try to interpret the pattern and generate a filename if indexes are found
        return SequenceParsing::generateFileNameFromPattern(getValue(), time, view);
    }
}

/***********************************OUTPUT_FILE_KNOB*****************************************/

OutputFile_Knob::OutputFile_Knob(KnobHolder* holder,
                                 const std::string &description,
                                 int dimension,
                                 bool declaredByPlugin)
    : Knob<std::string>(holder, description, dimension,declaredByPlugin)
      , _isOutputImage(false)
      , _sequenceDialog(true)
{
}

bool
OutputFile_Knob::canAnimate() const
{
    return false;
}

const std::string OutputFile_Knob::_typeNameStr("OutputFile");
const std::string &
OutputFile_Knob::typeNameStatic()
{
    return _typeNameStr;
}

const std::string &
OutputFile_Knob::typeName() const
{
    return typeNameStatic();
}

QString
OutputFile_Knob::generateFileNameAtTime(SequenceTime time) const
{
    int view = getHolder() ? getHolder()->getCurrentView() : 0;
    return SequenceParsing::generateFileNameFromPattern(getValue(0), time, view).c_str();
}

/***********************************PATH_KNOB*****************************************/

Path_Knob::Path_Knob(KnobHolder* holder,
                     const std::string &description,
                     int dimension,
                     bool declaredByPlugin)
    : Knob<std::string>(holder,description,dimension,declaredByPlugin)
      , _isMultiPath(false)
{
}

const std::string Path_Knob::_typeNameStr("Path");
const std::string &
Path_Knob::typeNameStatic()
{
    return _typeNameStr;
}

bool
Path_Knob::canAnimate() const
{
    return false;
}

const std::string &
Path_Knob::typeName() const
{
    return typeNameStatic();
}

void
Path_Knob::setMultiPath(bool b)
{
    _isMultiPath = b;
}

bool
Path_Knob::isMultiPath() const
{
    return _isMultiPath;
}

void
Path_Knob::getVariables(std::list<std::pair<std::string,std::string> >* paths) const
{
    if (!_isMultiPath) {
        return;
    }
    
    std::string startNameTag(NATRON_ENV_VAR_NAME_START_TAG);
    std::string endNameTag(NATRON_ENV_VAR_NAME_END_TAG);
    std::string startValueTag(NATRON_ENV_VAR_VALUE_START_TAG);
    std::string endValueTag(NATRON_ENV_VAR_VALUE_END_TAG);
    
    std::string raw = getValue().c_str();
    size_t i = raw.find(startNameTag);
    while (i != std::string::npos) {
        i += startNameTag.size();
        assert(i < raw.size());
        size_t endNamePos = raw.find(endNameTag,i);
        assert(endNamePos != std::string::npos && endNamePos < raw.size());
        
        std::string name,value;
        while (i < endNamePos) {
            name.push_back(raw[i]);
            ++i;
        }
        
        i = raw.find(startValueTag,i);
        i += startValueTag.size();
        assert(i != std::string::npos && i < raw.size());
        
        size_t endValuePos = raw.find(endValueTag,i);
        assert(endValuePos != std::string::npos && endValuePos < raw.size());
        
        while (i < endValuePos) {
            value.push_back(raw.at(i));
            ++i;
        }
        
        // In order to use XML tags, the text inside the tags has to be unescaped.
        paths->push_back(std::make_pair(name,Project::unescapeXML(value).c_str()));
        
        i = raw.find(startNameTag,i);
    }
}


void
Path_Knob::getPaths(std::list<std::string> *paths) const
{
    std::string raw = getValue().c_str();
    
    if (_isMultiPath) {
        std::list<std::pair<std::string,std::string> > ret;
        getVariables(&ret);
        for (std::list<std::pair<std::string,std::string> >::iterator it = ret.begin(); it != ret.end(); ++it) {
            paths->push_back(it->second);
        }
    } else {
        paths->push_back(raw);
    }
    
}

void
Path_Knob::setPaths(const std::list<std::pair<std::string,std::string> >& paths)
{
    if (!_isMultiPath) {
        return;
    }
    
    std::string path;
    

    for (std::list<std::pair<std::string,std::string> >::const_iterator it = paths.begin(); it != paths.end(); ++it) {
        // In order to use XML tags, the text inside the tags has to be escaped.
        path += NATRON_ENV_VAR_NAME_START_TAG;
        path += Project::escapeXML(it->first);
        path += NATRON_ENV_VAR_NAME_END_TAG;
        path += NATRON_ENV_VAR_VALUE_START_TAG;
        path += Project::escapeXML(it->second);
        path += NATRON_ENV_VAR_VALUE_END_TAG;
    }
    setValue(path, 0);
}

std::string
Path_Knob::generateUniquePathID(const std::list<std::pair<std::string,std::string> >& paths)
{
    std::string baseName("Path");
    int idx = 0;
    
    bool found;
    std::string name;
    do {
        
        std::stringstream ss;
        ss << baseName;
        ss << idx;
        name = ss.str();
        found = false;
        for (std::list<std::pair<std::string,std::string> >::const_iterator it = paths.begin(); it != paths.end(); ++it) {
            if (it->first == name) {
                found = true;
                break;
            }
        }
        ++idx;
    } while (found);
    return name;
}

void
Path_Knob::prependPath(const std::string& path)
{
    if (!_isMultiPath) {
        setValue(path, 0);
    } else {
        std::list<std::pair<std::string,std::string> > paths;
        getVariables(&paths);
        std::string name = generateUniquePathID(paths);
        paths.push_front(std::make_pair(name, path));
        setPaths(paths);
    }
}

void
Path_Knob::appendPath(const std::string& path)
{
    if (!_isMultiPath) {
        setValue(path, 0);
    } else {
        std::list<std::pair<std::string,std::string> > paths;
        getVariables(&paths);
        for (std::list<std::pair<std::string,std::string> >::iterator it = paths.begin(); it!=paths.end(); ++it) {
            if (it->second == path) {
                return;
            }
        }
        std::string name = generateUniquePathID(paths);
        paths.push_back(std::make_pair(name, path));
        setPaths(paths);
    }
}
