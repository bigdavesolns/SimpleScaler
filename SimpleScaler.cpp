#include <iostream>
#include <string>
#include "ImageScaler.h"

static bool parseWxH(const std::string& s, int& w, int& h)
{
    auto pos = s.find('x');
    if (pos == std::string::npos)
        return false;

    try {
        w = std::stoi(s.substr(0, pos));
        h = std::stoi(s.substr(pos + 1));
    }
    catch (...) {
        return false;
    }

    //return (w > 0 && h > 0);
    return true;
}

static void usage(const char* prog)
{
    std::cout
        << "Usage:\n"
        << "  " << prog << " --relative <in> <out> <sx> [sy] [--dry-run]\n"
        << "  " << prog << " --absolute <in> <out> <w> <h> | <WxH> [--dry-run]\n"
        << "  " << prog << " --self-test\n";
}

static int failMessage(const std::string& msg) {
    std::cerr << msg << "\n";
    return 1;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }
    ImageScaler::Mode mode;
    std::string modeflag(argv[1]);
    std::string infilepath, outfilepath;
    double sx{}, sy{};
    int w{}, h{};
    bool dryRun = false;
    if (modeflag == "--self-test")
        return ImageScaler::runSelfTests("tests") ? 0 : 1;
    else if (modeflag == "--relative")
        mode = ImageScaler::Mode::Relative;
    else if (modeflag == "--absolute")
        mode = ImageScaler::Mode::Absolute;
    else
        return failMessage("Invalid mode flag, see usage.");
    if (argc < 5) 
        return failMessage("Malformed command, see usage.");

    infilepath = argv[2];
    outfilepath = argv[3];

    //if (argc > 1 && std::string(argv[argc - 1]) == "--dry-run") {
    //    dryRun = true;
    //    --argc;
    //}


    if (mode == ImageScaler::Mode::Absolute) {
        if (parseWxH(argv[4], w, h)) {
            if (w < 1 || h < 1)
                return failMessage("Absolute width and height must be >=1.");
            sx = static_cast<double>(w);
            sy = static_cast<double>(h);
        }
        else if (argc > 5 && argv[5][0] != '-') {
            try {
                sx = std::stod(argv[4]);
                sy = std::stod(argv[5]);
            }
            catch (...) {
                return failMessage("Invalid scale value.");
            }
        }
        else
            return failMessage("Absolute mode flag needs both width and height specified.");
    }
    else if (mode == ImageScaler::Mode::Relative) {
        try {
            sx = std::stod(argv[4]);
            if (argc > 5 && argv[5][0] != '-')
                sy = std::stod(argv[5]);
            else sy = sx;
        }
        catch (...) {
            return failMessage("Invalid scale value.");
        }
    }
    if (std::string(argv[argc - 1]) == "--dry-run") {
        dryRun = true;
    }

    if (!ImageScaler::scaleImage(mode, infilepath, outfilepath, sx, sy, dryRun))
        return failMessage("Scaling operation failed.");
    if (!dryRun)
        std::cout << "Success: Scaled image written to: " << outfilepath << "\n";
    else
        std::cout
        << "Mode: " << ImageScaler::printModeString(mode) << "\n"
        << "Input: " << infilepath << "\n"
        << "Output: " << outfilepath << "\n"
        << "Params: " << sx << ", " << sy << "\n";

    return 0;
}
