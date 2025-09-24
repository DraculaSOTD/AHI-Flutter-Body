//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AvatarGenCommon_hpp
#define AvatarGenCommon_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <AHIBSCereal.hpp>
#include "Common.hpp"

using namespace BodyScanCommon;

namespace avatar_gen {

    struct common_models {
        std::map<std::string, AHIModelCV> male;
        std::map<std::string, AHIModelCV> female;
    };

    class common {
    private:
        SexType m_gender;
        common_models m_models;

        // Singleton constructor
        common(SexType gender,
               std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
               std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale);

        common();                   // Don't Implement.
        common(common const &);          // Don't Implement.
        void operator=(common const &);  // Don't implement

        void setModels(
                std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
                std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale
        );

    public:
        // Singleton methods
        static common *getInstance(
                SexType gender,
                std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
                std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale
        ) {
            static common instance(gender, cvModelsMale, cvModelsFemale);
            return &instance;
        }

        static common *getInstance() {
            std::map<std::string, std::pair<char *, std::size_t>> cvModelsMale;
            std::map<std::string, std::pair<char *, std::size_t>> cvModelsFemale;
            return getInstance(male, cvModelsMale, cvModelsFemale);
        }

        // Class methods
        std::vector<int> &getInvRightCalf() const;

        std::vector<int> &getInvRightThigh() const;

        std::vector<int> &getInvRightUpperArm() const;

        std::vector<float> getMvnMu() const;

        std::vector<float> getMvnMu(SexType gender) const;

        std::vector<std::vector<float> > &getRanges() const;

        std::vector<std::vector<float> > &getRanges(SexType gender) const;

        std::vector<std::vector<float> > &getCov() const;

        std::vector<std::vector<float> > &getCov(SexType gender) const;

        std::vector<float> &getAvgVerts() const;

        std::vector<float> &getAvgVerts(SexType gender) const;

        std::vector<float> &getVertsInv() const;

        std::vector<float> &getVertsInv(SexType gender) const;

        std::vector<int> &getFaces() const;

        std::vector<int> &getFaces(SexType gender) const;

        std::vector<int> &getFacesInv() const;

        std::vector<int> &getFacesInv(SexType gender) const;

        std::vector<std::vector<float> > &getSv() const;

        std::vector<std::vector<float> > &getSv(SexType gender) const;

        std::vector<std::vector<float> > &getSvInv() const;

        std::vector<std::vector<float> > &getSvInv(SexType gender) const;

        std::vector<std::vector<float> > &getSkV() const;

        std::vector<std::vector<float> > &getSkV(SexType gender) const;

        std::vector<std::vector<float> > &getBonW() const;

        std::vector<std::vector<float> > &getBonW(SexType gender) const;

        std::vector<std::vector<float> > &getBonWInv() const;

        std::vector<std::vector<float> > &getBonWInv(SexType gender) const;

        std::vector<int> &getLaplacianRings() const;

        std::vector<int> &getLaplacianRings(SexType gender) const;

        std::vector<int> &getLaplacianRingsAsVectors() const;

        std::vector<int> &getLaplacianRingsAsVectors(SexType gender) const;

    };

}

#endif /* AvatarGenCommon_hpp */
