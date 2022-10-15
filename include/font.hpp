#if !defined(PDFLIB_FONT_H)
    #define PDFLIB_FONT_H
    #include "face.hpp"
    #include "private/font-tables/parser.hpp"
    #include "private/harfbuzz-helpers.hpp"
    #include "private/types/font.hpp"

    #include <hb-ot.h>
    #include <hb.h>
    #include <qpdf/Buffer.hh>
    #include <qpdf/QPDFObjectHandle.hh>
    #include <qpdf/QUtil.hh>

    #include <climits>
    #include <filesystem>
    #include <string_view>
    #include <vector>

namespace PDFLib {
class FontManager;
class Document;

class Font {
    FontHolder font;
    std::vector<Face> faces;
    FontManager &manager;
    const size_t index;

    class HeadTable {
        static constexpr uint32_t Tag = 1751474532;
        FontTableParser parser;

      public:
        uint16_t majorVersion{parser.getNext()};
        uint16_t minorVersion{parser.getNext()};
        uint32_t fontRevision{parser.getNext<uint32_t>()};
        uint32_t checksum{parser.getNext<uint32_t>()};
        uint32_t magicNumber{parser.getNext<uint32_t>()};
        uint16_t flags{parser.getNext()};
        uint16_t unitsPerEM{parser.getNext()};
        uint64_t created{parser.getNext<uint64_t>()};
        uint64_t modified{parser.getNext<uint64_t>()};
        int16_t xMin{parser.getNext<int16_t>()};
        int16_t yMin{parser.getNext<int16_t>()};
        int16_t xMax{parser.getNext<int16_t>()};
        int16_t yMax{parser.getNext<int16_t>()};
        uint16_t macStyle{parser.getNext()};
        uint16_t lowestRecPPEM{parser.getNext()};
        int16_t fontDirectionHint{parser.getNext<int16_t>()};
        int16_t indexToLocFormat{parser.getNext<int16_t>()};
        int16_t glyphDataFormat{parser.getNext<int16_t>()};

        HeadTable(HbFontT *font) : parser{font, Tag} {};

    } headTable;

    class OS2Table {
        static constexpr uint32_t Tag = 1751474532;
        FontTableParser parser;

      public:
        uint16_t version{parser.getNext()};
        int16_t xAvgCharWidth{parser.getNext<int16_t>()};
        uint16_t usWeightClass{parser.getNext()};
        uint16_t usWidthClass{parser.getNext()};
        uint32_t fsType{parser.getNext()};
        int16_t ySubscriptXSize{parser.getNext<int16_t>()};
        int16_t ySubscriptYSize{parser.getNext<int16_t>()};
        int16_t ySubscriptXOffset{parser.getNext<int16_t>()};
        int16_t ySubscriptYOffset{parser.getNext<int16_t>()};
        int16_t ySuperscriptXSize{parser.getNext<int16_t>()};
        int16_t ySuperscriptYSize{parser.getNext<int16_t>()};
        int16_t ySuperscriptXOffset{parser.getNext<int16_t>()};
        int16_t ySuperscriptYOffset{parser.getNext<int16_t>()};
        int16_t yStrikeoutSize{parser.getNext<int16_t>()};
        int16_t yStrikeoutPosition{parser.getNext<int16_t>()};
        int16_t sFamilyClass{parser.getNext<int16_t>()};
        uint8_t panose[10]{parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>()};
        uint32_t ulUnicodeRange1{parser.getNext<uint32_t>()};
        uint32_t ulUnicodeRange2{parser.getNext<uint32_t>()};
        uint32_t ulUnicodeRange3{parser.getNext<uint32_t>()};
        uint32_t ulUnicodeRange4{parser.getNext<uint32_t>()};
        uint32_t achVendID{parser.getNext<uint32_t>()};
        uint16_t fsSelection{parser.getNext()};
        uint16_t usFirstCharIndex{parser.getNext()};
        uint16_t usLastCharIndex{parser.getNext()};
        int16_t sTypoAscender{parser.getNext<int16_t>()};
        int16_t sTypoDescender{parser.getNext<int16_t>()};
        int16_t sTypoLineGap{parser.getNext<int16_t>()};
        uint16_t usWinAscent{parser.getNext()};
        uint16_t usWinDescent{parser.getNext()};
        uint16_t usCodePageRange1{static_cast<uint16_t>(version > 0 ? parser.getNext() : 0)};
        uint16_t usCodePageRange2{static_cast<uint16_t>(version > 0 ? parser.getNext() : 0)};
        int16_t sxHeight{static_cast<int16_t>(version > 1 ? parser.getNext<int16_t>() : 0)};
        int16_t sxCapHeight{static_cast<int16_t>(version > 1 ? parser.getNext<int16_t>() : 0)};
        uint16_t usDefaultChar{static_cast<uint16_t>(version > 1 ? parser.getNext() : 0)};
        uint16_t usBreakChar{static_cast<uint16_t>(version > 1 ? parser.getNext() : 0)};
        uint16_t usMaxContext{static_cast<uint16_t>(version > 1 ? parser.getNext() : 0)};
        uint16_t usLowerOpticalPointSize{static_cast<uint16_t>(version > 4 ? parser.getNext() : 0)};
        uint16_t usUpperOpticalPointSize{static_cast<uint16_t>(version > 4 ? parser.getNext() : 0)};

