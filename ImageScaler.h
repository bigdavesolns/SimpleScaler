#pragma once

#include <string>
#include <cstdint>

class ImageScaler
{
public:
    // Opaque image handle (internally a cv::Mat*)
    using Image = void*;

	enum class Mode : uint8_t
    {
        Relative = 0,
        Absolute = 1
    };
    static std::string printModeString(const Mode& mode) {
        if (mode == Mode::Absolute) return std::string("Absolute");
        if (mode == Mode::Relative) return std::string("Relative");
        return std::string("Invalid");
    }

    //THE single entry point for scaling
    static bool scaleImage(
        Mode mode,
        const std::string& inputPath,
        const std::string& outputPath,
        double a,
        double b,
        bool dryRun);

    // Runs internal self-tests. Returns true on success.
    static bool runSelfTests(const std::string& outputDir);

private:
    static bool scaleRelative(
        const std::string& inputPath,
        const std::string& outputPath,
        double scaleX,
        double scaleY,
        bool dryRun);

    static bool scaleAbsolute(
        const std::string& inputPath,
        const std::string& outputPath,
        int width,
        int height,
        bool dryRun);

    // Image helpers
    static bool loadImage(const std::string& path, Image& image);
    static void releaseImage(Image& image);
    static bool writeImage(const std::string& path, Image image);

    static bool resizeRelative(
        Image input,
        Image& output,
        double sx,
        double sy,
        bool dryRun);

    static bool resizeAbsolute(
        Image input,
        Image& output,
        int width,
        int height,
        bool dryRun);

    static int chooseInterpolation(
        int srcW,
        int srcH,
        int dstW,
        int dstH);

	// Test helpers
    static bool generateTestImages(const std::string& outputDir);
    static bool verifyImageSize(const std::string& path, int expectedWidth, int expectedHeight);
};
