// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Wouter Deconinck, David Lawrence

#include "algorithms/interfaces/ParticleSvc.h"

const ParticleSvc::ParticleMap kParticleMap = {
    {           0, {           0,   0,   0.0            }},  // unknown
    {          11, {          11,  -1,   0.000510998928 }},  // e-
    {         -11, {         -11,   1,   0.000510998928 }},  // e+
    {          13, {          13,  -1,   0.105658357    }},  // mu-
    {         -13, {         -13,   1,   0.105658357    }},  // mu+
    {          22, {          22,   0,   0.0            }},  // gamma
    {         111, {         111,   0,   0.1349766      }},  // p0
    {         113, {         111,   0,   0.76850        }},  // rho(770)^0
    {         115, {         115,   0,   1.31800        }},  // a_2(1320)^0
    {         211, {         211,   1,   0.1395701      }},  // pi+
    {        -211, {        -211,  -1,   0.1395701      }},  // pi-
    {         213, {         213,   1,   0.76690        }},  // rho+
    {        -213, {        -213,  -1,   0.76690        }},  // rho-
    {         215, {         215,   1,   1.31800        }},  // a_2(1320)+
    {        -215, {        -215,  -1,   1.31800        }},  // a_2(1320)-
    {         221, {         221,   0,   0.54745        }},  // eta
    {         223, {         223,   0,   0.78194        }},  // omega
    {         225, {         225,   0,   1.27500        }},  // f_2(1270)
    {         130, {         130,   0,   0.49767        }},  // KL_0
    {         310, {         310,   0,   0.49767        }},  // KS_0
    {         311, {         311,   0,   0.49767        }},  // K^0
    {        -311, {        -311,   0,   0.49767        }},  // K~^0
    {         313, {         313,   0,   0.89610        }},  // K*(892)^0
    {        -313, {        -313,   0,   0.89610        }},  // K*(892)~^0
    {         315, {         315,   0,   1.43200        }},  // K*_2(1430)^0
    {        -315, {        -315,   0,   1.43200        }},  // K*_2(1430)~^0
    {         321, {         321,   1,   0.49360        }},  // K^+
    {        -321, {        -321,  -1,   0.49360        }},  // K^-
    {         323, {         323,   1,   0.89160        }},  // K*(892)^+
    {        -323, {        -323,  -1,   0.89160        }},  // K*(892)^-
    {         325, {         325,   1,   1.42500        }},  // K*_2(1430)^+
    {        -325, {        -325,  -1,   1.42500        }},  // K*_2(1430)^-
    {         331, {         331,   0,   0.95777        }},  // eta'(958)
    {         333, {         333,   0,   1.01940        }},  // phi(1020)
    {         335, {         335,   0,   1.52500        }},  // f'_2(1525)
    {         411, {         411,   1,   1.86930        }},  // D+
    {        -411, {         411,  -1,   1.86930        }},  // D-
    {         413, {         413,   1,   2.01000        }},  // D*(2010)^+
    {        -413, {        -413,  -1,   2.01000        }},  // D*(2010)^-
    {         415, {         415,   1,   2.46000        }},  // D*_2(2460)^+
    {        -415, {        -415,  -1,   2.46000        }},  // D*_2(2460)^-
    {         421, {         421,   0,   1.86450        }},  // D^0
    {        -421, {        -421,   0,   1.86450        }},  // D~^0
    {         423, {         423,   0,   2.00670        }},  // D*(2007)^0
    {        -423, {        -423,   0,   2.00670        }},  // D*(2007)~^0
    {         425, {         425,   0,   2.46000        }},  // D*_2(2460)^0
    {        -425, {        -425,   0,   2.46000        }},  // D*_2(2460)~^0
    {         431, {         431,   1,   1.96850        }},  // D_s^+
    {        -431, {        -431,  -1,   1.96850        }},  // D_s^-
    {         433, {         433,   1,   2.11240        }},  // D*_s^+
    {        -433, {        -433,  -1,   2.11240        }},  // D*_s^-
    {         435, {         435,   1,   2.57350        }},  // D*_s2(2573)^+
    {        -435, {        -435,  -1,   2.57350        }},  // D*_s2(2573)^-
    {         441, {         441,   0,   2.97980        }},  // eta_c(1S)
    {         443, {         443,   0,   3.09688        }},  // J/psi(1S)
    {         445, {         445,   0,   3.55620        }},  // chi_c2(1P)
    {         511, {         511,   0,   5.27920        }},  // B^0
    {        -511, {        -511,   0,   5.27920        }},  // B~^0
    {         513, {         513,   0,   5.32480        }},  // B*^0
    {        -513, {        -513,   0,   5.32480        }},  // B*~^0
    {         515, {         515,   0,   5.83000        }},  // B*_2^0
    {        -515, {        -515,   0,   5.83000        }},  // B*_2~^0
    {         521, {         521,   1,   5.27890        }},  // B^+
    {        -521, {        -521,  -1,   5.27890        }},  // B^-
    {         523, {         523,   1,   5.32480        }},  // B*^+
    {        -523, {        -523,  -1,   5.32480        }},  // B*^-
    {         525, {         525,   1,   5.83000        }},  // B*_2^+
    {        -525, {        -525,  -1,   5.83000        }},  // B*_2^-
    {         531, {         531,   0,   5.36930        }},  // B_s^0
    {        -531, {        -531,   0,   5.36930        }},  // B_s~^0
    {         533, {         533,   0,   5.41630        }},  // B*_s^0
    {        -533, {        -533,   0,   5.41630        }},  // B*_s~^0
    {         535, {         535,   0,   6.07000        }},  // B*_s2^0
    {        -535, {        -535,   0,   6.07000        }},  // B*_s2~^0
    {         541, {         541,   1,   6.59400        }},  // B_c^+
    {        -541, {        -541,  -1,   6.59400        }},  // B_c^-
    {         543, {         543,   1,   6.60200        }},  // B*_c^+
    {        -543, {        -543,  -1,   6.60200        }},  // B*_c^-
    {         545, {         545,   1,   7.35000        }},  // B*_c2^+
    {        -545, {        -545,  -1,   7.35000        }},  // B*_c2^-
    {         551, {         551,   0,   9.40000        }},  // eta_b(1S)
    {         553, {         553,   0,   9.46030        }},  // Upsilon(1S)
    {         555, {         555,   0,   9.91320        }},  // chi_b2(1P)
    {         990, {         990,   0,   0.00000        }},  // pomeron
    {        1114, {        1114,  -1,   1.23400        }},  // Delta^-
    {       -1114, {       -1114,   1,   1.23400        }},  // Delta~^+
    {        2112, {        2112,   0,   0.93957        }},  // n
    {       -2112, {       -2112,   0,   0.93957        }},  // n~^0
    {        2114, {        2114,   0,   1.23300        }},  // Delta^0
    {       -2114, {       -2114,   0,   1.23300        }},  // Delta~^0
    {        2212, {        2212,   1,   0.93827        }},  // p^+
    {       -2212, {       -2212,  -1,   0.93827        }},  // p~^-
    {        2214, {        2214,   1,   1.23200        }},  // Delta^+
    {       -2214, {       -2214,  -1,   1.23200        }},  // Delta~^-
    {        2224, {        2224,   2,   1.23100        }},  // Delta^++
    {       -2224, {       -2224,  -2,   1.23100        }},  // Delta~^--
    {        3112, {        3112,  -1,   1.19744        }},  // Sigma^-
    {       -3112, {       -3112,   1,   1.19744        }},  // Sigma~^+
    {        3114, {        3114,  -1,   1.38720        }},  // Sigma*^-
    {       -3114, {       -3114,   1,   1.38720        }},  // Sigma*~^+
    {        3122, {        3122,   0,   1.11568        }},  // Lambda^0
    {       -3122, {       -3122,   0,   1.11568        }},  // Lambda~^0
    {        3212, {        3212,   0,   1.19255        }},  // Sigma^0
    {       -3212, {       -3212,   0,   1.19255        }},  // Sigma~^0
    {        3214, {        3214,   0,   1.38370        }},  // Sigma*^0
    {       -3214, {       -3214,   0,   1.38370        }},  // Sigma*~^0
    {        3222, {        3222,   1,   1.18937        }},  // Sigma^+
    {       -3222, {       -3222,  -1,   1.18937        }},  // Sigma~^-
    {        3224, {        3224,   1,   1.38280        }},  // Sigma*^+
    {       -3224, {       -3224,  -1,   1.38280        }},  // Sigma*~^-
    {        3312, {        3312,  -1,   1.32130        }},  // Xi^-
    {       -3312, {       -3312,   1,   1.32130        }},  // Xi~^+
    {        3314, {        3314,  -1,   1.53500        }},  // Xi*^-
    {       -3314, {       -3314,   1,   1.53500        }},  // Xi*~^+
    {        3322, {        3322,   0,   1.31490        }},  // Xi^0
    {       -3322, {       -3322,   0,   1.31490        }},  // Xi~^0
    {        3324, {        3324,   0,   1.53180        }},  // Xi*^0
    {       -3324, {       -3324,   0,   1.53180        }},  // Xi*~^0
    {        3334, {        3334,  -1,   1.67245        }},  // Omega^-
    {       -3334, {       -3334,   1,   1.67245        }},  // Omega~^+
    {        4112, {        4112,   0,   2.45210        }},  // Sigma_c^0
    {       -4112, {       -4112,   0,   2.45210        }},  // Sigma_c~^0
    {        4114, {        4114,   0,   2.50000        }},  // Sigma*_c^0
    {       -4114, {       -4114,   0,   2.50000        }},  // Sigma*_c~^0
    {        4122, {        4122,   1,   2.28490        }},  // Lambda_c^+
    {       -4122, {       -4122,  -1,   2.28490        }},  // Lambda_c~^-
    {        4132, {        4132,   0,   2.47030        }},  // Xi_c^0
    {       -4132, {       -4132,   0,   2.47030        }},  // Xi_c~^0
    {        4212, {        4212,   1,   2.45350        }},  // Sigma_c^+
    {       -4212, {       -4212,  -1,   2.45350        }},  // Sigma_c~^-
    {        4214, {        4214,   1,   2.50000        }},  // Sigma*_c^+
    {       -4214, {       -4214,  -1,   2.50000        }},  // Sigma*_c~^-
    {        4222, {        4222,   2,   2.45290        }},  // Sigma_c^++
    {       -4222, {       -4222,  -2,   2.45290        }},  // Sigma_c~^--
    {        4224, {        4224,   2,   2.50000        }},  // Sigma*_c^++
    {       -4224, {       -4224,  -2,   2.50000        }},  // Sigma*_c~^--
    {        4232, {        4232,   1,   2.46560        }},  // Xi_c^+
    {       -4232, {       -4232,  -1,   2.46560        }},  // Xi_c~^-
    {        4312, {        4312,   0,   2.55000        }},  // Xi'_c^0
    {       -4312, {       -4312,   0,   2.55000        }},  // Xi'_c~^0
    {        4314, {        4314,   0,   2.63000        }},  // Xi*_c^0
    {       -4314, {       -4314,   0,   2.63000        }},  // Xi*_c~^0
    {        4322, {        4322,   1,   2.55000        }},  // Xi'_c^+
    {       -4322, {       -4322,  -1,   2.55000        }},  // Xi'_c~^-
    {        4324, {        4324,   1,   2.63000        }},  // Xi*_c^+
    {       -4324, {       -4324,  -1,   2.63000        }},  // Xi*_c~^-
    {        4332, {        4332,   0,   2.70400        }},  // Omega_c^0
    {       -4332, {       -4332,   0,   2.70400        }},  // Omega_c~^0
    {        4334, {        4334,   0,   2.80000        }},  // Omega*_c^0
    {       -4334, {       -4334,   0,   2.80000        }},  // Omega*_c~^0
    {        4412, {        4412,   1,   3.59798        }},  // Xi_cc^+
    {       -4412, {       -4412,  -1,   3.59798        }},  // Xi_cc~^-
    {        4414, {        4414,   1,   3.65648        }},  // Xi*_cc^+
    {       -4414, {       -4414,  -1,   3.65648        }},  // Xi*_cc~^-
    {        4422, {        4422,   2,   3.59798        }},  // Xi_cc^++
    {       -4422, {       -4422,  -2,   3.59798        }},  // Xi_cc~^--
    {        4424, {        4424,   2,   3.65648        }},  // Xi*_cc^++
    {       -4424, {       -4424,  -2,   3.65648        }},  // Xi*_cc~^--
    {        4432, {        4432,   1,   3.78663        }},  // Omega_cc^+
    {       -4432, {       -4432,  -1,   3.78663        }},  // Omega_cc~^-
    {        4434, {        4434,   1,   3.82466        }},  // Omega*_cc^+
    {       -4434, {       -4434,  -1,   3.82466        }},  // Omega*_cc~^-
    {        4444, {        4444,   2,   4.91594        }},  // Omega*_ccc^++
    {       -4444, {       -4444,  -2,   4.91594        }},  // Omega*_ccc~^--
    {        5112, {        5112,  -1,   5.80000        }},  // Sigma_b^-
    {       -5112, {       -5112,   1,   5.80000        }},  // Sigma_b~^+
    {        5114, {        5114,  -1,   5.81000        }},  // Sigma*_b^-
    {       -5114, {       -5114,   1,   5.81000        }},  // Sigma*_b~^+
    {        5122, {        5122,   0,   5.64100        }},  // Lambda_b^0
    {       -5122, {       -5122,   0,   5.64100        }},  // Lambda_b~^0
    {        5132, {        5132,  -1,   5.84000        }},  // Xi_b^-
    {       -5132, {       -5132,   1,   5.84000        }},  // Xi_b~^+
    {        5142, {        5142,   0,   7.00575        }},  // Xi_bc^0
    {       -5142, {       -5142,   0,   7.00575        }},  // Xi_bc~^0
    {        5212, {        5212,   0,   5.80000        }},  // Sigma_b^0
    {       -5212, {       -5212,   0,   5.80000        }},  // Sigma_b~^0
    {        5214, {        5214,   0,   5.81000        }},  // Sigma*_b^0
    {       -5214, {       -5214,   0,   5.81000        }},  // Sigma*_b~^0
    {        5222, {        5222,   1,   5.80000        }},  // Sigma_b^+
    {       -5222, {       -5222,  -1,   5.80000        }},  // Sigma_b~^-
    {        5224, {        5224,   1,   5.81000        }},  // Sigma*_b^+
    {       -5224, {       -5224,  -1,   5.81000        }},  // Sigma*_b~^-
    {        5232, {        5232,   0,   5.84000        }},  // Xi_b^0
    {       -5232, {       -5232,   0,   5.84000        }},  // Xi_b~^0
    {        5242, {        5242,   1,   7.00575        }},  // Xi_bc^+
    {       -5242, {       -5242,  -1,   7.00575        }},  // Xi_bc~^-
    {        5312, {        5312,  -1,   5.96000        }},  // Xi'_b^-
    {       -5312, {       -5312,   1,   5.96000        }},  // Xi'_b~^+
    {        5314, {        5314,  -1,   5.97000        }},  // Xi*_b^-
    {       -5314, {       -5314,   1,   5.97000        }},  // Xi*_b~^+
    {        5322, {        5322,   0,   5.96000        }},  // Xi'_b^0
    {       -5322, {       -5322,   0,   5.96000        }},  // Xi'_b~^0
    {        5324, {        5324,   0,   5.97000        }},  // Xi*_b^0
    {       -5324, {       -5324,   0,   5.97000        }},  // Xi*_b~^0
    {        5332, {        5332,  -1,   6.12000        }},  // Omega_b^-
    {       -5332, {       -5332,   1,   6.12000        }},  // Omega_b~^+
    {        5334, {        5334,  -1,   6.13000        }},  // Omega*_b^-
    {       -5334, {       -5334,   1,   6.13000        }},  // Omega*_b~^+
    {        5342, {        5342,   0,   7.19099        }},  // Omega_bc^0
    {       -5342, {       -5342,   0,   7.19099        }},  // Omega_bc~^0
    {        5412, {        5412,   0,   7.03724        }},  // Xi'_bc^0
    {       -5412, {       -5412,   0,   7.03724        }},  // Xi'_bc~^0
    {        5414, {        5414,   0,   7.04850        }},  // Xi*_bc^0
    {       -5414, {       -5414,   0,   7.04850        }},  // Xi*_bc~^0
    {        5422, {        5422,   1,   7.03724        }},  // Xi'_bc^+
    {       -5422, {       -5422,  -1,   7.03724        }},  // Xi'_bc~^-
    {        5424, {        5424,   1,   7.04850        }},  // Xi*_bc^+
    {       -5424, {       -5424,  -1,   7.04850        }},  // Xi*_bc~^-
    {        5432, {        5432,   0,   7.21101        }},  // Omega'_bc^0
    {       -5432, {       -5432,   0,   7.21101        }},  // Omega'_bc~^0
    {        5434, {        5434,   0,   7.21900        }},  // Omega*_bc^0
    {       -5434, {       -5434,   0,   7.21900        }},  // Omega*_bc~^0
    {        5442, {        5442,   1,   8.30945        }},  // Omega_bcc^+
    {       -5442, {       -5442,  -1,   8.30945        }},  // Omega_bcc~^-
    {        5444, {        5444,   1,   8.31325        }},  // Omega*_bcc^+
    {       -5444, {       -5444,  -1,   8.31325        }},  // Omega*_bcc~^-
    {        5512, {        5512,  -1,   10.42272       }},  // Xi_bb^-
    {       -5512, {       -5512,   1,   10.42272       }},  // Xi_bb~^+
    {        5514, {        5514,  -1,   10.44144       }},  // Xi*_bb^-
    {       -5514, {       -5514,   1,   10.44144       }},  // Xi*_bb~^+
    {        5522, {        5522,   0,   10.42272       }},  // Xi_bb^0
    {       -5522, {       -5522,   0,   10.42272       }},  // Xi_bb~^0
    {        5524, {        5524,   0,   10.44144       }},  // Xi*_bb^0
    {       -5524, {       -5524,   0,   10.44144       }},  // Xi*_bb~^0
    {        5532, {        5532,  -1,   10.60209       }},  // Omega_bb^-
    {       -5532, {       -5532,   1,   10.60209       }},  // Omega_bb~^+
    {        5534, {        5534,  -1,   10.61426       }},  // Omega*_bb^-
    {       -5534, {       -5534,   1,   10.61426       }},  // Omega*_bb~^+
    {        5542, {        5542,   0,   11.70767       }},  // Omega_bbc^0
    {       -5542, {       -5542,   0,   11.70767       }},  // Omega_bbc~^0
    {        5544, {        5544,   0,   11.71147       }},  // Omega*_bbc^0
    {       -5544, {       -5544,   0,   11.71147       }},  // Omega*_bbc~^0
    {        5554, {        5554,  -1,   15.11061       }},  // Omega*_bbb^-
    {       -5554, {       -5554,   1,   15.11061       }},  // Omega*_bbb~^+
    {  1000010020, {  1000010020,   1,   1.87561        }},  // Deuterium
    {  1000010030, {  1000010030,   1,   2.80925        }},  // Tritium
    {  1000020030, {  1000020030,   2,   2.80923        }},  // He-3
    {  1000020040, {  1000020040,   2,   3.72742        }},  // Alpha
};


ParticleSvc::ParticleSvc() {
  m_particleMap = kParticleMap;  
}

