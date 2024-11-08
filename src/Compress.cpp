#include <filesystem>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"
#include "dcmtk/dcmpstat/dvpstat.h"

using namespace std;
using namespace cv;
namespace fs = filesystem;
using namespace fs;

vector<string> getPath(vector<string> paths) {
    vector<string> filePaths;
    for (int i = 0; i < paths.size(); i++) {
        const string &path = paths[i];
        try {
            if (exists(path) && is_directory(path)) {
                for (const auto &entry: directory_iterator(path)) {
                    entry.is_directory() ? paths.push_back(entry.path()) : filePaths.push_back(entry.path());
                }
            } else {
                cerr << "Path does not exist or is not a directory." << endl;
            }
        } catch (const filesystem_error &e) {
            cerr << "Filesystem error: " << e.what() << endl;
        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
    return filePaths;
}

Mat exchangeBright(Mat src, const int c = 20) {
    if (c == 0) {
        return src;
    }
    Mat dst = src.clone();

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            for (int k = 0; k < src.channels(); k++) {
                int pixel = static_cast<int>(src.at<uchar>(i, j, k)) + c;
                if (pixel > 255) {
                    pixel = 255;
                } else if (pixel < 0) {
                    pixel = 0;
                }
                dst.at<uchar>(i, j) = pixel;
            }
        }
    }

    return dst;
}


void display(const string &path, const char *title = "DICOM Image", const int bright = 0) {
    auto *image = new DicomImage(path.data());

    if (image->getStatus() == EIS_Normal) {
        const void *pixelData = image->getOutputData(8);
        const int width = static_cast<int>(image->getWidth());
        const int height = static_cast<int>(image->getHeight());
        const Mat img(height, width, CV_8UC1, const_cast<void *>(pixelData));

        imshow(title, exchangeBright(img, bright));
        waitKey(0);
    } else {
        cerr << "Error loading DICOM image." << endl;
    }
    delete image;
}


void displayAll(const vector<string> &paths) {
    for (int i = 0; i < paths.size(); i++) {
        cout << i << endl;
        const auto &path = paths[i];
        display(path);
    }
}

bool encoding(const string &inputPath, const string &outputPath,
              const E_TransferSyntax transferSyntax = EXS_JPEGProcess14SV1) {
    DJEncoderRegistration::registerCodecs();
    if (DcmFileFormat fileFormat; fileFormat.loadFile(inputPath.data()).good()) {
        DcmDataset *dataset = fileFormat.getDataset();
        const DJ_RPLossless params;

        if (dataset->chooseRepresentation(transferSyntax, &params).good() &&
            dataset->canWriteXfer(transferSyntax)) {
            fileFormat.saveFile(outputPath.data(), transferSyntax);
        } else {
            DJEncoderRegistration::cleanup();
            return false;
        }
    }
    DJEncoderRegistration::cleanup();
    return true;
}

bool decoding(const string &path,
              const string &outputPath = "../test_decompressed.dcm",
              const E_TransferSyntax transferSyntax = EXS_LittleEndianImplicit) {
    DJDecoderRegistration::registerCodecs(); // register JPEG codecs
    if (DcmFileFormat fileFormat; fileFormat.loadFile(path.data()).good()) {
        if (DcmDataset *dataset = fileFormat.getDataset();
            dataset->chooseRepresentation(transferSyntax, nullptr).good() &&
            dataset->canWriteXfer(transferSyntax)) {
            fileFormat.saveFile(outputPath.data(), transferSyntax);
        } else {
            DJDecoderRegistration::cleanup();
            return false;
        }
    }
    DJDecoderRegistration::cleanup();
    return true;
}

Mat convertToMat(DicomImage *image) {
    const void *pixelData = image->getOutputData(8);
    const int width = static_cast<int>(image->getWidth());
    const int height = static_cast<int>(image->getHeight());
    const Mat img(height, width, CV_8UC1, const_cast<void *>(pixelData));
    return img;
}

double calculatePNSR(const string &origin, const string &decompressed) {
    auto *originalImage = new DicomImage(origin.data());
    auto *compressedImage = new DicomImage(decompressed.data());

    if (originalImage->getStatus() != EIS_Normal || compressedImage->getStatus() != EIS_Normal) {
        std::cerr << "Error: Failed to load DICOM images!" << std::endl;
        return -1;
    }

    Mat originMat = convertToMat(originalImage);
    Mat decompressedMat = convertToMat(compressedImage);

    const unsigned int size = originMat.rows * originMat.cols;

    double mse = 0.0;
    for (int i = 0; i < originMat.rows; i++) {
        for (int j = 0; j < originMat.cols; j++) {
            const int diff = originMat.at<u_char>(i, j) - decompressedMat.at<u_char>(i, j);
            mse += diff * diff;
        }
    }

    delete originalImage, delete compressedImage, originMat.release(), decompressedMat.release();

    mse /= size;
    if (mse == 0) {
        return INFINITY;
    }

    return 10.0 * log10((255.0 * 255.0) / mse);;
}

void deleteFiles(const vector<string> &paths) {
    for (const auto &path: paths) {
        if (exists(path)) {
            remove(path.c_str());
        }
    }
}


void statTest(const string *path) {
    if (DcmFileFormat infile; infile.loadFile(path->data()).good()) {
        if (DVPresentationState state; state.createFromImage(*infile.getDataset()).good()) {
            // serialize presentation state into DICOM data set structure
            if (DcmFileFormat outfile; state.write(*outfile.getDataset(), OFFalse).good()) {
                // and write to file
                outfile.saveFile("../temp.dcm", EXS_LittleEndianExplicit);
            }
        }
    }
}

int main() {
    const auto dictPath = "/mnt/c/Windows/System32/dcmtk/dcmdata/data/dicom.dic";
    setenv("DCMDICTPATH", dictPath, 1);

    vector<string> folder_path;
    const string dataPath = "/mnt/c/HoangTu/Programing/Data/CT3";
    folder_path.push_back(dataPath);

    const vector<string> paths = getPath(folder_path);

    const string &inputPath = paths[200];

    constexpr E_TransferSyntax tag = EXS_JPEGProcess14SV1;
    if (const string &outputPath = "../output.dcm";
        encoding(inputPath, outputPath, tag)) {
        constexpr int bright = 20;
        display(inputPath, "Original Image", bright);
        const string decompressedPath = "decompressed.dcm";
        if (decoding(outputPath, decompressedPath)) {
            display(decompressedPath, "Decompressed Image", bright);
            cout << "PNSR: " << calculatePNSR(inputPath, decompressedPath) << endl;
        } else {
            cerr << "Error in Decoding DICOM image." << endl;
        }
        vector<string> files;
        files.push_back(outputPath);
        files.push_back(decompressedPath);
        deleteFiles(files);
    } else {
        cerr << "Error in Encoding DICOM image." << endl;
    }

    statTest(&inputPath);

    return 0;
}
