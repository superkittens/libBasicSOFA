//
//  main.cpp
//  BasicSOFATest
//
//  Created by Allen Lee on 2020-07-09.
//  Copyright © 2020 meoWorkshop. All rights reserved.
//

#include <iostream>
#include <BasicSOFA.hpp>

int main(int argc, const char * argv[]) {
    // insert code here...
    BasicSOFA::BasicSOFA sofa;
    
    sofa.readSOFAFile("/Users/superkittens/projects/sound_prototypes/hrtf/hrtfs/nf_hrtf_sph.sofa");
    const double* ir = sofa.getHRIR(0, 90, 0, 1);
    
    for (auto i = 0; i < sofa.getN(); ++i)
        std::cout << ir[i] << std::endl;
    
    return 0;
}
