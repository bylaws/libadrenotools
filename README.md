### Adreno Tools
A library for applying rootless Adreno GPU driver modifications/replacements. Currently supports loading custom GPU drivers such as [turnip](https://docs.mesa3d.org/android.html#building-using-the-android-ndk),  enabling BCn textures and redirecting file operations to allow accessing shader dumps and modification of the [driver config file](https://gist.github.com/bylaws/04130932e2634d1c6a2a9729e3940d60) without root.

#### Documentation
API is documented in the `include/adrenotools` headers.

#### Android Build

Use the below command to build the libraries:
<libadrenotools>/android-build>gradlew.bat assemble

The Gradle script copies the libs at:
<libadrenotools>/android-build/build/outputs/libs

linkernsbypass.a library still needs to be manually copied from:
 <libadrenotools>/android-build/.cxx

#### Support
Android 9+
Arm64

Please create an issue if support for anything else is desired.

