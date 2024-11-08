#include <iostream>
#include <filesystem>
#include <vector>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = filesystem;
using namespace fs;

vector<string> getPath(vector<string> paths) {
    vector<string> filePaths;
    for (int i = 0; i < paths.size(); i++) {
        const string& path = paths[i];
        try {
            if (exists(path) && is_directory(path)) {
                for (const auto& entry : directory_iterator(path)) {
                    entry.is_directory() ? paths.push_back(entry.path()) : filePaths.push_back(entry.path());
                }
            } else {
                cerr << "Path does not exist or is not a directory." << endl;
            }
        } catch (const filesystem_error& e) {
            cerr << "Filesystem error: " << e.what() << endl;
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
    return filePaths;
}

void display(const string &path) {
    auto *image = new DicomImage(path.data());

    if (image->getStatus() == EIS_Normal) {
        const void* pixelData = image->getOutputData(8);
        const int width = static_cast<int> (image->getWidth());
        const int height = static_cast<int>(image->getHeight());
        const Mat img(height, width, CV_8UC1, const_cast<void *>(pixelData));

        imshow("DICOM Image", img);
        waitKey(0);
    } else {
        cerr << "Error loading DICOM image." << endl;
    }
    delete image;
}


void displayAll(const vector<string>& paths) {

    for (int i = 0; i < paths.size(); i++) {
        cout << i << endl;
        const auto &path = paths[i];
        display(path);
    }
}

int main() {
    const auto dictPath = "/mnt/c/Windows/System32/dcmtk/dcmdata/data/dicom.dic";
    setenv("DCMDICTPATH", dictPath, 1);

    vector<string> folder_path;
    const string path = "/mnt/c/HoangTu/Programing/Data/CT3";
    folder_path.push_back(path);

    const vector<string> paths = getPath(folder_path);

    displayAll(paths);
    return 0;
}
