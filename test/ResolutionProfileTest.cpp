#include <cmath>
#include <iostream>
#include <stdexcept>

#include "ResolutionProfile.h"


bool nearlyEqual(
    float a,
    float b,
    float epsilon = 0.0001f
) {
    return std::fabs(a - b) < epsilon;
}


int main() {

    std::cout
        << "====================================\n";

    std::cout
        << "ResolutionProfile tests\n";

    std::cout
        << "====================================\n";


    /*
     * --------------------------------------------------------
     * Default profile
     * --------------------------------------------------------
     */

    ResolutionProfile profile;


    /*
     * Near band
     *
     * 0 <= distance < 10
     */
    if (profile.getBand(0.0f) != 0) {
        std::cerr
            << "FAIL: 0m should be near band\n";
        return 1;
    }

    if (profile.getBand(9.999f) != 0) {
        std::cerr
            << "FAIL: 9.999m should be near band\n";
        return 1;
    }

    if (!nearlyEqual(
            profile.getResolution(5.0f),
            0.05f)) {

        std::cerr
            << "FAIL: near resolution\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Near -> Mid boundary
     * --------------------------------------------------------
     */

    if (profile.getBand(10.0f) != 1) {
        std::cerr
            << "FAIL: 10m should be mid band\n";
        return 1;
    }

    if (profile.getBand(10.001f) != 1) {
        std::cerr
            << "FAIL: 10.001m should be mid band\n";
        return 1;
    }

    if (!nearlyEqual(
            profile.getResolution(10.0f),
            0.10f)) {

        std::cerr
            << "FAIL: mid resolution\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Mid band
     * --------------------------------------------------------
     */

    if (profile.getBand(20.0f) != 1) {
        std::cerr
            << "FAIL: 20m should be mid band\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Mid -> Far boundary
     * --------------------------------------------------------
     */

    if (profile.getBand(25.0f) != 2) {
        std::cerr
            << "FAIL: 25m should be far band\n";
        return 1;
    }

    if (profile.getBand(25.001f) != 2) {
        std::cerr
            << "FAIL: 25.001m should be far band\n";
        return 1;
    }

    if (!nearlyEqual(
            profile.getResolution(25.0f),
            0.25f)) {

        std::cerr
            << "FAIL: far resolution\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Band -> resolution
     * --------------------------------------------------------
     */

    if (!nearlyEqual(
            profile.getResolutionForBand(0),
            0.05f)) {

        std::cerr
            << "FAIL: band 0 resolution\n";
        return 1;
    }

    if (!nearlyEqual(
            profile.getResolutionForBand(1),
            0.10f)) {

        std::cerr
            << "FAIL: band 1 resolution\n";
        return 1;
    }

    if (!nearlyEqual(
            profile.getResolutionForBand(2),
            0.25f)) {

        std::cerr
            << "FAIL: band 2 resolution\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Custom profile
     * --------------------------------------------------------
     */

    ResolutionProfile custom(
        5.0f,
        0.02f,
        15.0f,
        0.08f,
        0.20f
    );

    if (custom.getBand(4.999f) != 0) {
        std::cerr
            << "FAIL: custom near band\n";
        return 1;
    }

    if (custom.getBand(5.0f) != 1) {
        std::cerr
            << "FAIL: custom mid boundary\n";
        return 1;
    }

    if (custom.getBand(15.0f) != 2) {
        std::cerr
            << "FAIL: custom far boundary\n";
        return 1;
    }

    if (!nearlyEqual(
            custom.getResolution(2.0f),
            0.02f)) {

        std::cerr
            << "FAIL: custom near resolution\n";
        return 1;
    }

    if (!nearlyEqual(
            custom.getResolution(10.0f),
            0.08f)) {

        std::cerr
            << "FAIL: custom mid resolution\n";
        return 1;
    }

    if (!nearlyEqual(
            custom.getResolution(20.0f),
            0.20f)) {

        std::cerr
            << "FAIL: custom far resolution\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Invalid distance
     * --------------------------------------------------------
     */

    bool threw = false;

    try {
        profile.getBand(-1.0f);
    }
    catch (const std::invalid_argument &) {
        threw = true;
    }

    if (!threw) {
        std::cerr
            << "FAIL: negative distance\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Invalid band
     * --------------------------------------------------------
     */

    threw = false;

    try {
        profile.getResolutionForBand(3);
    }
    catch (const std::out_of_range &) {
        threw = true;
    }

    if (!threw) {
        std::cerr
            << "FAIL: invalid band\n";
        return 1;
    }


    /*
     * --------------------------------------------------------
     * Result
     * --------------------------------------------------------
     */

    std::cout
        << "All ResolutionProfile tests passed!\n";

    std::cout
        << "====================================\n";

    return 0;
}