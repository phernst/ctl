#ifndef SPECTRALEFFECTSEXTENSION_H
#define SPECTRALEFFECTSEXTENSION_H

#include "projectorextension.h"
#include "abstractprojectorconfig.h"
#include "acquisition/acquisitionsetup.h"
#include "processing/coordinates.h" // Range<T>
#include "acquisition/radiationencoder.h"

namespace CTL {

class SpectralEffectsExtension : public ProjectorExtension
{
    CTL_TYPE_ID(104)

public:
    using ProjectorExtension::ProjectorExtension;
    SpectralEffectsExtension() = default;
    explicit SpectralEffectsExtension(float energyBinWidth);

    void configure(const AcquisitionSetup& setup) override;
    ProjectionData project(const VolumeData& volume) override;

    ProjectionData projectComposite(const CompositeVolume& volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    void fromVariant(const QVariant &variant) override;
    QVariant toVariant() const override;

    void setSpectralSamplingResolution(float energyBinWidth);

private:
//    struct BinInformation
//    {
//        std::vector<double> intensities;        // for each view
//        std::vector<double> adjustedFluxMods;   // for each view
//        float energy;
//    };

//    struct SpectralInformation
//    {
//        std::vector<BinInformation> bins;       // for each bin
//        std::vector<double> totalIntensities;   // for each view
//        float binWidth{};
//        uint nbSamples{};

//        Range<float> fullCoverage = { std::numeric_limits<float>::max(), 0.0f };
//        float highestResolution = std::numeric_limits<float>::max();

//        void reserveMemory(uint nbViews);
//    };

    AcquisitionSetup _setup; //!< A copy of the setup used for acquisition.
    SpectralInformation _spectralInfo;
    float _deltaE = 0.0f;   

    void updateSpectralInformation();
//    void determineSampling();
//    void extractViewSpectrum(uint view);
    void applyDetectorResponse(ProjectionData& intensity, float energy) const;

    using BinInformation = SpectralInformation::BinInformation;

    ProjectionData projectLinear(const CompositeVolume& volume);
    ProjectionData singleBinIntensityLinear(const std::vector<ProjectionData>& materialProjs,
                                            const std::vector<float>& mu,
                                            const BinInformation& binInfo);
    ProjectionData singleBinIntensityNonLinear(const CompositeVolume& volume,
                                               const BinInformation& binInfo);
    ProjectionData projectNonLinear(const CompositeVolume& volume);

    void addDummyPrepareSteps();
    void removeDummyPrepareSteps();
    void replaceDummyPrepareSteps(const BinInformation& binInfo, float binWidth);
};


} // namespace CTL

#endif // SPECTRALEFFECTSEXTENSION_H
