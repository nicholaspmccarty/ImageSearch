// Nicholas McCarty
// Copyright 2024 @ nicholasmccarty252@gmail.com

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include "PNG.h"

// It is ok to use the following namespace declarations in C++ source
// files only. They must never be used in header files.
using namespace std;
using namespace std::string_literals;

// Declaration for computeBackgroundPixel. Ensures visibility when accessed in 
// the main method.
Pixel computeBackgroundPixel(const PNG& img1, const PNG& mask, const int 
    startRow, const int startCol, const int maxRow, const int maxCol);

bool isOverlapping(const vector<pair<int, int>>& regions, int row, int col, 
    const PNG& maskImg);
void hitOrMiss(const Pixel& maskPixel, const Pixel& Black, const Pixel& 
    White, bool isSameShade, size_t& hit, size_t& miss);
int processRegion(const PNG& largeImg, const PNG& maskImg, int row, 
    int col, const Pixel& bgColor, int tolerance);
void drawBox(PNG& png, int row, int col, int width, int height);

/**
 * This is the top-level method that is called from the main method to 
 * perform the necessary image search operation. 
 * 
 * \param[in] mainImageFile The PNG image in which the specified 
 * searchImage 
 * is to be found and marked (for example, this will be "Flag_of_the_US.png")
 * 
 * \param[in] srchImageFile The PNG sub-image for which we will be 
 * searching
 * in the main image (for example, this will be "star.png" or 
 * "start_mask.png") 
 * 
 * \param[in] outImageFile The output file to which the 
 * mainImageFile file is 
 * written with search image file highlighted.
 * 
 * \param[in] isMask If this flag is true then the searchImageFile 
 * should 
 * be deemed as a "mask". The default value is true.
 * 
 * \param[in] matchPercent The percentage of pixels in the mainImage and
 * searchImage that must match in order for a region in the mainImage to be
 * deemed a match.
 * 
 * \param[in] tolerance The absolute acceptable difference between 
 * each color
 * channel when comparing  
 */
void imageSearch(const std::string& mainImageFile,
                 const std::string& srchImageFile, 
                 const std::string& outImageFile, 
                 const bool isMask = true, 
                 const int matchPercent = 75, 
                 const int tole = 32) {
    PNG largeImg, maskImg;
    largeImg.load(mainImageFile);  
    maskImg.load(srchImageFile);
    vector<pair<int, int>> matchedRegions;

for (int row = 0; row <= largeImg.getHeight() - maskImg.getHeight(); ++row) {
    for (int col = 0; col <= largeImg.getWidth() - maskImg.getWidth(); ++col) {
    Pixel bgColor = computeBackgroundPixel(largeImg, maskImg, row, col, 
    maskImg.getHeight(), maskImg.getWidth());
    int netMatch = processRegion(largeImg, maskImg, row, col, bgColor, tole);
    if (netMatch > maskImg.getWidth() * maskImg.getHeight() * 
    matchPercent / 100 &&!isOverlapping(matchedRegions, row, col, maskImg)) {
    std::cout << "sub-image matched at: " << row << ", " << col << ", "<< row + 
    maskImg.getHeight() << ", " << col + maskImg.getWidth() << std::endl;
                drawBox(largeImg, row, col, maskImg.getWidth(), 
                     maskImg.getHeight());
                     matchedRegions.push_back({row, col});
            }
        }
    }
    largeImg.write(outImageFile);
    std::cout << "Number of matches: " << matchedRegions.size() << std::endl;}

/**
 * Processes a region of the image, compares pixel values, and calculates the net match score.
 * 
 * \param[in] largeImg The main image where the sub-image is being searched for.
 * \param[in] maskImg The sub-image or mask being searched for.
 * \param[in] row The starting row of the region.
 * \param[in] col The starting column of the region.
 * \param[in] bgColor The computed background pixel color.
 * \param[in] tolerance The tolerance for pixel comparison.
 * 
 * \returns The difference between hit and miss counts in the region.
 */
