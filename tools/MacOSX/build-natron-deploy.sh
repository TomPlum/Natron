#!/bin/bash
# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron <http://www.natron.fr/>,
# Copyright (C) 2015 INRIA and Alexandre Gauthier
#
# Natron is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Natron is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
# ***** END LICENSE BLOCK *****

if [ $# -ne 1 ]; then
    echo "$0: Make a Natron.app that doesn't depend on MacPorts (can be used out of the build system too)"
    echo "Usage: $0 App/Natron.app"
    exit 1
fi
package="$1"
if [ ! -d "$package" ]; then
    echo "Error: application directory '$package' does not exist"
    exit 1
fi

MACPORTS="/opt/local"
HOMEBREW="/brew2/local"
LOCAL="/usr/local"
PYVER="2.7"
SBKVER="1.2"

macdeployqt "${package}" || exit 1

binary="$package/Contents/MacOS/Natron"
libdir="Frameworks"
pkglib="$package/Contents/$libdir"

if [ ! -x "$binary" ]; then
   echo "Error: $binary does not exist or is not an executable"
   exit 1
fi

#Copy and change exec_path of the whole Python framework with libraries
rm -rf "$pkglib/Python.framework"
mkdir -p "$pkglib/Python.framework/Versions/${PYVER}/lib"
cp -r "${MACPORTS}/Library/Frameworks/Python.framework/Versions/${PYVER}/lib/python${PYVER}" "$pkglib/Python.framework/Versions/${PYVER}/lib/python${PYVER}" || exit 1
rm -rf "$pkglib/Python.framework/Versions/${PYVER}/Resources"
cp -r "${MACPORTS}/Library/Frameworks/Python.framework/Versions/${PYVER}/Resources" "$pkglib/Python.framework/Versions/${PYVER}/Resources" || exit 1
rm -rf "$pkglib/Python.framework/Versions/${PYVER}/Python"
cp "${MACPORTS}/Library/Frameworks/Python.framework/Versions/${PYVER}/Python" "$pkglib/Python.framework/Versions/${PYVER}/Python" || exit 1
chmod 755 "$pkglib/Python.framework/Versions/${PYVER}/Python"
install_name_tool -id "@executable_path/../Frameworks/Python.framework/Versions/${PYVER}/Python" "$pkglib/Python.framework/Versions/${PYVER}/Python"
ln -sf "Versions/${PYVER}/Python" "$pkglib/Python.framework/Python" || exit 1

rm -rf "$pkglib/Python.framework/Versions/${PYVER}/lib/python${PYVER}/site-packages/"*
#rm -rf "$pkglib/Python.framework/Versions/${PYVER}/lib/python${PYVER}/__pycache__"
#rm -rf "$pkglib/Python.framework/Versions/${PYVER}/lib/python${PYVER}/*/__pycache__"
#FILES=`ls -l "${MACPORTS}/Library/Frameworks/Python.framework/Versions/${PYVER}/lib|awk" '{print $9}'`
#for f in FILES; do
#    #FILE=echo "{$f}" | sed "s/cpython-34.//g"
#    cp -r "$f" "$pkglib/Python.framework/Versions/${PYVER}/lib/$FILE" || exit 1
#done

# a few elements of Natron.app/Contents/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-dynload may load other libraries
DYNLOAD="$pkglib/Python.framework/Versions/${PYVER}/lib/python${PYVER}/lib-dynload"
if [ ! -d "${DYNLOAD}" ]; then
    echo "lib-dynload not present"
    exit 1
fi
for mplib in `for i in "${DYNLOAD}"/*.so; do otool -L $i | fgrep "${MACPORTS}"; done |sort|uniq |awk '{print $1}'`; do
    if [ ! -f "$mplib" ]; then
        echo "missing python lib-dynload depend $mplib"
        exit 1
    fi
    lib=`echo $mplib | awk -F / '{print $NF}'`
    if [ ! -f "$pkglib/${lib}" ]; then
        cp "$mplib" "$pkglib/${lib}"
    fi
    for deplib in "${DYNLOAD}"/*.so; do
        install_name_tool -change "${mplib}" "@executable_path/../Frameworks/$lib" "$deplib"
    done
done


# macdeployqt doesn't deal correctly with libs in ${MACPORTS}/lib/libgcc : handle them manually
LIBGCC=0
if otool -L "$binary" |fgrep "${MACPORTS}/lib/libgcc"; then
    LIBGCC=1
fi
if [ "$LIBGCC" = "1" ]; then
    for l in gcc_s.1 gomp.1 stdc++.6; do
        lib=lib${l}.dylib
        cp "${MACPORTS}/lib/libgcc/$lib" "$pkglib/$lib"
        install_name_tool -id "@executable_path/../Frameworks/$lib" "$pkglib/$lib"
    done
    for l in gcc_s.1 gomp.1 stdc++.6; do
        lib=lib${l}.dylib
        install_name_tool -change "${MACPORTS}/lib/libgcc/$lib" "@executable_path/../Frameworks/$lib" "$binary"
        for deplib in "$pkglib/"*.dylib; do
            install_name_tool -change "${MACPORTS}/lib/libgcc/$lib" "@executable_path/../Frameworks/$lib" "$deplib"
        done
    done
    # use gcc's libraries everywhere
    for l in gcc_s.1 gomp.1 stdc++.6; do
        lib="lib${l}.dylib"
        for deplib in "$pkglib/"*.framework/Versions/*/* "$pkglib/"lib*.dylib; do
            test -f "$deplib" && install_name_tool -change "/usr/lib/$lib" "@executable_path/../Frameworks/$lib" "$deplib"
        done
    done
fi
for f in Python; do
    install_name_tool -change "${MACPORTS}/Library/Frameworks/${f}.framework/Versions/${PYVER}/${f}" "@executable_path/../Frameworks/${f}.framework/Versions/${PYVER}/${f}" "$binary"
done

if otool -L "${package}/Contents/MacOS/Natron"  |fgrep "${MACPORTS}"; then
    echo "Error: MacPorts libraries remaining in $binary, please check"
    exit 1
fi

mv "${package}/Contents/PlugIns" "${package}/Contents/Plugins" || exit 1
rm "${package}/Contents/Resources/qt.conf" || exit 1

#Make a qt.conf file in Contents/Resources/
cat > "${package}/Contents/Resources/qt.conf" <<EOF
[Paths]
Plugins = Plugins
EOF

cp "Gui/Resources/Stylesheets/mainstyle.qss" "${package}/Contents/Resources/" || exit 1

cp "Renderer/NatronRenderer" "${package}/Contents/MacOS"
binary="${package}/Contents/MacOS/NatronRenderer"

#Change @executable_path for NatronRenderer deps
for l in boost_serialization-mt boost_thread-mt boost_system-mt expat.1 cairo.2 pyside-python${PYVER}.${SBKVER} shiboken-python${PYVER}.${SBKVER} intl.8; do
    lib=lib${l}.dylib
    install_name_tool -change "${MACPORTS}/lib/$lib" "@executable_path/../Frameworks/$lib" "$binary"
done
for f in QtNetwork QtCore; do
    install_name_tool -change "${MACPORTS}/Library/Frameworks/${f}.framework/Versions/4/${f}" "@executable_path/../Frameworks/${f}.framework/Versions/4/${f}" "$binary"
done
if [ "$LIBGCC" = "1" ]; then
    for l in gcc_s.1 stdc++.6; do
        lib=lib${l}.dylib
        install_name_tool -change "${MACPORTS}/lib/libgcc/$lib" "@executable_path/../Frameworks/$lib" "$binary"
    done
fi

#Copy and change exec_path of the whole Python framework with libraries
for f in Python; do
    install_name_tool -change "${MACPORTS}/Library/Frameworks/${f}.framework/Versions/${PYVER}/${f}" "@executable_path/../Frameworks/${f}.framework/Versions/${PYVER}/${f}" "$binary"
done

if otool -L "$binary" |fgrep "${MACPORTS}"; then
    echo "Error: MacPorts libraries remaining in $binary, please check"
    exit 1
fi



#Do the same for crash reporter
if [ -f "CrashReporter/NatronCrashReporter" ]; then
    binary="${package}/Contents/MacOS/NatronCrashReporter"
    cp "CrashReporter/NatronCrashReporter" "$binary"
    for f in QtGui QtNetwork QtCore; do
        install_name_tool -change "${MACPORTS}/Library/Frameworks/${f}.framework/Versions/4/${f}" "@executable_path/../Frameworks/${f}.framework/Versions/4/${f}" "$binary"
    done
    if [ "$LIBGCC" = "1" ]; then
        for l in gcc_s.1 gomp.1 stdc++.6; do
            lib="lib${l}.dylib"
            install_name_tool -change "${MACPORTS}/lib/libgcc/$lib" "@executable_path/../Frameworks/$lib" "$binary"
        done
    fi

    if otool -L "$binary" |fgrep "${MACPORTS}"; then
        echo "Error: MacPorts libraries remaining in $binary, please check"
        exit 1
    fi
fi

if [ -f "CrashReporterCLI/NatronRendererCrashReporter" ]; then
    binary="${package}/Contents/MacOS/NatronRendererCrashReporter"
    cp "CrashReporterCLI/NatronRendererCrashReporter" "$binary"
    for f in QtNetwork QtCore; do
        install_name_tool -change "${MACPORTS}/Library/Frameworks/${f}.framework/Versions/4/${f}" "@executable_path/../Frameworks/${f}.framework/Versions/4/${f}" "$binary"
    done
    if [ "$LIBGCC" = "1" ]; then
        for l in gcc_s.1 gomp.1 stdc++.6; do
            lib=lib${l}.dylib
            install_name_tool -change "${MACPORTS}/lib/libgcc/$lib" "@executable_path/../Frameworks/$lib" "$binary"
        done
    fi

    if otool -L "$binary" |fgrep "${MACPORTS}"; then
        echo "Error: MacPorts libraries remaining in $binary, please check"
        exit 1
    fi
fi

# install PySide in site-packages
PYSIDE="Frameworks/Python.framework/Versions/${PYVER}/lib/python${PYVER}/site-packages/PySide"
rm -rf "${package}/Contents/${PYSIDE}"
cp -r "${MACPORTS}/Library/${PYSIDE}" "${package}/Contents/${PYSIDE}" || exit 1

QT_LIBS="QtCore QtGui QtNetwork QtOpenGL QtDeclarative QtHelp QtMultimedia QtScript QtScriptTools QtSql QtSvg QtTest QtUiTools QtXml QtWebKit QtXmlPatterns"

for qtlib in $QT_LIBS ;do
    binary="${package}/Contents/${PYSIDE}/${qtlib}.so"
    install_name_tool -id "@executable_path/../${PYSIDE}/${qtlib}.so" "$binary"
    for f in $QT_LIBS; do
        install_name_tool -change "${MACPORTS}/Library/Frameworks/${f}.framework/Versions/4/${f}" "@executable_path/../Frameworks/${f}.framework/Versions/4/${f}" "$binary"
    done

    for l in  pyside-python${PYVER}.${SBKVER} shiboken-python${PYVER}.${SBKVER}; do
        dylib="lib${l}.dylib"
        install_name_tool -change "${MACPORTS}/lib/$dylib" "@executable_path/../Frameworks/$dylib" "$binary"
    done
    if [ "$LIBGCC" = "1" ]; then
        for l in gcc_s.1 gomp.1 stdc++.6; do
            lib="lib${l}.dylib"
            install_name_tool -change "/usr/lib/$lib" "@executable_path/../Frameworks/$lib" "$binary"
        done
    fi
done



for bin in Natron NatronRenderer NatronCrashReporter NatronRendererCrashReporter; do
    binary="$package/Contents/MacOS/$bin"
    if [ -f "$binary" ]; then
        rpath=`otool -l $binary | grep -A 3 LC_RPATH |grep path|awk '{ print $2 }'`
        if [[ ! ("$rpath" == *"@loader_path/../$libdir"*) ]]; then
            echo "Error:: The runtime search path in $binary does not contain \"@loader_path/../$libdir\". Please set it in your Xcode project, or link the binary with the flags -Xlinker -rpath -Xlinker \"@loader_path/../$libdir\""
            exit 1
        fi
        # Test dirs
        test -d "$pkglib" || mkdir "$pkglib"

        LIBADD=

        #############################
        # test if ImageMagick is used
        if otool -L "$binary"  | fgrep libMagick > /dev/null; then
            # Check that ImageMagick is properly installed
            if ! pkg-config --modversion ImageMagick >/dev/null 2>&1; then
	        echo "Missing ImageMagick -- please install ImageMagick ('sudo port install ImageMagick +no_x11 +universal') and try again." >&2
	        exit 1
            fi

            # Update the ImageMagick path in startup script.
            IMAGEMAGICKVER=`pkg-config --modversion ImageMagick`
            IMAGEMAGICKMAJ=${IMAGEMAGICKVER%.*.*}
            IMAGEMAGICKLIB=`pkg-config --variable=libdir ImageMagick`
            IMAGEMAGICKSHARE=`pkg-config --variable=prefix ImageMagick`/share
            # if I get this right, sed substitutes in the exe the occurences of IMAGEMAGICKVER
            # into the actual value retrieved from the package.
            # We don't need this because we use MAGICKCORE_PACKAGE_VERSION declared in the <magick/magick-config.h>
            # sed -e "s,IMAGEMAGICKVER,$IMAGEMAGICKVER,g" -i "" $pkgbin/DisparityKillerM

            # copy the ImageMagick libraries (.la and .so)
            cp -r "$IMAGEMAGICKLIB/ImageMagick-$IMAGEMAGICKVER" "$pkglib/"
            test -d "$pkglib/share" || mkdir "$pkglib/share"
            cp -r "$IMAGEMAGICKSHARE/ImageMagick-$IMAGEMAGICKMAJ" "$pkglib/share/"

            LIBADD="$LIBADD $pkglib/ImageMagick-$IMAGEMAGICKVER/modules-*/*/*.so"
            WITH_IMAGEMAGICK=yes
        fi

        # expand glob patterns in LIBADD
        LIBADD=`echo $LIBADD`

        # Find out the library dependencies
        # (i.e. $LOCAL or $MACPORTS), then loop until no changes.
        a=1
        nfiles=0
        alllibs=""
        endl=true
        while $endl; do
	    #echo -e "\033[1mLooking for dependencies.\033[0m Round" $a
	    libs="`otool -L $pkglib/* $LIBADD $binary 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep -e $LOCAL'\\|'$HOMEBREW'\\|'$MACPORTS | sort | uniq`"
	    if [ -n "$libs" ]; then
                cp -f $libs $pkglib
	        alllibs="`ls $alllibs $libs | sort | uniq`"
	    fi
	    let "a+=1"	
	    nnfiles=`ls $pkglib | wc -l`
	    if [ $nnfiles = $nfiles ]; then
		endl=false
	    else
		nfiles=$nnfiles
	    fi
        done

        # all the libraries were copied, now change the names...
        ## We use @rpath instead of @executable_path/../$libdir because it's shorter
        ## than /opt/local, so it always works. The downside is that the XCode project
        ## has to link the binary with "Runtime search paths" set correctly
        ## (e.g. to "@loader_path/../Frameworks @loader_path/../Libraries" ),
        ## or the binary has to be linked with the following flags:
        ## -Wl,-rpath,@loader_path/../Frameworks -Wl,-rpath,@loader_path/../Libraries
        if [ -n "$alllibs" ]; then
            changes=""
            for l in $alllibs; do
                changes="$changes -change $l @rpath/`basename $l`"
            done

            for f in  $pkglib/* $LIBADD "$binary"; do
                # avoid directories
                if [ -f "$f" ]; then
                    chmod +w "$f"
                    if ! install_name_tool $changes "$f"; then
                        echo "Error: 'install_name_tool $changes $f' failed"
                        exit 1
                    fi
                    install_name_tool -id @rpath/`basename $f` "$f"
                    if ! install_name_tool -id @rpath/`basename $f` "$f"; then
                        echo "Error: 'install_name_tool -id @rpath/`basename $f` $f' failed"
                        exit 1
                    fi
                fi
            done
        fi

        # and now, obfuscate all the default paths in dynamic libraries
        # and ImageMagick modules and config files

        # generate a pseudo-random string which has the same length as $MACPORTS
        RANDSTR="R7bUU6jiFvqrPy6zLVPwIC3b93R2b1RG2qD3567t8hC3b93R2b1RG2qD3567t8h"
        MACRAND=${RANDSTR:0:${#MACPORTS}}
        HOMEBREWRAND=${RANDSTR:0:${#HOMEBREW}}
        LOCALRAND=${RANDSTR:0:${#LOCAL}}
        find $pkglib -type f -exec sed -e "s@$MACPORTS@$MACRAND@g" -e "s@$HOMEBREW@$HOMEBREWRAND@g" -e "s@$LOCAL@$LOCALRAND@g" -i "" {} \;
        sed -e "s@$MACPORTS@$MACRAND@g" -e "s@$HOMEBREW@$HOMEBREWRAND@g" -e "s@$LOCAL@$LOCALRAND@g" -i "" "$binary"

    fi
done
