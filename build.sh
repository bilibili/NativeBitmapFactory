#git clone https://android.googlesource.com/platform/external/skia AndroidSource/platform/external/skia
#git clone https://android.googlesource.com/platform/frameworks/base AndroidSource/platform/frameworks/base
#git clone https://android.googlesource.com/platform/system/core AndroidSource/platform/system/core
#git clone https://android.googlesource.com/platform/frameworks/native AndroidSource/platform/frameworks/native

export NDK_ROOT=/cygdrive/d/Android/android-ndk-r8b/ndk-build
export ANDROID_SOURCE_PATH=AndroidSource

function checkout_tags(){
	echo "external/skia: checkout -b $1..."
	cd $ANDROID_SOURCE_PATH/platform/external/skia
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "frameworks/base: checkout -b $1..."
	cd $ANDROID_SOURCE_PATH/platform/frameworks/base
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "system/core: checkout -b $1"
	cd $ANDROID_SOURCE_PATH/platform/system/core
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "frameworks/native: checkout -b $1"
	cd $ANDROID_SOURCE_PATH/platform/frameworks/native
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "====checkout tag ready======"
}

function build_so(){
	echo "-----ndk build libskia.so-------- "
	export API=$1
	$NDK_ROOT
}

function cp_so(){
	mkdir tmp
	cp libs tmp -rf
}

function cp_release_so(){
	rm -rf libs
	mkdir -p libs/armeabi
	yes | cp tmp/libs/armeabi/libndkbitmap*.so libs/armeabi
	mkdir -p libs/armeabi-v7a
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so libs/armeabi-v7a
	mkdir -p libs/x86
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so libs/x86
	mkdir -p libs/mips
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so libs/mips
}

rm -rf tmp

checkout_tags android-sdk-support_r11
build_so 11
cp_so

checkout_tags android-cts-4.0_r1
build_so 14
cp_so

checkout_tags android-4.0.3_r1
build_so 15
cp_so

checkout_tags android-4.1.2_r1
build_so 16
cp_so

checkout_tags android-4.2.2_r1
build_so 17
cp_so

checkout_tags android-4.3_r1
build_so 18
cp_so

checkout_tags android-4.4.2_r1
cp $ANDROID_SOURCE_PATH/platform/external/skia/include/lazy/SkBitmapFactory.h $ANDROID_SOURCE_PATH/platform/external/skia/include/core/SkBitmapFactory.h #file missing
build_so 19
cp_so

cp_release_so