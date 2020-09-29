//
//  main.cpp
//  BasicSOFATest
//
//  Created by Allen Lee on 2020-07-09.
//  Copyright © 2020 meoWorkshop. All rights reserved.
//

#include <iostream>
#include <BasicSOFA.hpp>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#define VALID_SOFA_FILEPATH "/Users/superkittens/projects/sound_prototypes/hrtf/hrtfs/nf_hrtf_sph.sofa"
#define UNSUPPORTED_SOFA_FILEPATH "/Users/superkittens/projects/sound_prototypes/hrtf/hrtfs/QU_KEMAR_Auditorium3.sofa"
#define IR_LOG_FILEPATH "/Users/superkittens/Desktop/test_ir.txt"

#define FLOAT_PRECISION 20


TEST_CASE("Valid SOFA File Test", "[Valid SOFA Test]")
{
    BasicSOFA::BasicSOFA sofa;
    bool success = sofa.readSOFAFile(VALID_SOFA_FILEPATH);
    
    REQUIRE(success == true);
    
    SECTION("File Properties Check")
    {
        REQUIRE(sofa.getFs() == 48000.0);
        REQUIRE(sofa.getN() == 512);
        REQUIRE(sofa.getM() == 55874);
        REQUIRE(sofa.getC() == 3);
        REQUIRE(sofa.getR() == 2);
        REQUIRE(sofa.getMinPhi() == -90);
        REQUIRE(sofa.getMaxPhi() == 90);
        REQUIRE(sofa.getDeltaPhi() == 10);
        REQUIRE(sofa.getMinTheta() == -170);
        REQUIRE(sofa.getMaxTheta() == 180);
        REQUIRE(sofa.getDeltaTheta() == 10);
        REQUIRE(sofa.getMinRadius() == 0.1);
        REQUIRE(sofa.getMaxRadius() == 1);
        REQUIRE(sofa.getDeltaRadius() == 0.1);
    }
    
    SECTION("Impulse Response Check")
    {
        //  Create the file used to log the actual impulse response
        std::ofstream outFile;
        outFile.open(IR_LOG_FILEPATH, std::ios::out | std::ios::trunc);
        REQUIRE(outFile.is_open() == true);
        
        //  Check left channel
        const double *ir = sofa.getHRIR(0, 0, 90, 1);
        REQUIRE(ir != nullptr);
        
        outFile << "LEFT CHANNEL" << std::endl << "=====================================" << std::endl;
        
        for (auto i = 0; i < sofa.getN(); ++i)
            outFile << std::setprecision(FLOAT_PRECISION) << ir[i] << std::endl;
        
        //  Check right channel
        ir = sofa.getHRIR(1, 0, 90, 1);
        REQUIRE(ir != nullptr);
        
        outFile << "RIGHT CHANNEL" << std::endl << "=====================================" << std::endl;
        
        for (auto i = 0; i < sofa.getN(); ++i)
            outFile << std::setprecision(FLOAT_PRECISION) << ir[i] << std::endl;
        
        outFile.close();
        REQUIRE(outFile.is_open() == false);
    }
    
    SECTION("BasicSOFA Object Reset")
    {
        sofa.resetSOFAData();
        
        REQUIRE(sofa.getFs() == 0);
        REQUIRE(sofa.getN() == 0);
        REQUIRE(sofa.getM() == 0);
        REQUIRE(sofa.getC() == 0);
        REQUIRE(sofa.getR() == 0);
        REQUIRE(sofa.getMinPhi() == 0);
        REQUIRE(sofa.getMaxPhi() == 0);
        REQUIRE(sofa.getDeltaPhi() == 0);
        REQUIRE(sofa.getMinTheta() == 0);
        REQUIRE(sofa.getMaxTheta() == 0);
        REQUIRE(sofa.getDeltaTheta() == 0);
        REQUIRE(sofa.getMinRadius() == 0);
        REQUIRE(sofa.getMaxRadius() == 0);
        REQUIRE(sofa.getDeltaRadius() == 0);
        
        const double *ir = sofa.getHRIR(0, 0, 90, 1);
        REQUIRE(ir == nullptr);
        
        
        SECTION("SOFA File Read after Reset")
        {
            bool success = sofa.readSOFAFile(VALID_SOFA_FILEPATH);
            REQUIRE(success == true);
            
            REQUIRE(sofa.getFs() == 48000.0);
            REQUIRE(sofa.getN() == 512);
            REQUIRE(sofa.getM() == 55874);
            REQUIRE(sofa.getC() == 3);
            REQUIRE(sofa.getR() == 2);
            REQUIRE(sofa.getMinPhi() == -90);
            REQUIRE(sofa.getMaxPhi() == 90);
            REQUIRE(sofa.getDeltaPhi() == 10);
            REQUIRE(sofa.getMinTheta() == -170);
            REQUIRE(sofa.getMaxTheta() == 180);
            REQUIRE(sofa.getDeltaTheta() == 10);
            REQUIRE(sofa.getMinRadius() == 0.1);
            REQUIRE(sofa.getMaxRadius() == 1);
            REQUIRE(sofa.getDeltaRadius() == 0.1);
            
            std::ifstream inFile;
            inFile.open(IR_LOG_FILEPATH);
            REQUIRE(inFile.is_open() == true);
            
            for (auto channel = 0; channel < 2; ++channel)
            {
                const double *ir = sofa.getHRIR(channel, 0, 90, 1);
                REQUIRE(ir != nullptr);
                
                char data[100];
                inFile.getline(data, 100);
                inFile.getline(data, 100);
                
                for (auto i = 0; i < sofa.getN(); ++i)
                {
                    inFile.getline(data, 100);
                    std::string str(data);
                    
                    REQUIRE(std::stod(str) == ir[i]);
                }
            }
            
            inFile.close();
            REQUIRE(inFile.is_open() == false);
        }
    }
}



TEST_CASE("Unsupported SOFA File Test", "[Unsupported SOFA Test]")
{
    BasicSOFA::BasicSOFA sofa;
    bool success = sofa.readSOFAFile(UNSUPPORTED_SOFA_FILEPATH);
    REQUIRE(success == false);
    
    SECTION("Unsupported SOFA Properties Check")
    {
        REQUIRE(sofa.getFs() == 0);
        REQUIRE(sofa.getN() == 0);
        REQUIRE(sofa.getM() == 0);
        REQUIRE(sofa.getC() == 0);
        REQUIRE(sofa.getR() == 0);
        REQUIRE(sofa.getMinPhi() == 0);
        REQUIRE(sofa.getMaxPhi() == 0);
        REQUIRE(sofa.getDeltaPhi() == 0);
        REQUIRE(sofa.getMinTheta() == 0);
        REQUIRE(sofa.getMaxTheta() == 0);
        REQUIRE(sofa.getDeltaTheta() == 0);
        REQUIRE(sofa.getMinRadius() == 0);
        REQUIRE(sofa.getMaxRadius() == 0);
        REQUIRE(sofa.getDeltaRadius() == 0);
    }
    
    SECTION("Attempt to Read Unsupported SOFA HRIR")
    {
        const double *ir = sofa.getHRIR(0, 0, 0, 1);
        REQUIRE(ir == nullptr);
    }
}

