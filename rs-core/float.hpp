#pragma once

#include "rs-core/common.hpp"
#include <cmath>
#include <cstdlib>
#include <type_traits>

namespace RS {

    // Arithmetic constants

    #define RS_DEFINE_CONSTANT(name, value) \
        constexpr float name##_f = value##f; \
        constexpr double name##_d = value; \
        constexpr long double name##_ld = value##l; \
        template <typename T> constexpr T name##_c = T(name##_ld);

    // Mathematical constants

    RS_DEFINE_CONSTANT(e,           2.71828'18284'59045'23536'02874'71352'66249'77572'47093'69996);
    RS_DEFINE_CONSTANT(ln2,         0.69314'71805'59945'30941'72321'21458'17656'80755'00134'36026);
    RS_DEFINE_CONSTANT(ln10,        2.30258'50929'94045'68401'79914'54684'36420'76011'01488'62877);
    RS_DEFINE_CONSTANT(log2e,       1.44269'50408'88963'40735'99246'81001'89213'74266'45954'15299);
    RS_DEFINE_CONSTANT(log10e,      0.43429'44819'03251'82765'11289'18916'60508'22943'97005'80367);
    RS_DEFINE_CONSTANT(pi,          3.14159'26535'89793'23846'26433'83279'50288'41971'69399'37511);
    RS_DEFINE_CONSTANT(pi_2,        1.57079'63267'94896'61923'13216'91639'75144'20985'84699'68755);
    RS_DEFINE_CONSTANT(pi_4,        0.78539'81633'97448'30961'56608'45819'87572'10492'92349'84378);
    RS_DEFINE_CONSTANT(one_pi,      0.31830'98861'83790'67153'77675'26745'02872'40689'19291'48091);
    RS_DEFINE_CONSTANT(two_pi,      0.63661'97723'67581'34307'55350'53490'05744'81378'38582'96183);
    RS_DEFINE_CONSTANT(two_sqrtpi,  1.12837'91670'95512'57389'61589'03121'54517'16881'01258'65800);
    RS_DEFINE_CONSTANT(sqrtpi,      1.77245'38509'05516'02729'81674'83341'14518'27975'49456'12239);
    RS_DEFINE_CONSTANT(sqrt2pi,     2.50662'82746'31000'50241'57652'84811'04525'30069'86740'60994);
    RS_DEFINE_CONSTANT(sqrt2,       1.41421'35623'73095'04880'16887'24209'69807'85696'71875'37695);
    RS_DEFINE_CONSTANT(sqrt3,       1.73205'08075'68877'29352'74463'41505'87236'69428'05253'81038);
    RS_DEFINE_CONSTANT(sqrt5,       2.23606'79774'99789'69640'91736'68731'27623'54406'18359'61153);
    RS_DEFINE_CONSTANT(one_sqrt2,   0.70710'67811'86547'52440'08443'62104'84903'92848'35937'68847);

    // Conversion factors

    RS_DEFINE_CONSTANT(inch,           0.0254);              // m
    RS_DEFINE_CONSTANT(foot,           0.3048);              // m
    RS_DEFINE_CONSTANT(yard,           0.9144);              // m
    RS_DEFINE_CONSTANT(mile,           1609.344);            // m
    RS_DEFINE_CONSTANT(nautical_mile,  1852.0);              // m
    RS_DEFINE_CONSTANT(ounce,          0.028349523125);      // kg
    RS_DEFINE_CONSTANT(pound,          0.45359237);          // kg
    RS_DEFINE_CONSTANT(short_ton,      907.18474);           // kg
    RS_DEFINE_CONSTANT(long_ton,       1016.0469088);        // kg
    RS_DEFINE_CONSTANT(pound_force,    4.4482216152605);     // N
    RS_DEFINE_CONSTANT(erg,            1.0e-7);              // J
    RS_DEFINE_CONSTANT(foot_pound,     1.3558179483314004);  // J
    RS_DEFINE_CONSTANT(calorie,        4.184);               // J
    RS_DEFINE_CONSTANT(kilocalorie,    4184.0);              // J
    RS_DEFINE_CONSTANT(ton_tnt,        4.184e9);             // J
    RS_DEFINE_CONSTANT(horsepower,     745.69987158227022);  // W
    RS_DEFINE_CONSTANT(mmHg,           133.322387415);       // Pa
    RS_DEFINE_CONSTANT(atmosphere,     101325.0);            // Pa
    RS_DEFINE_CONSTANT(zero_celsius,   273.15);              // K

    // Physical constants

    RS_DEFINE_CONSTANT(atomic_mass_unit,           1.660538921e-27);  // kg
    RS_DEFINE_CONSTANT(avogadro_constant,          6.02214129e23);    // mol^-1
    RS_DEFINE_CONSTANT(boltzmann_constant,         1.3806488e-23);    // J K^-1
    RS_DEFINE_CONSTANT(elementary_charge,          1.602176565e-19);  // C
    RS_DEFINE_CONSTANT(gas_constant,               8.3144621);        // J mol^-1 K^-1
    RS_DEFINE_CONSTANT(gravitational_constant,     6.67384e-11);      // m^3 kg^-1 s^-2
    RS_DEFINE_CONSTANT(planck_constant,            6.62606957e-34);   // J s
    RS_DEFINE_CONSTANT(speed_of_light,             299792458.0);      // m s^-1
    RS_DEFINE_CONSTANT(stefan_boltzmann_constant,  5.670373e-8);      // W m^-2 K^-4

    // Astronomical constants

    RS_DEFINE_CONSTANT(earth_mass,         5.97219e24);          // kg
    RS_DEFINE_CONSTANT(earth_radius,       6.3710e6);            // m
    RS_DEFINE_CONSTANT(earth_gravity,      9.80665);             // m s^-2
    RS_DEFINE_CONSTANT(jupiter_mass,       1.8986e27);           // kg
    RS_DEFINE_CONSTANT(jupiter_radius,     6.9911e7);            // m
    RS_DEFINE_CONSTANT(solar_mass,         1.98855e30);          // kg
    RS_DEFINE_CONSTANT(solar_radius,       6.96342e8);           // m
    RS_DEFINE_CONSTANT(solar_luminosity,   3.846e26);            // W
    RS_DEFINE_CONSTANT(solar_temperature,  5778.0);              // K
    RS_DEFINE_CONSTANT(astronomical_unit,  1.49597870700e11);    // m
    RS_DEFINE_CONSTANT(light_year,         9.4607304725808e15);  // m
    RS_DEFINE_CONSTANT(parsec,             3.08567758149e16);    // m
    RS_DEFINE_CONSTANT(julian_day,         86'400.0);            // s
    RS_DEFINE_CONSTANT(julian_year,        31'557'600.0);        // s
    RS_DEFINE_CONSTANT(sidereal_year,      31'558'149.7635);     // s
    RS_DEFINE_CONSTANT(tropical_year,      31'556'925.19);       // s

    // Arithmetic literals

    namespace Literals {

        constexpr float operator""_degf(long double x) noexcept { return float(x * (pi_ld / 180.0L)); }
        constexpr float operator""_degf(unsigned long long x) noexcept { return float(static_cast<long double>(x) * (pi_ld / 180.0L)); }
        constexpr double operator""_deg(long double x) noexcept { return double(x * (pi_ld / 180.0L)); }
        constexpr double operator""_deg(unsigned long long x) noexcept { return double(static_cast<long double>(x) * (pi_ld / 180.0L)); }
        constexpr long double operator""_degl(long double x) noexcept { return x * (pi_ld / 180.0L); }
        constexpr long double operator""_degl(unsigned long long x) noexcept { return static_cast<long double>(x) * (pi_ld / 180.0L); }

    }

    // Floating point arithmetic functions

    namespace RS_Detail {

        template <typename T2, typename T1, char Mode, bool FromFloat = std::is_floating_point<T1>::value>
        struct Round;

        template <typename T2, typename T1, char Mode>
        struct Round<T2, T1, Mode, true> {
            T2 operator()(T1 value) const noexcept {
                using std::ceil;
                using std::floor;
                T1 t = Mode == '<' ? floor(value) : Mode == '>' ? ceil(value) : floor(value + T1(1) / T1(2));
                return T2(t);
            }
        };

        template <typename T2, typename T1, char Mode>
        struct Round<T2, T1, Mode, false> {
            T2 operator()(T1 value) const noexcept {
                return T2(value);
            }
        };

    }

    template <typename T> constexpr T degrees(T rad) noexcept { return rad * (T(180) / pi_c<T>); }
    template <typename T> constexpr T radians(T deg) noexcept { return deg * (pi_c<T> / T(180)); }
    template <typename T1, typename T2> constexpr T2 interpolate(T1 x1, T2 y1, T1 x2, T2 y2, T1 x) noexcept
        { return x1 == x2 ? (y1 + y2) * (T1(1) / T1(2)) : y1 == y2 ? y1 : y1 + (y2 - y1) * ((x - x1) / (x2 - x1)); }
    template <typename T2, typename T1> T2 iceil(T1 value) noexcept { return RS_Detail::Round<T2, T1, '>'>()(value); }
    template <typename T2, typename T1> T2 ifloor(T1 value) noexcept { return RS_Detail::Round<T2, T1, '<'>()(value); }
    template <typename T2, typename T1> T2 iround(T1 value) noexcept { return RS_Detail::Round<T2, T1, '='>()(value); }

}
