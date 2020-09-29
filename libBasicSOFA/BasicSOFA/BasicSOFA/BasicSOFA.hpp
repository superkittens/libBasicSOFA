//
//  BasicSOFA.hpp
//  BasicSOFA
//
//  Created by Allen Lee on 2020-07-09.
//  Copyright © 2020 meoWorkshop. All rights reserved.
//

#ifndef BasicSOFA_
#define BasicSOFA_

#include <H5Cpp.h>
#include <vector>
#include <unordered_map>
#include <stdio.h>

#define SOFA_STANDARD_STRING    "AES69-2015"
#define SUPPORTED_VERSION       "1.0"
#define LIBBASICSOFA_VERSION    "1.2"

#define SOFA_HRIR_STRING        "Data.IR"
#define SOFA_FS_STRING          "Data.SamplingRate"
#define SOFA_M_STRING           "M"
#define SOFA_N_STRING           "N"
#define SOFA_R_STRING           "R"
#define SOFA_C_STRING           "C"
#define SOFA_SRC_POS_STRING     "SourcePosition"
#define SOFA_LIS_POS_STRING     "ListenerPosition"



/* The classes below are exported */
#pragma GCC visibility push(default)

namespace BasicSOFA
{
    
    struct SOFACoordinateMap
    {
        double                                          radius;
        std::vector<std::vector<size_t>>                map;
        std::unordered_map<double, size_t>              phiMap;
        std::vector<std::unordered_map<double, size_t>> thetaMaps;
        
        SOFACoordinateMap (double radius) { this->radius = radius; }
    };

    
    class BasicSOFA
    {
    public:
        
        void            HelloWorld (const char *);
        
                        BasicSOFA();
        bool            readSOFAFile (std::string filePath);
        const double*   getHRIR (size_t channel, double theta, double phi, double radius) const noexcept;
        
        double          getFs () const { return fs; }
        double          getM () const { return M; }
        double          getN () const { return N; }
        double          getR () const { return R; }
        double          getC () const { return C; }
        
        double          getMinRadius () const { return minRadius; }
        double          getMaxRadius () const { return maxRadius; }
        double          getDeltaRadius () const { return dRadius; }
        double          getMinPhi () const { return minPhi; }
        double          getMaxPhi () const { return maxPhi; }
        double          getDeltaPhi () const { return dPhi; }
        double          getMinTheta () const { return minTheta; }
        double          getMaxTheta () const { return maxTheta; }
        double          getDeltaTheta () const { return dTheta; }
        
        size_t          getMinImpulseDelay () const { return minImpulseDelay; }
        
        void            resetSOFAData ();
        
        
        
    protected:
        
        double                  round (const double &x) const;
        void                    addValueToArray (const double &x, std::vector<double> &A);
        bool                    calculateCoordinateStatisticalData ();
        std::vector<double>     getCoordinatesFromSOFAFile ();
        bool                    buildCoordinateMap (const std::vector<double> &coordinates);
        hsize_t                 getSOFASingleDimParameterSize(std::string parameter);
        bool                    findMinImpulseDelay();
        
        
        H5::H5File  h5File;
        
        //  SOFA file measurement properties
        //  More information about these constants can be found in the standards manual
        //  Not constants are read at the moment since BasicSOFA does not support multiple emitters
        double      fs;
        hsize_t     M;      //  Number of measurement points
        hsize_t     N;      //  Number of samples per measurement
        hsize_t     R;      //  Number of receivers
        hsize_t     C;      //  Number of values in coordinate triplet...so should be 3.  Maybe they might change this in future revisions?
        
        
        //  Statistical data on SOFA properties
        double                              minTheta;
        double                              maxTheta;
        double                              dTheta;
        double                              minPhi;
        double                              maxPhi;
        double                              dPhi;
        double                              minRadius;
        double                              maxRadius;
        double                              dRadius;
        size_t                              minImpulseDelay;
        std::vector<double>                 thetaList;
        std::vector<double>                 phiList;
        std::vector<double>                 radiusList;
        
        std::vector<double>                 hrir;
        std::vector<SOFACoordinateMap>      coordinateMaps;
        std::unordered_map<double, size_t>  radiusMap;
        
        static constexpr double             epsilon = 0.1;
        
        bool                                dataLoaded;
    };
}

#pragma GCC visibility pop
#endif