        OS2Table(HbFontT *font) : parser{font, Tag} {};

    } os2Table;

    class NameTable {
      public:
        static constexpr uint32_t Tag = 1851878757;
        enum class RecordType : uint16_t {
            Family = 1,
            SubFamily,
            UID,
            FullName,
            Version,
            PostScriptName,
            TypographicFamily = 16,
            TypographicSubFamily,
            CompatibleFull,
            VariationsPostScriptNamePrefix = 25
        };
        enum class LanguageID {
            En,
            Fr,
            De,
            It,
            Nl,
            Sv,
            Es,
            Da,
            Pt,
            No,
            He,
            Ja,
            Ar,
            Fi,
            El,
            Is,
            Mt,
            Tr,
            Hr,
            ZhHant,
            Ur,
            Hi,
            Th,
            Ko,
            Lt,
            Pl,
            Hu,
            Lv,
            Se,
            Fo,
            Fa,
            Ru,
            Zh,
            NlBE,
            Ga,
            Sq,
            Ro,
            Cz,
            Sk,
            Si,
            Yi,
            Sr,
            Mk,
            Bg,
            Uk,
            Be,
            Uz,
            Kk,
            AzCyrl,
            AzArab,
            Hy,
            Ka,
            Mo,
            Ky,
            Tg,
            Tk,
            MnCN,
            Mn,
            Ps,
            Ks,
            Ku,
            Sd,
            Bo,
            Ne,
            Sa,
            Mr,
            Bn,
            As,
            Gu,
            Pa,
            Or,
            Ml,
            Kn,
            Ta,
            Te,
            My,
            Km,
            Lo,
            Vi,
            Id,
            Tl,
            Ms,
            MsArab,
            Am,
            Ti,
            Om,
            So,
            Sw,
            Rw,
            Rn,
            Ny,
            Mg,
            Eo,
            Cy,
            Eu,
            Ca,
            La,
            Qu,
            Gn,
            Ay,
            Tt,
            Ug,
            Dz,
            Jv,
            Su,
            Gl,
            Af,
            Br,
            Iu,
            Gd,
            Gv,
            To,
            ElPolyton,
            Kl,
            Az,
            Nn,
            Unknown
        };

