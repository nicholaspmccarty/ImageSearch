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
#include <vector>

// It is ok to use the following namespace delarations in C++ source
// files only. They must never be used in header files.
using namespace std;
using namespace std::string_literals;

// Declaration for computeBackgroundPixel. Ensures visibility when accessed in the main method.
Pixel computeBackgroundPixel(const PNG& img1, const PNG& mask, const int startRow, const int startCol, 
                             const int maxRow, const int maxCol);




/**
 * This is the top-level method that is called from the main method to 
 * perform the necessary image search operation. 
 * 
 * \param[in] mainImageFile The PNG image in which the specified searchImage 
 * is to be found and marked (for example, this will be "Flag_of_the_US.png")
 * 
 * \param[in] srchImageFile The PNG sub-image for which we will be searching
 * in the main image (for example, this will be "star.png" or "start_mask.png") 
 * 
 * \param[in] outImageFile The output file to which the mainImageFile file is 
 * written with search image file highlighted.
 * 
 * \param[in] isMask If this flag is true then the searchImageFile should 
 * be deemed as a "mask". The default value is false.
 * 
 * \param[in] matchPercent The percentage of pixels in the mainImage and
 * searchImage that must match in order for a region in the mainImage to be
 * deemed a match.
 * 
 * \param[in] tolerance The absolute acceptable difference between each color
 * channel when comparing  
 */
void imageSearch(const std::string& mainImageFile,
                const std::string& srchImageFile, 
                const std::string& outImageFile, const bool isMask = true, 
                const int matchPercent = 75, const int tolerance = 32) {
    // Implement this method using various methods or even better
    // use an object-oriented approach.
    std::cout << "This is a test message" << std::endl;
    // Create local variables
    PNG largeImg;
    PNG maskImg;
    // Load the images
    largeImg.load(mainImageFile);  
    maskImg.load(srchImageFile);
    vector<pair<int, int>> matchedRegions;
    for (int row = 0; row <= largeImg.getHeight() - maskImg.getHeight(); ++row) {
    for (int col = 0; col <= largeImg.getWidth() - maskImg.getWidth(); ++col) {
        size_t hit = 0;
        size_t miss = 0;
        // Compute the background pixel once for the current subregion.
        Pixel bgColor = computeBackgroundPixel(largeImg, maskImg, row, col, maskImg.getHeight(), maskImg.getWidth());
        for (int maskRow = 0; maskRow < maskImg.getHeight(); ++maskRow) {
            for (int maskCol = 0; maskCol < maskImg.getWidth(); ++maskCol) {
                const Pixel Black{ .rgba = 0xff'00'00'00U };
                const Pixel White{ .rgba = 0xff'ff'ff'ffU };

                // Fetch pixels once
                const auto imgPixel = largeImg.getPixel(row + maskRow, col + maskCol);
                // Pixel comparison: compute once outside of conditional checks
                const bool isSameShade = (std::abs(imgPixel.color.red - bgColor.color.red) < tolerance) &&
                                         (std::abs(imgPixel.color.green - bgColor.color.green) < tolerance) &&
                                         (std::abs(imgPixel.color.blue - bgColor.color.blue) < tolerance);
                const auto maskPixel = maskImg.getPixel(maskRow, maskCol);
                const int maskPixelRGBA = static_cast<int>(maskPixel.rgba);
                if (maskPixelRGBA == static_cast<int>(Black.rgba)) {
                    // Check if the pixel is "same shade"
                    if (isSameShade) {
                        hit++; // Matching pixel
                    } else {
                        miss++; // Mismatching pixel
                    }
                } else if (maskPixelRGBA == static_cast<int>(White.rgba)) {
                    // Check if the pixel is not the "same shade"
                    if (!isSameShade) {
                        hit++; // Correct mismatch
                    } else {
                        miss++; // Incorrect match
                    }
                } else {
                    miss++; // Other cases (not black or white)
                }
            }
        }
        int netMatch = hit - miss;
        if (netMatch > maskImg.getWidth() * maskImg.getHeight() * matchPercent / 100) {
            bool overlap = false;
            // Check for overlap with previously matched regions
            for (const auto& region : matchedRegions) {
                if (abs(region.first - row) < maskImg.getHeight() && abs(region.second - col) < maskImg.getWidth()) {
                    overlap = true;
                    break;
                }
            }
            // Only add non-overlapping matches
            if (!overlap) {
                std::cout << "sub-image matched at: " << row << ", " << col << ", " << row + maskImg.getHeight() << ", " << col + maskImg.getWidth() << std::endl;
                matchedRegions.push_back({row, col});
            } 
        }
    }
}
std::cout << "Number of matches: " << matchedRegions.size() << std::endl;
}






    


/**
 * The main method simply checks for command-line arguments and then calls
 * the image search method in this file.
 * 
 * \param[in] argc The number of command-line arguments. This program
 * needs at least 3 command-line arguments.
 * 
 * \param[in] argv The actual command-line arguments in the following order:
 *    1. The main PNG file in which we will be searching for sub-images
 *    2. The sub-image or mask PNG file to be searched-for
 *    3. The file to which the resulting PNG image is to be written.
 *    4. Optional: Flag (True/False) to indicate if the sub-image is a mask 
 *       (deault: false)
 *    5. Optional: Number indicating required percentage of pixels to match
 *       (default is 75)
 *    6. Optiona: A tolerance value to be specified (default: 32)
 */
int main(int argc, char *argv[]) {
    if (argc < 4) {
        // Insufficient number of required parameters.
        std::cout << "Usage: " << argv[0] << " <MainPNGfile> <SearchPNGfile> "
                  << "<OutputPNGfile> [isMaskFlag] [match-percentage] "
                  << "[tolerance]\n";
        return 1;
    }
    
    const std::string True("true");
    // Call the method that starts off the image search with the necessary
    // parameters.
    imageSearch(argv[1], argv[2], argv[3],       // The 3 required PNG files
        (argc > 4 ? (True == argv[4]) : true),   // Optional mask flag
        (argc > 5 ? std::stoi(argv[5]) : 75),    // Optional percentMatch
        (argc > 6 ? std::stoi(argv[6]) : 32));   // Optional tolerance

    return 0;
}
Pixel computeBackgroundPixel(const PNG& img1, const PNG& mask, const int startRow, const int startCol, 
    const int maxRow, const int maxCol) {
    const Pixel Black{ .rgba = 0xff'00'00'00U };
    int red = 0, blue = 0, green = 0, count = 0;

    for (int row = 0; (row < maxRow); row++) {
        for (int col = 0; col < maxCol; col++) {
            if (mask.getPixel(row, col).rgba == Black.rgba) { // Check for black pixels in the mask
                const auto pix = img1.getPixel(row + startRow, col + startCol); 
                     // Get corresponding pixel from the larger image
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
// End of source code
