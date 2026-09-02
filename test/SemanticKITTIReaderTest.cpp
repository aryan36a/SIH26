#include "SemanticKITTI.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace
{

int testsPassed = 0;
int testsFailed = 0;


void check(
    bool condition,
    const std::string& testName)
{
    if (condition)
    {
        std::cout
            << "[PASS] "
            << testName
            << '\n';

        ++testsPassed;
    }
    else
    {
        std::cerr
            << "[FAIL] "
            << testName
            << '\n';

        ++testsFailed;
    }
}


void writeTestBin(
    const std::string& filename)
{
    std::ofstream file(
        filename,
        std::ios::binary
    );

    const Point points[] =
    {
        {1.0f,  2.0f,  3.0f, 10.0f},
        {4.0f,  5.0f,  6.0f, 20.0f},
        {-1.0f, 0.5f, 7.0f, 30.0f}
    };

    file.write(
        reinterpret_cast<const char*>(points),
        sizeof(points)
    );
}


void writeTestLabels(
    const std::string& filename)
{
    std::ofstream file(
        filename,
        std::ios::binary
    );

    /*
     * semantic ID:
     *     10, 20, 30
     *
     * instance ID:
     *     1, 2, 3
     *
     * packed:
     *
     *     instance << 16 | semantic
     */

    const std::uint32_t labels[] =
    {
        (1u << 16) | 10u,
        (2u << 16) | 20u,
        (3u << 16) | 30u
    };

    file.write(
        reinterpret_cast<const char*>(labels),
        sizeof(labels)
    );
}


void testValidFrame()
{
    const std::string binFile =
        "phase2_test.bin";

    const std::string labelFile =
        "phase2_test.label";

    writeTestBin(binFile);
    writeTestLabels(labelFile);

    SemanticKITTIFrame frame;

    check(
        frame.load(binFile, labelFile),
        "valid SemanticKITTI frame loads"
    );

    check(
        frame.size() == 3,
        "point count is three"
    );

    check(
        frame.getPackedLabel(0) ==
            ((1u << 16) | 10u),
        "raw packed label is preserved"
    );

    check(
        frame.getSemanticId(0) == 10,
        "semantic ID is extracted"
    );

    check(
        frame.getInstanceId(0) == 1,
        "instance ID is extracted"
    );

    check(
        frame.getSemanticId(1) == 20 &&
        frame.getInstanceId(1) == 2,
        "second point label correspondence is preserved"
    );

    check(
        frame.getPoint(2).x == -1.0f &&
        frame.getPoint(2).z == 7.0f,
        "point data remains aligned with labels"
    );

    frame.clear();

    check(
        frame.empty(),
        "clear removes frame data"
    );

    std::remove(binFile.c_str());
    std::remove(labelFile.c_str());
}


void testMissingLabelFile()
{
    const std::string binFile =
        "phase2_missing_label_test.bin";

    writeTestBin(binFile);

    SemanticKITTIFrame frame;

    check(
        !frame.load(
            binFile,
            "phase2_missing.label"
        ),
        "missing label file is rejected"
    );

    check(
        frame.empty(),
        "failed load leaves frame empty"
    );

    std::remove(binFile.c_str());
}


void testMismatchedCounts()
{
    const std::string binFile =
        "phase2_mismatch_test.bin";

    const std::string labelFile =
        "phase2_mismatch_test.label";

    writeTestBin(binFile);

    std::ofstream file(
        labelFile,
        std::ios::binary
    );

    const std::uint32_t labels[] =
    {
        1u,
        2u
    };

    file.write(
        reinterpret_cast<const char*>(labels),
        sizeof(labels)
    );

    file.close();

    SemanticKITTIFrame frame;

    check(
        !frame.load(binFile, labelFile),
        "point/label count mismatch is rejected"
    );

    check(
        frame.empty(),
        "mismatched frame is not retained"
    );

    std::remove(binFile.c_str());
    std::remove(labelFile.c_str());
}


void testInvalidLabelSize()
{
    const std::string binFile =
        "phase2_invalid_label_size.bin";

    const std::string labelFile =
        "phase2_invalid_label_size.label";

    writeTestBin(binFile);

    std::ofstream file(
        labelFile,
        std::ios::binary
    );

    const unsigned char invalidBytes[] =
    {
        0x01,
        0x02,
        0x03
    };

    file.write(
        reinterpret_cast<const char*>(invalidBytes),
        sizeof(invalidBytes)
    );

    file.close();

    SemanticKITTIFrame frame;

    check(
        !frame.load(binFile, labelFile),
        "invalid label byte size is rejected"
    );

    std::remove(binFile.c_str());
    std::remove(labelFile.c_str());
}

}


int main()
{
    std::cout
        << "========================================\n"
        << "SIH 26 SemanticKITTI Reader Tests\n"
        << "Phase 2\n"
        << "========================================\n";

    testValidFrame();
    testMissingLabelFile();
    testMismatchedCounts();
    testInvalidLabelSize();

    std::cout
        << '\n'
        << "========================================\n"
        << "Tests passed: "
        << testsPassed
        << '\n'
        << "Tests failed: "
        << testsFailed
        << '\n'
        << "========================================\n";

    return testsFailed == 0
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}