      private:
        enum class WinLanguageID {
            Af = 0x0436,
            Sq = 0x041C,
            Gsw = 0x0484,
            Am = 0x045E,
            ArDZ = 0x1401,
            ArBH = 0x3C01,
            Ar = 0x0C01,
            ArIQ = 0x0801,
            ArJO = 0x2C01,
            ArKW = 0x3401,
            ArLB = 0x3001,
            ArLY = 0x1001,
            ArMA = 0x1801,
            ArOM = 0x2001,
            ArQA = 0x4001,
            ArSA = 0x0401,
            ArSY = 0x2801,
            ArTN = 0x1C01,
            ArAE = 0x3801,
            ArYE = 0x2401,
            Hy = 0x042B,
            As = 0x044D,
            AzCyrl = 0x082C,
            Az = 0x042C,
            Ba = 0x046D,
            Eu = 0x042D,
            Be = 0x0423,
            Bn = 0x0845,
            BnIN = 0x0445,
            BsCyrl = 0x201A,
            Bs = 0x141A,
            Br = 0x047E,
            Bg = 0x0402,
            Ca = 0x0403,
            ZhHK = 0x0C04,
            ZhMO = 0x1404,
            Zh = 0x0804,
            ZhSG = 0x1004,
            ZhTW = 0x0404,
            Co = 0x0483,
            Hr = 0x041A,
            HrBA = 0x101A,
            Cs = 0x0405,
            Da = 0x0406,
            Prs = 0x048C,
            Dv = 0x0465,
            NlBE = 0x0813,
            Nl = 0x0413,
            EnAU = 0x0C09,
            EnBZ = 0x2809,
            EnCA = 0x1009,
            En029 = 0x2409,
            EnIN = 0x4009,
            EnIE = 0x1809,
            EnJM = 0x2009,
            EnMY = 0x4409,
            EnNZ = 0x1409,
            EnPH = 0x3409,
            EnSG = 0x4809,
            EnZA = 0x1C09,
            EnTT = 0x2C09,
            EnGB = 0x0809,
            En = 0x0409,
            EnZW = 0x3009,
            Et = 0x0425,
            Fo = 0x0438,
            Fil = 0x0464,
            Fi = 0x040B,
            FrBE = 0x080C,
            FrCA = 0x0C0C,
            Fr = 0x040C,
            FrLU = 0x140C,
            FrMC = 0x180C,
            FrCH = 0x100C,
            Fy = 0x0462,
            Gl = 0x0456,
            Ka = 0x0437,
            DeAT = 0x0C07,
            De = 0x0407,
            DeLI = 0x1407,
            DeLU = 0x1007,
            DeCH = 0x0807,
            El = 0x0408,
            Kl = 0x046F,
            Gu = 0x0447,
            Ha = 0x0468,
            He = 0x040D,
            Hi = 0x0439,
            Hu = 0x040E,
            Is = 0x040F,
            Ig = 0x0470,
            Id = 0x0421,
            Iu = 0x045D,
            IuLatn = 0x085D,
            Ga = 0x083C,
            Xh = 0x0434,
            Zu = 0x0435,
            It = 0x0410,
            ItCH = 0x0810,
            Ja = 0x0411,
            Kn = 0x044B,
            Kk = 0x043F,
            Km = 0x0453,
            Quc = 0x0486,
            Rw = 0x0487,
            Sw = 0x0441,
            Kok = 0x0457,
            Ko = 0x0412,
            Ky = 0x0440,
            Lo = 0x0454,
            Lv = 0x0426,
            Lt = 0x0427,
            Dsb = 0x082E,
            Lb = 0x046E,
            Mk = 0x042F,
            MsBN = 0x083E,
            Ms = 0x043E,
            Ml = 0x044C,
            Mt = 0x043A,
            Mi = 0x0481,
            Arn = 0x047A,
            Mr = 0x044E,
            Moh = 0x047C,
            Mn = 0x0450,
            MnCN = 0x0850,
            Ne = 0x0461,
            Nb = 0x0414,
            Nn = 0x0814,
            Oc = 0x0482,
            Or = 0x0448,
            Ps = 0x0463,
            Pl = 0x0415,
            Pt = 0x0416,
            PtPT = 0x0816,
            Pa = 0x0446,
            QuBO = 0x046B,
            QuEC = 0x086B,
            Qu = 0x0C6B,
            Ro = 0x0418,
            Rm = 0x0417,
            Ru = 0x0419,
            Smn = 0x243B,
            SmjNO = 0x103B,
            Smj = 0x143B,
            SeFI = 0x0C3B,
            Se = 0x043B,
            SeSE = 0x083B,
            Sms = 0x203B,
            SmaNO = 0x183B,
            Sms1 = 0x1C3B,
            Sa = 0x044F,
            SrCyrlBA = 0x1C1A,
            Sr = 0x0C1A,
            SrLatnBA = 0x181A,
            SrLatn = 0x081A,
            Nso = 0x046C,
            Tn = 0x0432,
            Si = 0x045B,
            Sk = 0x041B,
            Sl = 0x0424,
            EsAR = 0x2C0A,
            EsBO = 0x400A,
            EsCL = 0x340A,
            EsCO = 0x240A,
            EsCR = 0x140A,
            EsDO = 0x1C0A,
            EsEC = 0x300A,
            EsSV = 0x440A,
            EsGT = 0x100A,
            EsHN = 0x480A,
            EsMX = 0x080A,
            EsNI = 0x4C0A,
            EsPA = 0x180A,
            EsPY = 0x3C0A,
            EsPE = 0x280A,
            EsPR = 0x500A,
            Es = 0x0C0A,
            Es1 = 0x040A,
            EsUS = 0x540A,
            EsUY = 0x380A,
            EsVE = 0x200A,
            SvFI = 0x081D,
            Sv = 0x041D,
            Syr = 0x045A,
            Tg = 0x0428,
            Tzm = 0x085F,
            Ta = 0x0449,
            Tt = 0x0444,
            Te = 0x044A,
            Th = 0x041E,
            Bo = 0x0451,
            Tr = 0x041F,
            Tk = 0x0442,
            Ug = 0x0480,
            Uk = 0x0422,
            Hsb = 0x042E,
            Ur = 0x0420,
            UzCyrl = 0x0843,
            Uz = 0x0443,
            Vi = 0x042A,
            Cy = 0x0452,
            Wo = 0x0488,
            Sah = 0x0485,
            Ii = 0x0478,
            Yo = 0x046A
        };

