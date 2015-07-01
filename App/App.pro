#This Source Code Form is subject to the terms of the Mozilla Public
#License, v. 2.0. If a copy of the MPL was not distributed with this
#file, You can obtain one at http://mozilla.org/MPL/2.0/.


TARGET = Natron
VERSION = 2.0.0
TEMPLATE = app
CONFIG += app
CONFIG += moc
CONFIG += boost glew opengl qt expat cairo python shiboken pyside
QT += gui core opengl network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent

macx {
  ### custom variables for the Info.plist file
  # use a custom Info.plist template
  QMAKE_INFO_PLIST = NatronInfo.plist
  # Set the application icon
  ICON = ../Gui/Resources/Images/natronIcon256_osx.icns
  # replace com.yourcompany with something more meaningful
  QMAKE_TARGET_BUNDLE_PREFIX = fr.inria
  QMAKE_PKGINFO_TYPEINFO = Ntrn
}

#OpenFX C api includes and OpenFX c++ layer includes that are located in the submodule under /libs/OpenFX
INCLUDEPATH += $$PWD/../libs/OpenFX/include
INCLUDEPATH += $$PWD/../libs/OpenFX_extensions
INCLUDEPATH += $$PWD/../libs/OpenFX/HostSupport/include
INCLUDEPATH += $$PWD/..

#System library is required on windows to map network share names from drive letters
win32 {
    LIBS += mpr.lib
}

win32-msvc* {
	CONFIG(64bit) {
		QMAKE_LFLAGS += /MACHINE:X64
	} else {
		QMAKE_LFLAGS += /MACHINE:X86
	}
        QMAKE_LFLAGS += /ENTRY:"mainCRTStartup"
}

################
# Gui


win32-msvc*{
        CONFIG(64bit) {
                CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Gui/x64/release/ -lGui
                CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Gui/x64/debug/ -lGui
        } else {
                CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Gui/win32/release/ -lGui
                CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Gui/win32/debug/ -lGui
        }
} else {
        win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Gui/release/ -lGui
        else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Gui/debug/ -lGui
        else:*-xcode:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Gui/build/Release/ -lGui
        else:*-xcode:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Gui/build/Debug/ -lGui
        else:unix {
            LIBS += -L$$OUT_PWD/../Gui/ -lGui
            # GLU is required by ViewerGL, but some versions of glew don't link it (e.g. Ubuntu 12.04)
            !macx: LIBS += -lGLU
        }
}
INCLUDEPATH += $$PWD/../Gui
DEPENDPATH += $$PWD/../Gui

win32-msvc*{
        CONFIG(64bit) {
                CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/x64/release/libGui.lib
                CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/x64/debug/libGui.lib
        } else {
                CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/win32/release/libGui.lib
                CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/win32/debug/libGui.lib
        }
} else {
        win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/release/libGui.a
        else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/debug/libGui.a
        else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/release/Gui.lib
        else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/debug/Gui.lib
        else:*-xcode:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/build/Release/libGui.a
        else:*-xcode:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Gui/build/Debug/libGui.a
        else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Gui/libGui.a
}
################
# Engine

win32-msvc*{
	CONFIG(64bit) {
		CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Engine/x64/release/ -lEngine
		CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Engine/x64/debug/ -lEngine
	} else {
		CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Engine/win32/release/ -lEngine
		CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Engine/win32/debug/ -lEngine
	}
} else {
	win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Engine/release/ -lEngine
	else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Engine/debug/ -lEngine
	else:*-xcode:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Engine/build/Release/ -lEngine
	else:*-xcode:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Engine/build/Debug/ -lEngine
	else:unix: LIBS += -L$$OUT_PWD/../Engine/ -lEngine
}

INCLUDEPATH += $$PWD/../Engine
DEPENDPATH += $$PWD/../Engine

win32-msvc*{
	CONFIG(64bit) {
		CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/x64/release/libEngine.lib
		CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/x64/debug/libEngine.lib
	} else {
		CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/win32/release/libEngine.lib
		CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/win32/debug/libEngine.lib
	}
} else {
	win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/release/libEngine.a
	else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/debug/libEngine.a
	else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/release/Engine.lib
	else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/debug/Engine.lib
	else:*-xcode:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/build/Release/libEngine.a
	else:*-xcode:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Engine/build/Debug/libEngine.a
	else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Engine/libEngine.a
}

################
# HostSupport

win32-msvc*{
	CONFIG(64bit) {
		CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/x64/release/ -lHostSupport
		CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/x64/debug/ -lHostSupport
	} else {
		CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/win32/release/ -lHostSupport
		CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/win32/debug/ -lHostSupport
	}
} else {
	win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/release/ -lHostSupport
	else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/debug/ -lHostSupport
	else:*-xcode:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/build/Release/ -lHostSupport
	else:*-xcode:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../HostSupport/build/Debug/ -lHostSupport
	else:unix: LIBS += -L$$OUT_PWD/../HostSupport/ -lHostSupport
}

INCLUDEPATH += $$PWD/../HostSupport
DEPENDPATH += $$PWD/../HostSupport

win32-msvc*{
	CONFIG(64bit) {
		CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/x64/release/libHostSupport.lib
		CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/x64/debug/libHostSupport.lib
	} else {
		CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/win32/release/libHostSupport.lib
		CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/win32/debug/libHostSupport.lib
	}
} else {
	win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/release/libHostSupport.a
	else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/debug/libHostSupport.a
	else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/release/HostSupport.lib
	else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/debug/HostSupport.lib
	else:*-xcode:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/build/Release/libHostSupport.a
	else:*-xcode:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/build/Debug/libHostSupport.a
	else:unix: PRE_TARGETDEPS += $$OUT_PWD/../HostSupport/libHostSupport.a
}


################
# LibMV


win32-msvc*{
        CONFIG(64bit) {
                CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libmv/x64/release/ -lLibMV
                CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libmv/x64/debug/ -lLibMV
        } else {
                CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libmv/win32/release/ -lLibMV
                CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libmv/win32/debug/ -lLibMV
        }
} else {
        win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libmv/release/ -lLibMV
        else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libmv/debug/ -lLibMV
        else:*-xcode:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libmv/build/Release/ -lLibMV
        else:*-xcode:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libmv/build/Debug/ -lLibMV
        else:unix: LIBS += -L$$OUT_PWD/../libmv/ -lLibMV
}

win32-msvc*{
        CONFIG(64bit) {
                CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/x64/release/libLibMV.lib
                CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/x64/debug/libLibMV.lib
        } else {
                CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/win32/release/libLibMV.lib
                CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/win32/debug/libLibMV.lib
        }
} else {
        win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/release/libLibMV.a
        else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/debug/libLibMV.a
        else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/release/libLibMV.lib
        else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/debug/libLibMV.lib
        else:*-xcode:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/build/Release/libLibMV.a
        else:*-xcode:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libmv/build/Debug/libLibMV.a
        else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libmv/libLibMV.a
}

include(../global.pri)
include(../config.pri)

SOURCES += \
    NatronApp_main.cpp

INSTALLS += target

Resources.files = $$PWD/../Gui/Resources/OpenColorIO-Configs
macx {
    Resources.path = Contents/Resources
    QMAKE_BUNDLE_DATA += Resources
}
!macx {
    Resources.path = $$OUT_PWD
    INSTALLS += Resources
}