int processRegion(const PNG& largeImg, const PNG& maskImg, int row, int col,
                  const Pixel& bgColor, int tolerance) {
    size_t hit = 0, miss = 0;
    const Pixel Black{ .rgba = 0xff'00'00'00U };
    const Pixel White{ .rgba = 0xff'ff'ff'ffU };

    for (int maskRow = 0; maskRow < maskImg.getHeight(); ++maskRow) {
        for (int maskCol = 0; maskCol < maskImg.getWidth(); ++maskCol) {
            const auto imgPixel = largeImg.getPixel(row + maskRow, 
                                                    col + maskCol);
            const auto maskPixel = maskImg.getPixel(maskRow, maskCol);
            
            bool isSameShade = (std::abs(imgPixel.color.red - bgColor.color.red)
                < tolerance) &&
                (std::abs(imgPixel.color.green - bgColor.color.green)
                < tolerance) &&
                (std::abs(imgPixel.color.blue - bgColor.color.blue)
                < tolerance);
            
            hitOrMiss(maskPixel, Black, White, isSameShade, hit, miss);
        }
    }

    return hit - miss;
}

/**
 * Checks if a given pixel is close to white within a certain tolerance.
 * 
 * \param[in] pixel The pixel to be checked.
 * \param[in] White The reference white pixel.
 * \param[in] tolerance The allowed tolerance for RGB channel differences.
 * 
 * \returns True if the pixel is close to white, false otherwise.
 */
bool isCloseToWhite(const Pixel& pixel, const Pixel& White, int tolerance) {
    return (abs(pixel.color.red - White.color.red) <= tolerance &&
            abs(pixel.color.green - White.color.green) <= tolerance &&
            abs(pixel.color.blue - White.color.blue) <= tolerance);
}

/**
 * Determines if a pixel results in a "hit" or "miss" when compared to the background pixel.
 * 
 * \param[in] maskPixel The mask pixel to compare.
 * \param[in] Black The reference black pixel.
 * \param[in] White The reference white pixel.
 * \param[in] isSameShade Whether the pixel has the same shade as the background.
 * \param[out] hit The number of hits incremented.
 * \param[out] miss The number of misses incremented.
 */
void hitOrMiss(const Pixel& maskPixel, const Pixel& Black, const Pixel& White, 
               bool isSameShade, size_t& hit, size_t& miss) {
    if (maskPixel.rgba == Black.rgba) {
        isSameShade ? hit++ : miss++;
    } else {
        isSameShade ? miss++ : hit++;
    }
}

/**
 * Draws a red box around the matched region in the main image.
 * 
 * \param[out] png The main PNG image to be modified.
 * \param[in] row The starting row of the box.
 * \param[in] col The starting column of the box.
 * \param[in] width The width of the box.
 * \param[in] height The height of the box.
 */
void drawBox(PNG& png, int row, int col, int width, int height) { 
    for (int i = 0; i < width; i++) {
        png.setRed(row, col + i);
        png.setRed(row + height, col + i);
    }
    for (int i = 0; i < height; i++) { 
        png.setRed(row + i, col);
        png.setRed(row + i, col + width);
    }
}

/**
 * Checks if a region overlaps with any previously matched regions.
 * 
 * \param[in] regions The vector of previously matched regions.
 * \param[in] row The current row of the region.
 * \param[in] col The current column of the region.
 * \param[in] maskImg The sub-image mask used for matching.
 * 
 * \returns True if the region overlaps, false otherwise.
 */
bool isOverlapping(const vector<pair<int, int>>& regions, int row, int col, 
                   const PNG& maskImg) {
    for (const auto& region : regions) {
        if (abs(region.first - row) < maskImg.getHeight() && 
            abs(region.second - col) < maskImg.getWidth()) {
            return true;
        }
    }
    return false;
}

/**
 * Main function to check command-line arguments and invoke image search.
 * 
 * \param[in] argc The number of command-line arguments.
 * \param[in] argv The command-line arguments.
 * 
 * \returns 0 if the process was successful, 1 otherwise.
 */
int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <MainPNGfile> <SearchPNGfile> "
                  << "<OutputPNGfile> [isMaskFlag] [match-percentage] "
                  << "[tolerance]\n";
        return 1;
    }
    
    const std::string True("true");
    imageSearch(argv[1], argv[2], argv[3],       // The 3 required PNG files
                (argc > 4 ? (True == argv[4]) : true),   // Optional mask flag
            (argc > 5 ? std::stoi(argv[5]) : 75),    // Optional percentMatch
                (argc > 6 ? std::stoi(argv[6]) : 32));   // Optional tolerance

    return 0;
}

/**
 * Computes the average background pixel of a specified region in the large image.
 * 
 * \param[in] img1 The larger image where the background pixel is computed.
 * \param[in] mask The mask image.
 * \param[in] startRow The starting row of the region in the image.
 * \param[in] startCol The starting column of the region in the image.
 * \param[in] maxRow The number of rows to process.
 * \param[in] maxCol The number of columns to process.
 * 
 * \returns The average background pixel color.
 */
Pixel computeBackgroundPixel(const PNG& img1, const PNG& mask, 
    const int startRow, const int startCol, 
        const int maxRow, const int maxCol) {
    const Pixel Black{ .rgba = 0xff'00'00'00U };
    int red = 0, blue = 0, green = 0, count = 0;

    for (int row = 0; row < maxRow; row++) {
        for (int col = 0; col < maxCol; col++) {
            if (mask.getPixel(row, col).rgba == Black.rgba) { 
                const auto pix = img1.getPixel(row + startRow, col + startCol); 
                red += pix.color.red;
                green += pix.color.green;
                blue += pix.color.blue;
                count++;
            }
        }
    }

    unsigned char avgRed = 0, avgGreen = 0, avgBlue = 0;
    if (count > 0) {
        avgRed = red / count;
        avgGreen = green / count;
        avgBlue = blue / count;
    }
    return { .color = {avgRed, avgGreen, avgBlue, 0} };
}