        enum class MacLanguageID {
            En,
            Fr,
            De,
            It,
            Nl,
            Sv,
            Es,
            Da,
            Pt,
            No,
            He,
            Ja,
            Ar,
            Fi,
            El,
            Is,
            Mt,
            Tr,
            Hr,
            ZhHant,
            Ur,
            Hi,
            Th,
            Ko,
            Lt,
            Pl,
            Hu,
            Es2,
            Lv,
            Se,
            Fo,
            Fa,
            Ru,
            Zh,
            NlBE,
            Ga,
            Sq,
            Ro,
            Cz,
            Sk,
            Si,
            Yi,
            Sr,
            Mk,
            Bg,
            Uk,
            Be,
            Uz,
            Kk,
            AzCyrl,
            AzArab,
            Hy,
            Ka,
            Mo,
            Ky,
            Tg,
            Tk,
            MnCN,
            Mn,
            Ps,
            Ks,
            Ku,
            Sd,
            Bo,
            Ne,
            Sa,
            Mr,
            Bn,
            As,
            Gu,
            Pa,
            Or,
            Ml,
            Kn,
            Ta,
            Te,
            Si2,
            My,
            Km,
            Lo,
            Vi,
            Id,
            Tl,
            Ms,
            MsArab,
            Am,
            Ti,
            Om,
            So,
            Sw,
            Rw,
            Rn,
            Ny,
            Mg,
            Eo,
            Cy = 123,
            Eu,
            Ca,
            La,
            Qu,
            Gn,
            Ay,
            Tt,
            Ug,
            Dz,
            Jv,
            Su,
            Gl,
            Af,
            Br,
            Iu,
            Gd,
            Gv,
            Ga2,
            To,
            ElPolyton,
            Kl,
            Az,
            Nn
        };

