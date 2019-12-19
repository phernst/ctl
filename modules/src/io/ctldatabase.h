#ifndef CTLDATABASEHANDLER_H
#define CTLDATABASEHANDLER_H

#include "models/abstractdatamodel.h"
#include "models/tabulateddatamodel.h"
#include "io/jsonserializer.h"

#include <QDir>
namespace CTL {

namespace database {

enum class Element {
    H = 1,
    He = 2,
    Li = 3,
    Be = 4,
    B = 5,
    C = 6,
    N = 7,
    O = 8,
    F = 9,
    Ne = 10,
    Na = 11,
    Mg = 12,
    Al = 13,
    Si = 14,
    P = 15,
    S = 16,
    Cl = 17,
    Ar = 18,
    K = 19,
    Ca = 20,
    Sc = 21,
    Ti = 22,
    V = 23,
    Cr = 24,
    Mn = 25,
    Fe = 26,
    Co = 27,
    Ni = 28,
    Cu = 29,
    Zn = 30,
    Ga = 31,
    Ge = 32,
    As = 33,
    Se = 34,
    Br = 35,
    Kr = 36,
    Rb = 37,
    Sr = 38,
    Y = 39,
    Zr = 40,
    Nb = 41,
    Mo = 42,
    Tc = 43,
    Ru = 44,
    Rh = 45,
    Pd = 46,
    Ag = 47,
    Cd = 48,
    In = 49,
    Sn = 50,
    Sb = 51,
    Te = 52,
    I = 53,
    Xe = 54,
    Cs = 55,
    Ba = 56,
    La = 57,
    Ce = 58,
    Pr = 59,
    Nd = 60,
    Pm = 61,
    Sm = 62,
    Eu = 63,
    Gd = 64,
    Tb = 65,
    Dy = 66,
    Ho = 67,
    Er = 68,
    Tm = 69,
    Yb = 70,
    Lu = 71,
    Hf = 72,
    Ta = 73,
    W = 74,
    Re = 75,
    Os = 76,
    Ir = 77,
    Pt = 78,
    Au = 79,
    Hg = 80,
    Tl = 81,
    Pb = 82,
    Bi = 83,
    Po = 84,
    At = 85,
    Rn = 86,
    Fr = 87,
    Ra = 88,
    Ac = 89,
    Th = 90,
    Pa = 91,
    U = 92
};

enum class Composite {
    Adipose_Tissue = 1001,
    Air = 1002,
    Air_Equivalent_Plastic = 1003,
    Alanine = 1004,
    Ammonium_Cerium_Sulfate_Solution = 1005,
    Bakelite = 1006,
    Blood = 1007,
    Bone_Cortical = 1008,
    Bone_Equivalent_Plastic = 1009,
    Brain = 1010,
    Breast = 1011,
    Cadmium_Telluride = 1012,
    Calcium_Fluoride = 1013,
    Calcium_Sulfate = 1014,
    Cesium_Iodide = 1015,
    Concrete_Barite = 1016,
    Concrete_Ordinary = 1017,
    Eye_Lens = 1018,
    Ferrous_Sulfate = 1019,
    Gadolinium_Oxysulfide = 1020,
    Gafchromic_Sensor = 1021,
    Gallium_Arsenide = 1022,
    Glass_Borosilicate = 1023,
    Glass_Lead = 1024,
    Lithium_Fluoride = 1025,
    Lithium_Tetraborate = 1026,
    Lung = 1027,
    Magnesium_Tetraborate = 1028,
    Mercuric_Iodide = 1029,
    Muscle_Skeletal = 1030,
    Ovary = 1031,
    Photographic_Emulsion_Kodak_Type_AA = 1032,
    Photographic_Emulsion_Standard_Nuclear = 1033,
    Plastic_Scintillator_Vinyltoluene = 1034,
    Polyethylene = 1035,
    Polyethylene_Terephthalate = 1036,
    Polymethyl_Methacrylate = 1037,
    Polystyrene = 1038,
    PVC = 1039,
    Radiochromic_Dye_Film = 1040,
    Teflon = 1041,
    Testis = 1042,
    Tissue_Equivalent_Gas_Methane_Based = 1043,
    Tissue_Equivalent_Gas_Propane_Based = 1044,
    Tissue_Equivalent_Plastic = 1045,
    Tissue_Soft = 1046,
    Tissue_Soft_Four_Component = 1047,
    Water = 1048
};

enum class Spectrum {
  Example = 2001
};

std::shared_ptr<AbstractIntegrableDataModel> attenuationModel(database::Element Element);
std::shared_ptr<AbstractIntegrableDataModel> attenuationModel(database::Composite Composite);
std::shared_ptr<TabulatedDataModel> xRaySpectrum(database::Spectrum Spectrum);

}

class CTLDatabaseHandler
{
public:
    static CTLDatabaseHandler& instance();

    void setDataBaseRoot(const QString& path);

    std::shared_ptr<AbstractIntegrableDataModel> loadAttenuationModel(database::Composite composite);
    std::shared_ptr<AbstractIntegrableDataModel> loadAttenuationModel(database::Element element);
    std::shared_ptr<TabulatedDataModel> loadXRaySpectrum(database::Spectrum spectrum);

private:
    CTLDatabaseHandler();
    // non-copyable
    CTLDatabaseHandler(const CTLDatabaseHandler&) = delete;
    CTLDatabaseHandler& operator=(const CTLDatabaseHandler&) = delete;

    void makeFileMap();

    QDir _dbRoot;

    JsonSerializer _serializer;
    QMap<int, QString> _fileMap;
};


} // namespace CTL

#endif // CTLDATABASEHANDLER_H
