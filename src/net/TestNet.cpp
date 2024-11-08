#include <filesystem>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"

#include "SCU.h"

using namespace cv;
using namespace std;
namespace fs = filesystem;
using namespace fs;

const string dicomFilePath = "/mnt/c/HoangTu/Programing/Data/CT3/1.2.840.1728440274.894.1812400989643967/1.2.840.1728440274.894.1812400989644550.dcm";

int main(){

    test();

    return 0;
}