        static inline std::map<WinLanguageID, LanguageID> WinMap{
            {WinLanguageID::Af, LanguageID::Af},         {WinLanguageID::Sq, LanguageID::Sq},
            {WinLanguageID::Gsw, LanguageID::De},        {WinLanguageID::Am, LanguageID::Am},
            {WinLanguageID::ArDZ, LanguageID::Ar},       {WinLanguageID::ArBH, LanguageID::Ar},
            {WinLanguageID::Ar, LanguageID::Ar},         {WinLanguageID::ArIQ, LanguageID::Ar},
            {WinLanguageID::ArJO, LanguageID::Ar},       {WinLanguageID::ArKW, LanguageID::Ar},
            {WinLanguageID::ArLB, LanguageID::Ar},       {WinLanguageID::ArLY, LanguageID::Ar},
            {WinLanguageID::ArMA, LanguageID::Ar},       {WinLanguageID::ArOM, LanguageID::Ar},
            {WinLanguageID::ArQA, LanguageID::Ar},       {WinLanguageID::ArSA, LanguageID::Ar},
            {WinLanguageID::ArSY, LanguageID::Ar},       {WinLanguageID::ArTN, LanguageID::Ar},
            {WinLanguageID::ArAE, LanguageID::Ar},       {WinLanguageID::ArYE, LanguageID::Ar},
            {WinLanguageID::Hy, LanguageID::Am},         {WinLanguageID::As, LanguageID::As},
            {WinLanguageID::AzCyrl, LanguageID::AzCyrl}, {WinLanguageID::Az, LanguageID::Az},
            {WinLanguageID::Ba, LanguageID::Ru},         {WinLanguageID::Eu, LanguageID::Eu},
            {WinLanguageID::Be, LanguageID::Be},         {WinLanguageID::Bn, LanguageID::Bn},
            {WinLanguageID::BnIN, LanguageID::Bn},       {WinLanguageID::BsCyrl, LanguageID::Ru},
            {WinLanguageID::Bs, LanguageID::En},         {WinLanguageID::Br, LanguageID::Br},
            {WinLanguageID::Bg, LanguageID::Bg},         {WinLanguageID::Ca, LanguageID::Ca},
            {WinLanguageID::ZhHK, LanguageID::Zh},       {WinLanguageID::ZhMO, LanguageID::Zh},
            {WinLanguageID::Zh, LanguageID::Zh},         {WinLanguageID::ZhSG, LanguageID::Zh},
            {WinLanguageID::ZhTW, LanguageID::Zh},       {WinLanguageID::Co, LanguageID::Fr},
            {WinLanguageID::Hr, LanguageID::Hr},         {WinLanguageID::HrBA, LanguageID::En},
            {WinLanguageID::Cs, LanguageID::Pl},         {WinLanguageID::Da, LanguageID::Da},
            {WinLanguageID::Prs, LanguageID::Fa},        {WinLanguageID::Dv, LanguageID::Unknown},
            {WinLanguageID::NlBE, LanguageID::NlBE},     {WinLanguageID::Nl, LanguageID::Nl},
            {WinLanguageID::EnAU, LanguageID::En},       {WinLanguageID::EnBZ, LanguageID::En},
            {WinLanguageID::EnCA, LanguageID::En},       {WinLanguageID::En029, LanguageID::En},
            {WinLanguageID::EnIN, LanguageID::En},       {WinLanguageID::EnIE, LanguageID::En},
            {WinLanguageID::EnJM, LanguageID::En},       {WinLanguageID::EnMY, LanguageID::En},
            {WinLanguageID::EnNZ, LanguageID::En},       {WinLanguageID::EnPH, LanguageID::En},
            {WinLanguageID::EnSG, LanguageID::En},       {WinLanguageID::EnZA, LanguageID::En},
            {WinLanguageID::EnTT, LanguageID::En},       {WinLanguageID::EnGB, LanguageID::En},
            {WinLanguageID::En, LanguageID::En},         {WinLanguageID::EnZW, LanguageID::En},
            {WinLanguageID::Et, LanguageID::En},         {WinLanguageID::Fo, LanguageID::Fo},
            {WinLanguageID::Fil, LanguageID::Unknown},   {WinLanguageID::Fi, LanguageID::Fi},
            {WinLanguageID::FrBE, LanguageID::Fr},       {WinLanguageID::FrCA, LanguageID::Fr},
            {WinLanguageID::Fr, LanguageID::Fr},         {WinLanguageID::FrLU, LanguageID::Fr},
            {WinLanguageID::FrMC, LanguageID::Fr},       {WinLanguageID::FrCH, LanguageID::Fr},
            {WinLanguageID::Fy, LanguageID::Unknown},    {WinLanguageID::Gl, LanguageID::Gl},
            {WinLanguageID::Ka, LanguageID::Ka},         {WinLanguageID::DeAT, LanguageID::De},
            {WinLanguageID::De, LanguageID::De},         {WinLanguageID::DeLI, LanguageID::De},
            {WinLanguageID::DeLU, LanguageID::De},       {WinLanguageID::DeCH, LanguageID::De},
            {WinLanguageID::El, LanguageID::El},         {WinLanguageID::Kl, LanguageID::Kl},
            {WinLanguageID::Gu, LanguageID::Gu},         {WinLanguageID::Ha, LanguageID::En},
            {WinLanguageID::He, LanguageID::He},         {WinLanguageID::Hi, LanguageID::Hi},
            {WinLanguageID::Hu, LanguageID::Hu},         {WinLanguageID::Is, LanguageID::Is},
            {WinLanguageID::Ig, LanguageID::Unknown},    {WinLanguageID::Id, LanguageID::Id},
            {WinLanguageID::Iu, LanguageID::Iu},         {WinLanguageID::IuLatn, LanguageID::En},
            {WinLanguageID::Ga, LanguageID::Ga},         {WinLanguageID::Xh, LanguageID::Unknown},
            {WinLanguageID::Zu, LanguageID::Unknown},    {WinLanguageID::It, LanguageID::It},
            {WinLanguageID::ItCH, LanguageID::It},       {WinLanguageID::Ja, LanguageID::Ja},
            {WinLanguageID::Kn, LanguageID::Kn},         {WinLanguageID::Kk, LanguageID::Kk},
            {WinLanguageID::Km, LanguageID::Km},         {WinLanguageID::Quc, LanguageID::Unknown},
            {WinLanguageID::Rw, LanguageID::Rw},         {WinLanguageID::Sw, LanguageID::Sw},
            {WinLanguageID::Kok, LanguageID::Unknown},   {WinLanguageID::Ko, LanguageID::Ko},
            {WinLanguageID::Ky, LanguageID::Ky},         {WinLanguageID::Lo, LanguageID::Lo},
            {WinLanguageID::Lv, LanguageID::Lv},         {WinLanguageID::Lt, LanguageID::Lt},
            {WinLanguageID::Dsb, LanguageID::Unknown},   {WinLanguageID::Lb, LanguageID::Unknown},
            {WinLanguageID::Mk, LanguageID::Mk},         {WinLanguageID::MsBN, LanguageID::Ms},
            {WinLanguageID::Ms, LanguageID::Ms},         {WinLanguageID::Ml, LanguageID::Ml},
            {WinLanguageID::Mt, LanguageID::Mt},         {WinLanguageID::Mi, LanguageID::Unknown},
            {WinLanguageID::Arn, LanguageID::Unknown},   {WinLanguageID::Mr, LanguageID::Mr},
            {WinLanguageID::Moh, LanguageID::Unknown},   {WinLanguageID::Mn, LanguageID::Mn},
            {WinLanguageID::MnCN, LanguageID::MnCN},     {WinLanguageID::Ne, LanguageID::Ne},
            {WinLanguageID::Nb, LanguageID::Unknown},    {WinLanguageID::Nn, LanguageID::Nn},
            {WinLanguageID::Oc, LanguageID::Fr},         {WinLanguageID::Or, LanguageID::Or},
            {WinLanguageID::Ps, LanguageID::Ps},         {WinLanguageID::Pl, LanguageID::Pl},
            {WinLanguageID::Pt, LanguageID::Pt},         {WinLanguageID::PtPT, LanguageID::Pt},
            {WinLanguageID::Pa, LanguageID::Pa},         {WinLanguageID::QuBO, LanguageID::Qu},
            {WinLanguageID::QuEC, LanguageID::Qu},       {WinLanguageID::Qu, LanguageID::Qu},
            {WinLanguageID::Ro, LanguageID::Ro},         {WinLanguageID::Rm, LanguageID::Unknown},
            {WinLanguageID::Ru, LanguageID::Ru},         {WinLanguageID::Smn, LanguageID::Unknown},
            {WinLanguageID::SmjNO, LanguageID::Unknown}, {WinLanguageID::Smj, LanguageID::Unknown},
            {WinLanguageID::SeFI, LanguageID::Se},       {WinLanguageID::Se, LanguageID::Se},
            {WinLanguageID::SeSE, LanguageID::Se},       {WinLanguageID::Sms, LanguageID::Unknown},
            {WinLanguageID::SmaNO, LanguageID::Unknown}, {WinLanguageID::Sms1, LanguageID::Unknown},
            {WinLanguageID::Sa, LanguageID::Sa},         {WinLanguageID::SrCyrlBA, LanguageID::Ru},
            {WinLanguageID::Sr, LanguageID::Sr},         {WinLanguageID::SrLatnBA, LanguageID::En},
            {WinLanguageID::SrLatn, LanguageID::En},     {WinLanguageID::Nso, LanguageID::Unknown},
            {WinLanguageID::Tn, LanguageID::Unknown},    {WinLanguageID::Si, LanguageID::Si},
            {WinLanguageID::Sk, LanguageID::Sk},         {WinLanguageID::Sl, LanguageID::Pl},
            {WinLanguageID::EsAR, LanguageID::Es},       {WinLanguageID::EsBO, LanguageID::Es},
            {WinLanguageID::EsCL, LanguageID::Es},       {WinLanguageID::EsCO, LanguageID::Es},
            {WinLanguageID::EsCR, LanguageID::Es},       {WinLanguageID::EsDO, LanguageID::Es},
            {WinLanguageID::EsEC, LanguageID::Es},       {WinLanguageID::EsSV, LanguageID::Es},
            {WinLanguageID::EsGT, LanguageID::Es},       {WinLanguageID::EsHN, LanguageID::Es},
            {WinLanguageID::EsMX, LanguageID::Es},       {WinLanguageID::EsNI, LanguageID::Es},
            {WinLanguageID::EsPA, LanguageID::Es},       {WinLanguageID::EsPY, LanguageID::Es},
            {WinLanguageID::EsPE, LanguageID::Es},       {WinLanguageID::EsPR, LanguageID::Es},
            {WinLanguageID::Es, LanguageID::Es},         {WinLanguageID::Es1, LanguageID::Es},
            {WinLanguageID::EsUS, LanguageID::Es},       {WinLanguageID::EsUY, LanguageID::Es},
            {WinLanguageID::EsVE, LanguageID::Es},       {WinLanguageID::SvFI, LanguageID::Sv},
            {WinLanguageID::Sv, LanguageID::Sv},         {WinLanguageID::Syr, LanguageID::Unknown},
            {WinLanguageID::Tg, LanguageID::Tg},         {WinLanguageID::Tzm, LanguageID::Unknown},
            {WinLanguageID::Ta, LanguageID::Ta},         {WinLanguageID::Tt, LanguageID::Tt},
            {WinLanguageID::Te, LanguageID::Te},         {WinLanguageID::Th, LanguageID::Th},
            {WinLanguageID::Bo, LanguageID::Bo},         {WinLanguageID::Tr, LanguageID::Tr},
            {WinLanguageID::Tk, LanguageID::Tk},         {WinLanguageID::Ug, LanguageID::Ug},
            {WinLanguageID::Uk, LanguageID::Uk},         {WinLanguageID::Hsb, LanguageID::Unknown},
            {WinLanguageID::Ur, LanguageID::Ur},         {WinLanguageID::UzCyrl, LanguageID::Uz},
            {WinLanguageID::Uz, LanguageID::Uz},         {WinLanguageID::Vi, LanguageID::Vi},
            {WinLanguageID::Cy, LanguageID::Cy},         {WinLanguageID::Wo, LanguageID::Unknown},
            {WinLanguageID::Sah, LanguageID::Unknown},   {WinLanguageID::Ii, LanguageID::Unknown},
            {WinLanguageID::Yo, LanguageID::Unknown},
        };
        enum class PlatformID { Unicode, Macintosh, Windows = 3 };

