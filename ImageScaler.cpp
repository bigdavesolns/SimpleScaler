#include "ImageScaler.h"

#include <iostream>
#include <random>
#include <filesystem>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

bool ImageScaler::scaleImage(
    Mode mode,
    const std::string& inputPath,
    const std::string& outputPath,
    double a,
    double b,
    bool dryRun)
{
    switch (mode)
    {
    case Mode::Relative:
        return scaleRelative(inputPath, outputPath, a, b, dryRun);
    case Mode::Absolute:
        return scaleAbsolute(inputPath, outputPath, static_cast<int>(a), static_cast<int>(b), dryRun);
    default:
		return false;
    }
}

bool ImageScaler::loadImage(const std::string& path, Image& image)
{
    cv::Mat* mat = new cv::Mat(cv::imread(path));
    if (mat->empty())
    {
        delete mat;
        image = nullptr;
        return false;
    }

    image = mat;
    return true;
}

void ImageScaler::releaseImage(Image& image)
{
    delete static_cast<cv::Mat*>(image);
    image = nullptr;
}

int ImageScaler::chooseInterpolation(
    int srcW, int srcH,
    int dstW, int dstH)
{
    return (dstW < srcW || dstH < srcH)
        ? cv::INTER_AREA
        : cv::INTER_CUBIC;
}

bool ImageScaler::resizeRelative(
    Image input,
    Image& output,
    double sx,
    double sy,
    bool dryRun)
{
    if (sx <= 0.0 || sy <= 0.0)
        return false;

    cv::Mat* in = static_cast<cv::Mat*>(input);

    int dstW = static_cast<int>(in->cols * sx);
    int dstH = static_cast<int>(in->rows * sy);

    int interp = chooseInterpolation(
        in->cols, in->rows,
        dstW, dstH);

    if (dryRun)
    {
        std::cout
            << "Dry run (relative):\n"
            << "  Scale: " << sx << ", " << sy << "\n"
            << "  Estimated size: " << dstW << " x " << dstH << "\n"
            << "  Interpolation: "
            << ((interp == cv::INTER_AREA) ? "INTER_AREA" : "INTER_CUBIC")
            << "\n";
        return true;
    }

    cv::Mat* out = new cv::Mat();
    cv::resize(*in, *out, cv::Size(), sx, sy, interp);
    output = out;
    return true;
}

bool ImageScaler::resizeAbsolute(
    Image input,
    Image& output,
    int width,
    int height,
    bool dryRun)
{
    if (width <= 0 || height <= 0)
        return false;

    cv::Mat* in = static_cast<cv::Mat*>(input);

    int interp = chooseInterpolation(
        in->cols, in->rows,
        width, height);

    if (dryRun)
    {
        std::cout
            << "Dry run (absolute):\n"
            << "  Target size: " << width << " x " << height << "\n"
            << "  Interpolation: "
            << ((interp == cv::INTER_AREA) ? "INTER_AREA" : "INTER_CUBIC")
            << "\n";
        return true;
    }

    cv::Mat* out = new cv::Mat();
    cv::resize(*in, *out, cv::Size(width, height), 0.0, 0.0, interp);
    output = out;
    return true;
}

bool ImageScaler::writeImage(const std::string& path, Image image)
{
    return cv::imwrite(path, *static_cast<cv::Mat*>(image));
}

bool ImageScaler::scaleRelative(
    const std::string& inputPath,
    const std::string& outputPath,
    double scaleX,
    double scaleY,
    bool dryRun)
{
    Image input = nullptr;
    Image output = nullptr;

    if (!loadImage(inputPath, input))
        return false;

    bool ok = resizeRelative(input, output, scaleX, scaleY, dryRun);

    if (ok && !dryRun)
        ok = writeImage(outputPath, output);

    releaseImage(input);
    releaseImage(output);

    return ok;
}

