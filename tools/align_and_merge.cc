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

const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.15f;

// Aligns im1 to im2, putting result in im1Reg, and the homography in h.
void alignImages(const Mat &im1, const Mat &im2, Mat &im1Reg, Mat &h) {
    // Convert images to grayscale
    Mat im1Gray, im2Gray;
    cvtColor(im1, im1Gray, CV_BGR2GRAY);
    cvtColor(im2, im2Gray, CV_BGR2GRAY);

    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
    orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
    orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);

    // Match features.
    std::vector<DMatch> matches;
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    matcher->match(descriptors1, descriptors2, matches, Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches
    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin()+numGoodMatches, matches.end());

    // Extract location of good matches
    std::vector<Point2f> points1, points2;

    for (size_t i = 0; i < matches.size(); i++) {
        points1.push_back(keypoints1[ matches[i].queryIdx ].pt);
        points2.push_back(keypoints2[ matches[i].trainIdx ].pt);
    }

    // Find homography
    h = findHomography( points1, points2, RANSAC );

    // Use homography to warp image
    warpPerspective(im1, im1Reg, h, im2.size());
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        std::cout << "Usage: " << argv[0] << " [-g] img1 img2 dst\n"
            << "\n"
            << "Reads img1 and img2, registers the first to match the second,\n"
            << "and then linearly averages them, putting the result in dst.\n"
            << "If -g is used, then images are treated as grayscale.\n"
            << "Note that command line args must be in this order.\n";
        return 1;
    }

    // Read command line params.  Very hacky.
    bool grayscale = (argc == 5);
    int param_offset = (argc == 5) ? 1 : 0;
    std::string im1_path = argv[1 + param_offset];
    std::string im2_path = argv[2 + param_offset];
    std::string output_path = argv[3 + param_offset];

    Mat im1 = imread(im1_path, IMREAD_COLOR);
    if(im1.empty()) {
        std::cout << "Could not read image: " << im1_path << "\n";
        return 1;
    }

    Mat im2 = imread(im2_path, IMREAD_COLOR);
    if(im2.empty()) {
        std::cout << "Could not read image: " << im2_path << "\n";
        return 1;
    }

    Mat im1_aligned;
    Mat homography;
    alignImages(im1, im2, im1_aligned, homography);

    // merge
    double alpha = 0.5;
    Mat dst;
    if (grayscale) {
        Mat im1_aligned_gray, im2_gray;
        cvtColor(im1_aligned, im1_aligned_gray, CV_BGR2GRAY);
        cvtColor(im2, im2_gray, CV_BGR2GRAY);
        addWeighted(im1_aligned_gray, alpha, im2_gray, 1-alpha, 0.0, dst);
    } else {
        addWeighted(im1_aligned, alpha, im2, 1-alpha, 0.0, dst);
    }

    imwrite(output_path, dst);

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
