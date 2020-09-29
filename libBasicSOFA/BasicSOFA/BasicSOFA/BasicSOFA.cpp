//
//  BasicSOFA.cpp
//  BasicSOFA
//
//  Created by Allen Lee on 2020-07-09.
//  Copyright © 2020 meoWorkshop. All rights reserved.
//

#include <iostream>
#include "BasicSOFA.hpp"
#include "BasicSOFAPriv.hpp"

namespace BasicSOFA
{
    void BasicSOFA::HelloWorld (const char * s)
    {
        BasicSOFAPriv *theObj = new BasicSOFAPriv;
        theObj->HelloWorldPriv(s);
        delete theObj;
    };
    
    
    BasicSOFA::BasicSOFA()
    {
        minRadius = 0;
        maxRadius = 0;
        dRadius = 0;
        minPhi = 0;
        maxPhi = 0;
        dPhi = 0;
        minTheta = 0;
        maxTheta = 0;
        dTheta = 0;
        minImpulseDelay = 0;
        
        fs = 0;
        M = 0;
        N = 0;
        C = 0;
        R = 0;
    }
    
    
    bool BasicSOFA::readSOFAFile (std::string filePath)
    {
        if (filePath == "")
            return false;
        
        try
        {
            h5File = H5::H5File(filePath, H5F_ACC_RDONLY);
            
            //  SOFA single dimension parameter size check
            //  SOFA standard states that these values should be > 0
            hsize_t dim;
            
            dim = getSOFASingleDimParameterSize(SOFA_M_STRING);
            if (dim == 0)
            {
                std::cout << "Invalid SOFA M parameter size" << std::endl;
                resetSOFAData();
                return false;
            }
            M = dim;
            
            
            dim = getSOFASingleDimParameterSize(SOFA_N_STRING);
            if (dim == 0)
            {
                std::cout << "Invalid SOFA N parameter size" << std::endl;
                resetSOFAData();
                return false;
            }
            N = dim;
            
            
            dim = getSOFASingleDimParameterSize(SOFA_R_STRING);
            if (dim == 0)
            {
                std::cout << "Invalid SOFA R parameter size" << std::endl;
                resetSOFAData();
                return false;
            }
            R = dim;
            
            
            dim = getSOFASingleDimParameterSize(SOFA_C_STRING);
            if (dim == 0)
            {
                std::cout << "Invalid SOFA C parameter size" << std::endl;
                resetSOFAData();
                return false;
            }
            C = dim;
            
            
            auto dataSet = h5File.openDataSet(SOFA_FS_STRING);
            dataSet.read(&fs, H5::PredType::NATIVE_DOUBLE);
            if (fs == 0)
            {
                std::cout << "Invalid SOFA sampling frequency" << std::endl;
                resetSOFAData();
                return false;
            }
        
            
            //  HRIR dimensionality check
            //  HRIR dimensionality should be 3 ie. [M x R x N]
            dataSet = h5File.openDataSet(SOFA_HRIR_STRING);
            auto dataSpace = dataSet.getSpace();
            auto nDims = dataSpace.getSimpleExtentNdims();
            if (nDims != 3)
            {
                std::cout << "Invalid number of dimensions in SOFA HRIR"  << std::endl;
                resetSOFAData();
                return false;
            }
            
            hsize_t dims[3];
            dataSpace.getSimpleExtentDims(dims);
            
            if (dims[0] != M ||
                dims[1] != R ||
                dims[2] != N)
            {
                std::cout << "Invalid dimensionality in SOFA HRIR" << std::endl;
                resetSOFAData();
                return false;
            }
            
            std::cout << "Reading HRIR data..." << std::endl;
            hrir = std::vector<double>(M * N * R);
            dataSet.read(hrir.data(), H5::PredType::NATIVE_DOUBLE);
            
            
            //  Map coordinates to impulse responses
            std::vector<double> coordinates = getCoordinatesFromSOFAFile();
            if (coordinates.size() == 0)
            {
                std::cout << "Error getting coordinates from SOFA file" << std::endl;
                resetSOFAData();
                return false;
            }
            
            //  Number of coordinates must be a value divisible by that defined in C
            if (coordinates.size() % C != 0)
            {
                std::cout << "Invalid number of coordinates" << std::endl;
                resetSOFAData();
                return false;
            }
            
            bool success = buildCoordinateMap(coordinates);
            if (!success)
            {
                std::cout << "Error in building coordinate map" << std::endl;
                resetSOFAData();
                return false;
            }
            
            
            //  Get statistical data on the coordinates
            success = calculateCoordinateStatisticalData();
            if (!success)
            {
                std::cout << "Delta radius, phi and theta must be consistent" << std::endl;
                resetSOFAData();
                return false;
            }
            
            
            success = findMinImpulseDelay();
            if (!success)
            {
                std::cout << "Could not find impulse delay\n";
                resetSOFAData();
                return false;
            }
            
        }
        catch (H5::FileIException error)
        {
            error.printErrorStack();
            return false;
        }
        catch (H5::DataSetIException error)
        {
            h5File.close();
            error.printErrorStack();
            return false;
        }
        catch (H5::DataSpaceIException error)
        {
            h5File.close();
            error.printErrorStack();
            return false;
        }
        
        std::cout << "Finished reading HRIR data" << std::endl;
        dataLoaded = true;
        h5File.close();
        
        return true;
    }
    
    
    const double* BasicSOFA::getHRIR(size_t channel, double theta, double phi, double radius) const noexcept
    {
        if (!dataLoaded)
            return nullptr;
        
        if (channel >= R)
            return nullptr;
        
        auto radiusIt = radiusMap.find(round(radius));
        if (radiusIt == radiusMap.end())
            return nullptr;
        
        auto map = coordinateMaps.at(radiusIt->second);
        
        auto phiIt = map.phiMap.find(round(phi));
        if (phiIt == map.phiMap.end())
            return nullptr;
        
        auto thetaMap = map.thetaMaps.at(phiIt->second);
        auto thetaIt = thetaMap.find(round(theta));
        if (thetaIt == thetaMap.end())
            return nullptr;
        
        auto irIndex = map.map.at(phiIt->second).at(thetaIt->second);
        
        return hrir.data() + ((irIndex * R) + channel) * N;
    }
    
    
    void BasicSOFA::resetSOFAData()
    {
        if (hrir.size() != 0)
        {
            hrir.erase(hrir.begin(), hrir.end());
            hrir.shrink_to_fit();
        }
        
        if (radiusList.size() != 0)
        {
            radiusList.erase(radiusList.begin(), radiusList.end());
            radiusList.shrink_to_fit();
        }
        
        if (phiList.size() != 0)
        {
            phiList.erase(phiList.begin(), phiList.end());
            phiList.shrink_to_fit();
        }
        
        if (thetaList.size() != 0)
        {
            thetaList.erase(thetaList.begin(), thetaList.end());
            thetaList.shrink_to_fit();
        }
        
        if (coordinateMaps.size() != 0)
        {
            coordinateMaps.erase(coordinateMaps.begin(), coordinateMaps.end());
            coordinateMaps.shrink_to_fit();
        }
        
        if (radiusMap.size() != 0)
            radiusMap.erase(radiusMap.begin(), radiusMap.end());
        
        minRadius = 0;
        maxRadius = 0;
        dRadius = 0;
        minPhi = 0;
        maxPhi = 0;
        dPhi = 0;
        minTheta = 0;
        maxTheta = 0;
        dTheta = 0;
        
        minImpulseDelay = 0;
        
        fs = 0;
        M = 0;
        N = 0;
        C = 0;
        R = 0;
    }
    
    
    std::vector<double> BasicSOFA::getCoordinatesFromSOFAFile()
    {
        std::vector<double> coordinates;
        
        try
        {
            auto dataSet = h5File.openDataSet(SOFA_SRC_POS_STRING);
            auto dataSpace = dataSet.getSpace();
            
            //  According to the standard, the source and listener positions should have dimensionaltiy [M x C] or [I x C]
            //  The object having M x C dimensionality contains the coordinate data
            hsize_t dims[2];
            dataSpace.getSimpleExtentDims(dims);
            
            if (dims[0] == M)
            {
                coordinates = std::vector<double>(M * C);
                dataSet.read(coordinates.data(), H5::PredType::NATIVE_DOUBLE);
            }
            
            dataSet = h5File.openDataSet(SOFA_LIS_POS_STRING);
            dataSpace = dataSet.getSpace();
            dataSpace.getSimpleExtentDims(dims);
            
            //  If both source and listener objects contain coordinate data, error out and return an empty array
            if (dims[0] == M && coordinates.size() != 0)
            {
                coordinates.erase(coordinates.begin(), coordinates.end());
                coordinates.shrink_to_fit();
                return coordinates;
            }
            
            if (dims[0] == M)
            {
                coordinates = std::vector<double>(M * C);
                dataSet.read(coordinates.data(), H5::PredType::NATIVE_DOUBLE);
            }
            
        }
        catch(...)
        {
            if (coordinates.size() != 0)
            {
                coordinates.erase(coordinates.begin(), coordinates.end());
                coordinates.shrink_to_fit();
            }
            
            return coordinates;
        }
        
        return coordinates;
    }
    
    
    hsize_t BasicSOFA::getSOFASingleDimParameterSize(std::string parameter)
    {
        hsize_t dim;
        
        try
        {
            auto dataSet = h5File.openDataSet(parameter);
            auto dataSpace = dataSet.getSpace();

            dataSpace.getSimpleExtentDims(&dim);
        }
        catch(...)
        {
            return 0;
        }

        return dim;
    }
    
    
    /*
     *  Build tables to map a given radius, theta and phi to its corresponding impulse response
     *
     *  For a given radius, there is a 2D matrix that stores the location of an impulse response for a given theta and phi
     *  Each row of the matrix corresponds to a phi value while each column corresponds to a theta value
     *  For a given phi and theta, there are corresponding std::unordered_map objects that map to the appropriate row and column
     *  This is done to ensure that queries to a particular impulse response is done in constant time - ie: O(1)
     */
    bool BasicSOFA::buildCoordinateMap(const std::vector<double> &coordinates)
    {
        if (coordinates.size() == 0 || (coordinates.size() % C != 0))
            return false;
        
        auto numBlocks = coordinates.size() / C;
        
        for (auto block = 0; block < numBlocks; ++block)
        {
            auto radiusIndex = (block * C) + (C - 1);
            auto phiIndex = (block * C) + (C - 2);
            auto thetaIndex = block * C;
            
            //  Search if a coordinate map exists for the current radius
            auto radius = round(coordinates.at(radiusIndex));
            if (radius < 0)
                return false;
            
            auto radiusIt = radiusMap.find(radius);
            
            //  If a new radius is found, create a coordinate map for it
            if (radiusIt == radiusMap.end())
            {
                coordinateMaps.push_back(SOFACoordinateMap(radius));
                radiusMap.insert({radius, coordinateMaps.size() - 1});
                addValueToArray(radius, radiusList);
            }
            
            auto &map = coordinateMaps.at(radiusMap.at(radius));
            
            //  Search if phi exists in the coordinate map
            //  If not, add another row to the array mapping a (phi, theta) pair to the location of the impulse response
            auto phi = round(coordinates.at(phiIndex));
            auto phiIt = map.phiMap.find(phi);
            
            if (phiIt == map.phiMap.end())
            {
                map.map.push_back(std::vector<size_t>());
                map.phiMap.insert({phi, map.map.size() - 1});
                map.thetaMaps.push_back(std::unordered_map<double, size_t>());
                addValueToArray(phi, phiList);
            }
            
            auto mapRow = map.phiMap.at(phi);
            
            auto theta = round(coordinates.at(thetaIndex));
            if (theta > 180)
                theta -= 360;
            auto &thetaMap = map.thetaMaps.at(mapRow);
            
            //  Search if theta already exists in the coordinate map
            //  If not, add a column to the array mapping a (phi, theta) pair to the location of the impulse response
            //  Normally, there should only be one unique (phi, theta) pair for a given radius
            //  If a pair already exists, it will be overwritten by the new pair
            auto thetaIt = thetaMap.find(theta);
            if (thetaIt == thetaMap.end())
            {
                map.map.at(mapRow).push_back(block);
                thetaMap.insert({theta, map.map.at(mapRow).size() - 1});
                addValueToArray(theta, thetaList);
            }
            else
                map.map.at(mapRow).at(thetaIt->second) = block;
        }
        
        return true;
    }
    
    
    void BasicSOFA::addValueToArray (const double &x, std::vector<double> &A)
    {
        auto search = std::find(A.begin(), A.end(), x);
        if (search == A.end())
            A.push_back(x);
    }
    
    
    bool BasicSOFA::calculateCoordinateStatisticalData()
    {
        std::sort(radiusList.begin(), radiusList.end());
        std::sort(thetaList.begin(), thetaList.end());
        std::sort(phiList.begin(), phiList.end());
        
        auto delta = round(radiusList[1] - radiusList[0]);
        for (auto i = 2; i < radiusList.size() - 1; ++i)
        {
            if (round(radiusList[i] - radiusList[i - 1]) != delta)
                return false;
        }
        dRadius = delta;
        minRadius = radiusList.at(0);
        maxRadius = radiusList.at(radiusList.size() - 1);
        
        
        delta = abs(round(thetaList[1] - thetaList[0]));
        for (auto i = 2; i < thetaList.size() - 1; ++i)
        {
            if (abs(round(thetaList[i] - thetaList[i - 1])) != delta)
                return false;
        }
        dTheta = delta;
        minTheta = thetaList.at(0);
        maxTheta = thetaList.at(thetaList.size() - 1);
        
        
        delta = abs(round(phiList[1] - phiList[0]));
        for (auto i = 2; i < phiList.size() - 1; ++i)
        {
            if (abs(round(phiList[i] - phiList[i - 1])) != delta)
                return false;
        }
        dPhi = delta;
        minPhi = phiList.at(0);
        maxPhi = phiList.at(phiList.size() - 1);
        
        
        return true;
    }
    
    
    double BasicSOFA::round(const double &x) const
    {
        double temp = 0;
        
        if (x > 0.0)
            temp = x + (epsilon / 2);
        else
            temp = x - (epsilon / 2);
        
        int tempInt = static_cast<int>(temp / epsilon);
        
        return tempInt * epsilon;
    }
    
    
    /*
     *  Go through all of the impulse responses and find where the impulse peak happens and track the index
     *  The minimum index is the 'earliest' impulse delay
     *  Of course, you should not simply use this value when truncating your HRIRs
     *  The starting point of the truncated HRIR should be well less than minImpulseDelay
     *  minImpulseDelay / 2 is a good place to start
     */
    bool BasicSOFA::findMinImpulseDelay()
    {
        if (M == 0)
            return false;
        
        minImpulseDelay = N;
        
        for (auto i = 0; i < M; ++i)
        {
            double hrirMax = 0.0;
            size_t maxLocation = 0;
            
            for (auto j = i * N; j < (i * N) + N; ++j)
            {
                if (hrirMax < abs(hrir[j]))
                {
                    hrirMax = abs(hrir[j]);
                    maxLocation = j;
                }
            }
            
            if (maxLocation < minImpulseDelay)
                minImpulseDelay = maxLocation;
        }
        
        return true;
    }
    
}



void BasicSOFAPriv::HelloWorldPriv (const char * s)
{
    std::cout << s << std::endl;
};