        struct Record {
            LanguageID language;
            std::string data;
        };

        std::map<RecordType, Record> records;

      public:
        NameTable(HbFontT *font) {
            FontTableParser parser{font, Tag};
            uint16_t version = parser.getNext();
            uint16_t nRecords = parser.getNext();
            auto *tableOffset = parser.startPtr + parser.getNext();
            for (size_t i = 0; i < nRecords; i++) {
                auto platformId = static_cast<PlatformID>(parser.getNext());
                auto encodingId = parser.getNext();
                auto languageId = parser.getNext();
                auto nameId = static_cast<RecordType>(parser.getNext());
                auto stringLength = parser.getNext();
                auto stringOffset = parser.getNext();
                if (nameId > RecordType::VariationsPostScriptNamePrefix) // Not a standard name
                    continue;

                LanguageID language;
                switch (platformId) {
                    case PlatformID::Macintosh:
                        if (languageId <= static_cast<uint16_t>(MacLanguageID::Eo))
                            language = static_cast<LanguageID>(languageId);
                        else
                            language = static_cast<LanguageID>(languageId + static_cast<uint16_t>(MacLanguageID::Eo) -
                                                               static_cast<uint16_t>(MacLanguageID::Cy) - 1);
                        break;
                    case PlatformID::Windows:
                        language = WinMap.contains(static_cast<WinLanguageID>(languageId))
                                       ? WinMap.at(static_cast<WinLanguageID>(languageId))
                                       : LanguageID::Unknown;
                        break;
                    case PlatformID::Unicode:
                        language = LanguageID::En;
                        break;
                };
                if ((!records.contains(nameId)) ||
                    (language == LanguageID::En && records.at(nameId).language != LanguageID::En)) {
                    auto &ref = records[nameId];
                    ref.language = language;
                    switch (platformId) {
                        case PlatformID::Unicode:
                        case PlatformID::Windows:
                            ref.data = QUtil::utf16_to_utf8(std::string((tableOffset + stringOffset), stringLength));
                            break;
                        case PlatformID::Macintosh:
                            ref.data = ref.data =
                                QUtil::mac_roman_to_utf8(std::string((tableOffset + stringOffset), stringLength));
                            break;
                    }
                }
            }
        };

