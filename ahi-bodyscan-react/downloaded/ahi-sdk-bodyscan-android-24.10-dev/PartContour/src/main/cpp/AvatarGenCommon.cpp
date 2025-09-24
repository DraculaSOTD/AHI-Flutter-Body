//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AvatarGenCommon.hpp"

namespace avatar_gen {
// Class methods
    common::common(
            SexType gender,
            std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
            std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale
    ) {
        m_gender = gender;
        setModels(cvModelsMale, cvModelsFemale);
    }

    void common::setModels(
            std::map<std::string, std::pair<char *, std::size_t>> &cvModelsMale,
            std::map<std::string, std::pair<char *, std::size_t>> &cvModelsFemale
    ) {
        m_models = common_models();
        for (auto &model : cvModelsMale) {
            auto cv = ahiDecodeCvFromBytes(model.second.first, model.second.second);
            m_models.male[model.first] = cv;
        }
        for (auto &model : cvModelsFemale) {
            auto cv = ahiDecodeCvFromBytes(model.second.first, model.second.second);
            m_models.female[model.first] = cv;
        }
    }

    std::vector<int> &common::getInvRightCalf() const {
        return const_cast<std::vector<int> &>(m_models.male.at("InvRightCalf").vi);
    }

    std::vector<int> &common::getInvRightThigh() const {
        return const_cast<std::vector<int> &>(m_models.male.at("InvRightThigh").vi);
    }

    std::vector<int> &common::getInvRightUpperArm() const {
        return const_cast<std::vector<int> &>(m_models.male.at("InvRightUpperArm").vi);
    }


    std::vector<float> common::getMvnMu() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<float> &>(m_models.male.at("MvnMu").vf);
        } else {
            return const_cast<std::vector<float> &>(m_models.female.at("MvnMu").vf);
        }
    }

    std::vector<float> common::getMvnMu(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<float> &>(m_models.male.at("MvnMu").vf);
        } else {
            return const_cast<std::vector<float> &>(m_models.female.at("MvnMu").vf);
        }
    }

    std::vector<std::vector<float> > &common::getRanges() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("Ranges").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("Ranges").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getRanges(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("Ranges").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("Ranges").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getCov() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("Cov").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("Cov").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getCov(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("Cov").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("Cov").vvf);
        }
    }

    std::vector<float> &common::getAvgVerts() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<float> &>(m_models.male.at("AvgVerts").vf);
        } else {
            return const_cast<std::vector<float> &>(m_models.female.at("AvgVerts").vf);
        }
    }

    std::vector<float> &common::getAvgVerts(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<float> &>(m_models.male.at("AvgVerts").vf);
        } else {
            return const_cast<std::vector<float> &>(m_models.female.at("AvgVerts").vf);
        }
    }

    std::vector<float> &common::getVertsInv() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<float> &>(m_models.male.at("VertsInv").vf);
        } else {
            return const_cast<std::vector<float> &>(m_models.female.at("VertsInv").vf);
        }
    }

    std::vector<float> &common::getVertsInv(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<float> &>(m_models.male.at("VertsInv").vf);
        } else {
            return const_cast<std::vector<float> &>(m_models.female.at("VertsInv").vf);
        }
    }

    std::vector<int> &common::getFaces() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("Faces").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("Faces").vi);
        }
    }

    std::vector<int> &common::getFaces(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("Faces").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("Faces").vi);
        }
    }

    std::vector<int> &common::getFacesInv() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("FacesInv").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("FacesInv").vi);
        }
    }

    std::vector<int> &common::getFacesInv(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("FacesInv").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("FacesInv").vi);
        }
    }

    std::vector<std::vector<float> > &common::getSv() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("Sv").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("Sv").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getSv(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("Sv").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("Sv").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getSvInv() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("SvInv").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("SvInv").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getSvInv(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("SvInv").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("SvInv").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getSkV() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("SkV").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("SkV").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getSkV(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("SkV").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("SkV").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getBonW() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("BonW").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("BonW").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getBonW(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("BonW").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("BonW").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getBonWInv() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("BonWInv").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("BonWInv").vvf);
        }
    }

    std::vector<std::vector<float> > &common::getBonWInv(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<std::vector<float>> &>(m_models.male.at("BonWInv").vvf);
        } else {
            return const_cast<std::vector<std::vector<float>> &>(m_models.female.at("BonWInv").vvf);
        }
    }

    std::vector<int> &common::getLaplacianRings() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("LaplacianRings").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("LaplacianRings").vi);
        }
    }

    std::vector<int> &common::getLaplacianRings(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("LaplacianRings").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("LaplacianRings").vi);
        }
    }

    std::vector<int> &common::getLaplacianRingsAsVectors() const {
        if (m_gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("LaplacianRingsAsVectors").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("LaplacianRingsAsVectors").vi);
        }
    }

    std::vector<int> &common::getLaplacianRingsAsVectors(SexType gender) const {
        if (gender == SexType::male) {
            return const_cast<std::vector<int> &>(m_models.male.at("LaplacianRingsAsVectors").vi);
        } else {
            return const_cast<std::vector<int> &>(m_models.female.at("LaplacianRingsAsVectors").vi);
        }
    }
}