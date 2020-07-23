# libBasicSOFA

A very basic library for reading Spatially Oriented Format for Acoustics (SOFA) files, a format for storing HRTFs and/or BRIRs for binuaral audio reproduction.  
More information on this format can be found in the [SOFA Conventions](https://www.sofaconventions.org/mediawiki/index.php/SOFA_(Spatially_Oriented_Format_for_Acoustics)) website or in the AES standard document, AES69-2015.  For those looking for a more comprehensive library, check out [libMySofa](https://github.com/hoene/libmysofa)


## Library Constraints
### Listeners and Sources
The SOFA specification allows for multiple sound sources and listeners.  A listener can have multiple view vectors (orientation of head).  However, this library only supports one view vector.  A head rotation should be reflected only by the data in ListenerPosition.  Alternatively, multiple sources surrounding the head are reflected by the data in SourcePosition.


### Coordinates
Only spherical coordinates are supported in this library.  **Cartesian coordinates are NOT supported**


## Conventions

The following chart outlines the spherical coordinate system used in the SOFA specification.  
![coordinates](/readme_resources/coordinates.png)


*From the following [paper](https://cesardsalvador.github.io/doc/salvador_2018_near_distance_hrtf_dataset.pdf) by César D. Salvador*


## Building
### Dependencies
libBasicSOFA depends on the [HDF5 API](https://www.hdfgroup.org).  MacOS users can obtain this API through Homebrew:
```shell
brew install hdf5
```


### Build Instructions
A CMake file is in the works.  In the meantime, to build on MacOS, create a new library project, add the libBasicSOFA source files and build.


## Using the Library
The following code snippet demonstrates instantiating a libBasicSOFA object, reading a SOFA file, getting some properties and finally getting an impulse response for a given channel and spherical coordinate (theta, phi, radius):


```c++
BasicSOFA::BasicSOFA sofa;

bool success = sofa.readSofaFile("/path/to/sofa/file.sofa");
if (!success)
    return;
    
auto samplingFrequency = sofa.getFs();
auto numSourcePositions = sofa.getM();

auto theta = 150.0;
auto phi = 0.0;
auto radius = 1.0;
auto channel = 0;

const double *impulseResp = sofa.getHRIR(channel, theta, phi, radius);
if (impulseResp == nullptr)
    return;
```


## Unit Testing
libBasicSOFA uses the [Catch2](https://github.com/catchorg/Catch2) test framework.  Ensure that you have this framework installed before running the tests.


As the SOFA files used in unit testing are large in size, they are not included in this repository.  The supported SOFA file example can be downloaded [here](https://drive.google.com/open?id=1s0GVAG0jt4RZWZEaUOzLjPo5cirXqhLW) while the unsupported SOFA file example can be downloaded [here](https://zenodo.org/record/160749#.Xw6XMy0ZNQI).


To run the unit tests on MacOS, ensure you have a copy of the libBasicSOFA library built.  Create a new XCode project, import the source files in BasicSOFATest into your project, specify the SOFA file paths and run.


