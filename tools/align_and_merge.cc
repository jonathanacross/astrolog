#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>

#include <iostream>

using namespace cv;
using namespace cv::xfeatures2d;

// Aligns im1 to im2, putting result in im1Reg, and the homography in h.
void alignImages(const Mat &im1, const Mat &im2, Mat &im1Reg, Mat &h) {
    // Convert images to grayscale
    Mat im1Gray, im2Gray;
    cvtColor(im1, im1Gray, CV_BGR2GRAY);
    cvtColor(im2, im2Gray, CV_BGR2GRAY);

    Ptr<SIFT> detector = SIFT::create();
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute( im1Gray, noArray(), keypoints1, descriptors1 );
    detector->detectAndCompute( im2Gray, noArray(), keypoints2, descriptors2 );

    // Match features.
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector<std::vector<cv::DMatch>> matches;
    matcher->knnMatch(descriptors1, descriptors2, matches, 2);  // Find two nearest matches
    const float ratio = 0.75; // can be tuned
    std::vector<cv::DMatch> good_matches;
    for (int i = 0; i < matches.size(); ++i)
    {
        if (matches[i][0].distance < ratio * matches[i][1].distance)
        {
            good_matches.push_back(matches[i][0]);
        }
    }

    // Extract location of good matches
    std::vector<Point2f> points1, points2;
    for (size_t i = 0; i < good_matches.size(); i++) {
        points1.push_back(keypoints1[ good_matches[i].queryIdx ].pt);
        points2.push_back(keypoints2[ good_matches[i].trainIdx ].pt);
    }

    // DEBUG
    //Mat img_matches;
    //drawMatches( im1, keypoints1, im2, keypoints2, good_matches, img_matches, Scalar::all(-1),
    //             Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    //imshow("matches", img_matches);
    //waitKey(0);
    // end DEBUG

    // Find homography
    h = findHomography( points1, points2, RANSAC );

    // Use homography to warp image
    warpPerspective(im1, im1Reg, h, im2.size());
}

struct CommandLineOptions {
    enum BlendMode {
        AVERAGE,
        DARKEN,
        LIGHTEN,
        MULTIPLY,
        SCREEN
    };

    // if parsing is successful
    bool is_valid;

    std::string img1;
    std::string img2;
    std::string dst;
    bool grayscale;
    BlendMode blend_mode;
};

CommandLineOptions parseCommandLine(int argc, char **argv) {
    CommandLineOptions options;
    options.is_valid = true;
    options.grayscale = false;
    options.blend_mode = CommandLineOptions::BlendMode::AVERAGE;

    std::vector<std::string> positional_args;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--grayscale") {
            options.grayscale = true;
        } else if (arg == "--average") {
            options.blend_mode = CommandLineOptions::BlendMode::AVERAGE;
        } else if (arg == "--darken") {
            options.blend_mode = CommandLineOptions::BlendMode::DARKEN;
        } else if (arg == "--lighten") {
            options.blend_mode = CommandLineOptions::BlendMode::LIGHTEN;
        } else if (arg == "--multiply") {
            options.blend_mode = CommandLineOptions::BlendMode::MULTIPLY;
        } else if (arg == "--screen") {
            options.blend_mode = CommandLineOptions::BlendMode::SCREEN;
        } else if (arg.rfind("-", 0) == 0) {
            std::cerr << "unknown flag: " << arg << "\n";
            options.is_valid = false;
        } else {
            positional_args.push_back(arg);
        }
    }

    if (positional_args.size() != 3) {
        std::cerr << "expected 3 arguments for img1, img2, and dst.\n";
        options.is_valid = false;
    } else {
        options.img1 = positional_args[0];
        options.img2 = positional_args[1];
        options.dst = positional_args[2];
    }

    if (!options.is_valid) {
        std::cerr << "\nUsage: " << argv[0] << " [--grayscale] [blend_mode] img1 img2 dst\n"
            << "\n"
            << "Reads img1 and img2, registers the first to match the second,\n"
            << "and then linearly averages them, putting the result in dst.\n"
            << "Options:\n"
            << "  --grayscale: converts all images to grayscale\n"
            << "  blend_mode: one of --average --darken --lighten\n";
    }

    return options; 
}

int main(int argc, char **argv) {
    CommandLineOptions options = parseCommandLine(argc, argv);
    if (!options.is_valid) {
        return 1;
    }

    Mat im1 = imread(options.img1, IMREAD_COLOR);
    if(im1.empty()) {
        std::cerr << "Could not read image: " << options.img1 << "\n";
        return 1;
    }

    Mat im2 = imread(options.img2, IMREAD_COLOR);
    if(im2.empty()) {
        std::cerr << "Could not read image: " << options.img2 << "\n";
        return 1;
    }

    Mat im1_aligned;
    Mat homography;
    alignImages(im1, im2, im1_aligned, homography);

    // merge
    Mat src1 = im1_aligned;
    Mat src2 = im2;
    if (options.grayscale) {
        cvtColor(src1, src1, CV_BGR2GRAY);
        cvtColor(src2, src2, CV_BGR2GRAY);
    } 

    Mat dst;
    switch (options.blend_mode) {
        case CommandLineOptions::BlendMode::AVERAGE: {
                double alpha = 0.5;
                addWeighted(src1, alpha, src2, 1-alpha, 0.0, dst);
                break;
            }
        case CommandLineOptions::BlendMode::DARKEN: {
                dst = cv::min(src1, src2);
                break;
            }
        case CommandLineOptions::BlendMode::LIGHTEN: {
                dst = cv::max(src1, src2);
                break;
            }
        case CommandLineOptions::BlendMode::MULTIPLY: {
                dst = cv::max(src1, src2);
                break;
            }
        case CommandLineOptions::BlendMode::SCREEN: {
                dst = cv::max(src1, src2);
                break;
            }
    }

    imwrite(options.dst, dst);

    // UI for visual debugging
    // imshow("Image 1", im1);
    // waitKey(0);
    // imshow("Image 2", im2);
    // waitKey(0);
    // imshow("Image 1 Aligned", im1_aligned);
    // waitKey(0);
    // imshow("Merged", dst);
    // waitKey(0);
    return 0;
}
