#include <filesystem>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmnet/assoc.h"
#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/scu.h"

using namespace std;


void test() {
    cout << "===TEST SCU===" << endl;
    DcmSCU scu;
    scu.releaseAssociation();

    OFList<OFString> transferSyntaxes;
    transferSyntaxes.push_back("1.2.840.10008.1.2.1");
    scu.addPresentationContext("1.2.840.10008.5.1.4.1.1.2", transferSyntaxes);

    if (const OFCondition status = scu.initNetwork(); status.good()) {
        cout << "Network initialized successfully." << endl;
    }else {
        cerr << "Failed to initialize network: " << status.text() << endl;
    }

    scu.setAETitle("MY_SCU");

    scu.setPeerHostName("127.0.0.1");
    scu.setPeerPort(104);
    scu.setPeerAETitle("SCU_AE");

    if (const OFCondition negotiateStatus = scu.negotiateAssociation(); negotiateStatus.good()) {
        cout << "NegotiateAssociation successful." << endl;
    }else if (negotiateStatus == NET_EC_AlreadyConnected){
        cout << "Already connected to a peer." << endl;
    }else {
        cerr << "Failed to negotiate association: " << negotiateStatus.text() << endl;
    }

    scu.freeNetwork();
}