bool ImageScaler::scaleAbsolute(
    const std::string& inputPath,
    const std::string& outputPath,
    int width,
    int height,
    bool dryRun)
{
    Image input = nullptr;
    Image output = nullptr;

    if (!loadImage(inputPath, input))
        return false;

    bool ok = resizeAbsolute(input, output, width, height, dryRun);
    if (ok && !dryRun)
        ok = writeImage(outputPath, output);

    releaseImage(input);
    releaseImage(output);

    return ok;
}

namespace fs = std::filesystem;
bool ImageScaler::generateTestImages(const std::string& outputDir)
{
	fs::create_directories(outputDir);

    // test1.png: 50x50 solid color
    {
        cv::Mat img(50, 50, CV_8UC3, cv::Scalar(150, 100, 50));
        if (!cv::imwrite(outputDir + "/test1.png", img))
            return false;
	}

    // test2.png: 37x91 random colors
    {
        cv::Mat img(91, 37, CV_8UC3);
        std::mt19937 rng(12345);
        std::uniform_int_distribution<int> dist(0, 255);
        for (int y = 0; y < img.rows; ++y)
        {
            for (int x = 0; x < img.cols; ++x)
            {
                img.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    static_cast<uchar>(dist(rng)),
                    static_cast<uchar>(dist(rng)),
                    static_cast<uchar>(dist(rng)));
            }
        }
        if (!cv::imwrite(outputDir + "/test2.png", img))
            return false;

        // test3.png: overlapping rectangles
        {
            cv::Mat img(100, 150, CV_8UC3, cv::Scalar(0, 0, 0));

            cv::rectangle(img, { 10,10 }, { 80,70 }, { 0,255,0 }, -1);
            cv::rectangle(img, { 40,30 }, { 120,90 }, { 255,0,0 }, -1);
            cv::rectangle(img, { 60,5 }, { 140,40 }, { 0,0,255 }, -1);

            if (!cv::imwrite(outputDir + "/test3.png", img))
                return false;
        }
        return true;
    }
}

bool ImageScaler::verifyImageSize(const std::string& path, int expectedWidth, int expectedHeight)
{
    cv::Mat img = cv::imread(path);
    if (img.empty())
        return false;
    return (img.cols == expectedWidth) && (img.rows == expectedHeight);
}

bool ImageScaler::runSelfTests(const std::string& outputDir)
{
	std::cout << "Running ImageScaler self-tests...\n";
    //std::string testImageDir = outputDir + "/test_images";
    if (!generateTestImages(outputDir))
    {
        std::cerr << "Failed to generate test images.\n";
        return false;
    }
    struct TestCase
    {
        Mode mode;
        std::string inputFilename;
        std::string outputFilename;
        double a;
        double b;
        //int width;
        //int height;
        int expectedWidth;
        int expectedHeight;
    };
    std::vector<TestCase> testCases = {
        { Mode::Relative, "test1.png", "out1_rel.png", 0.5, 2.0, 25, 100 },
        { Mode::Relative, "test2.png", "out2_rel.png", 2.0, 2.0, 74, 182 },
        { Mode::Relative, "test3.png", "out3_rel.png", 0.4, 1.6, 60, 160 },

        { Mode::Absolute, "test1.png", "out1_abs.png", 200, 200, 200, 200 },
        { Mode::Absolute, "test2.png", "out2_abs.png", 75, 50, 75, 50 },
        { Mode::Absolute, "test3.png", "out3_abs.png", 200, 150, 200, 150 },
    };
    for (const auto& test : testCases)
    {
        std::string infilename = outputDir + "/" + test.inputFilename;
        std::string outfilename = outputDir + "/" + test.outputFilename;

        if (!scaleImage(test.mode, infilename, outfilename, test.a, test.b, false))
        {
            std::cerr << "Scaling failed for " << test.inputFilename << "\n";
            return false;
        }

        if (!verifyImageSize(outfilename, test.expectedWidth, test.expectedHeight))
        {
            std::cerr << "Size verification failed for " << test.outputFilename << "\n";
            return false;
        }
    }
    std::cout << "All ImageScaler tests passed.\n";
    return true;
}