        bool contains(RecordType type) const {
            return records.contains(type);
        }

        const std::string operator[](RecordType type) const {
            return records.at(type).data;
        };

        const Record get(RecordType type) const {
            return records.at(type);
        }

    } nameTable;

  public:
    const double scale;
    BlobHolder blob;

    Font(HbFontT *fontHandle, FontManager &manager, size_t index);

    HbFontT *getHbObj() {
        return font;
    }

    std::string getFamily() {
        if (nameTable.contains(NameTable::RecordType::TypographicFamily))
            return nameTable[NameTable::RecordType::TypographicFamily];
        else
            return nameTable[NameTable::RecordType::Family];
    }

    std::string getName() {
        if (nameTable.contains(NameTable::RecordType::CompatibleFull))
            return nameTable[NameTable::RecordType::CompatibleFull];
        else
            return nameTable[NameTable::RecordType::FullName];
    }

    std::string getSubFamily() {
        if (nameTable.contains(NameTable::RecordType::TypographicSubFamily))
            return nameTable[NameTable::RecordType::TypographicSubFamily];
        else
            return nameTable[NameTable::RecordType::SubFamily];
    }

    std::string getPostScriptName() {
        return nameTable[NameTable::RecordType::PostScriptName];
    }

    BBox getBoundingBox() {
        BBox out{headTable.xMin, headTable.yMin, headTable.xMax, headTable.yMax};
        out *= scale;
        return out;
    }

    int getVariations() {
        return hb_ot_var_get_axis_count(font);
    }

    FontHolder makeSubset(Face &face);
    Face &makeFace();

    decltype(faces) &getFaces() {
        return faces;
    }

    FontManager &getManager() {
        return manager;
    }
};
} // namespace PDFLib

#endif // PDFLIB_FONT_H
