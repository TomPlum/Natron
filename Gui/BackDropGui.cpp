//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>

#include "BackDropGui.h"

#include <algorithm> // min, max

CLANG_DIAG_OFF(deprecated)
CLANG_DIAG_OFF(uninitialized)
#include <QGraphicsTextItem>
#include <QCoreApplication>
#include <QThread>
CLANG_DIAG_ON(deprecated)
CLANG_DIAG_ON(uninitialized)

#include "Engine/KnobTypes.h"
#include "Engine/Node.h"
#include "Engine/BackDrop.h"

#include "Gui/KnobGuiTypes.h"

#define RESIZE_HANDLE_SIZE 20

#define NATRON_BACKDROP_DEFAULT_WIDTH 80
#define NATRON_BACKDROP_DEFAULT_HEIGHT 80

struct BackDropGuiPrivate
{
    BackDropGui* _publicInterface;
    
    QGraphicsTextItem* label;

    BackDropGuiPrivate(BackDropGui* publicInterface)
    : _publicInterface(publicInterface)
    , label(0)
    {
    }

    void refreshLabelText(int nameHeight,const QString & text);
    
    std::string getLabelValue() const;
    
};

BackDropGui::BackDropGui(QGraphicsItem* parent)
    : NodeGui(parent)
    , _imp( new BackDropGuiPrivate(this) )
{
}


BackDropGui::~BackDropGui()
{
    
}



std::string
BackDropGuiPrivate::getLabelValue() const
{
    boost::shared_ptr<KnobI> k = _publicInterface->getNode()->getKnobByName("Label");
    assert(k);
    String_Knob* isStr = dynamic_cast<String_Knob*>(k.get());
    assert(isStr);
    return isStr->getValue();
}

void
BackDropGui::getInitialSize(int *w, int *h) const
{
    *w = NATRON_BACKDROP_DEFAULT_WIDTH;
    *h = NATRON_BACKDROP_DEFAULT_HEIGHT;
}

void
BackDropGui::createGui()
{
    NodeGui::createGui();
    
    _imp->label = new QGraphicsTextItem("",this);
    _imp->label->setDefaultTextColor( QColor(0,0,0,255) );
    _imp->label->setZValue(getBaseDepth() + 1);
    
    Natron::EffectInstance* effect = dynamic_cast<Natron::EffectInstance*>(getNode()->getLiveInstance());
    assert(effect);
    BackDrop* isBd = dynamic_cast<BackDrop*>(effect);
    assert(isBd);
    
    QObject::connect(isBd,SIGNAL(labelChanged(QString)),this, SLOT(onLabelChanged(QString)));
    
    refreshTextLabelFromKnob();
}

void
BackDropGui::onLabelChanged(const QString& label)
{
    int nameHeight = getFrameNameHeight();
    _imp->refreshLabelText(nameHeight, label);
}


void
BackDropGui::adjustSizeToContent(int *w,int *h,bool /*adjustToTextSize*/)
{
    NodeGui::adjustSizeToContent(w, h,false);
    QRectF labelBbox = _imp->label->boundingRect();
    
    *h = std::max((double)*h,labelBbox.height() * 1.5);
    *w = std::max((double)*w, _imp->label->textWidth());
    
}

void
BackDropGui::resizeExtraContent(int /*w*/,int /*h*/,bool forceResize)
{
    QPointF p = pos();
    QPointF thisItemPos = mapFromParent(p);
    
    int nameHeight = getFrameNameHeight();
    
    _imp->label->setPos(thisItemPos.x(), thisItemPos.y() + nameHeight + 10);
    if (!forceResize) {
        _imp->label->adjustSize();
    }
}

void
BackDropGui::refreshTextLabelFromKnob()
{
    int nameHeight = getFrameNameHeight();
    _imp->refreshLabelText( nameHeight, QString( _imp->getLabelValue().c_str() ) );
}

void
BackDropGuiPrivate::refreshLabelText(int nameHeight,const QString &text)
{
    QString textLabel = text;

    textLabel.replace("\n", "<br>");
    textLabel.prepend("<div align=\"left\">");
    textLabel.append("</div>");
    QFont f;
    QColor color;
    if (!text.isEmpty()) {
        String_KnobGui::parseFont(textLabel, &f, &color);
        label->setFont(f);
    }
    
    label->setHtml(textLabel);

    
    QRectF bbox = _publicInterface->boundingRect();
    
    //label->adjustSize();
    int w = std::max( bbox.width(), label->textWidth() * 1.2 );
    QRectF labelBbox = label->boundingRect();
    int h = std::max( labelBbox.height() + nameHeight + 10, bbox.height() );
    _publicInterface->resize(w, h);
    _publicInterface->update();
    
}

