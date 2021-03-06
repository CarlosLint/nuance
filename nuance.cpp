#include "nuance.hpp"
#include "nuancec.h"

nuance::nuance(void) {
}

nuance::~nuance(void) {
}

void nuance::errMsg(RECERR rc, char* errBuff, int errBuffSize) {
    LONG ErrExt;
    char szBuff[ERR_BUFFER_SIZE];
    char errStr[ERR_BUFFER_SIZE];
    const char *symb = NULL;

    memset(szBuff, 0, sizeof(szBuff));
    memset(errStr, 0, sizeof(errStr));
    memset(errBuff, 0, errBuffSize);

    kRecGetLastError(&ErrExt, errStr, sizeof(errStr));

    kRecGetErrorInfo(rc, &symb);
    sprintf(szBuff + strlen(szBuff), "%s: ", symb);

    int actlen = strlen(szBuff);
    int remlen = sizeof(szBuff) - actlen - 1;

    kRecGetErrorUIText(rc, ErrExt, errStr, szBuff + actlen, &remlen);

    strncpy(errBuff, szBuff, errBuffSize);
}

void nuance::errStrMsg(const char* msg, char* errBuff, int errBuffSize) {
    memset(errBuff, 0, errBuffSize);
    strncpy(errBuff, msg, errBuffSize);
}

int nuance::SetLicense(const char *licenceFile,
                       const char *oemCode,
                       char *errBuff,
                       const int errSize) {

    RECERR rc = kRecSetLicense(licenceFile, oemCode);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecQuit();
        return -1;
    }
    return 0;
}

int nuance::Init(const char *company,
                 const char *product,
                 char *errBuff,
                 const int errSize) {

    RECERR rc = kRecInit(company, product);
    if ((rc != REC_OK) &&
        (rc != API_INIT_WARN) &&
        (rc != API_LICENSEVALIDATION_WARN)) {

        this->errMsg(rc, errBuff, errSize);
        printf("kRecInit %s\n", errBuff);
        kRecQuit();
        return -1;
    }

    return 0;
}

void nuance::Quit(void) {
    kRecQuit();
}

int nuance::LoadFormTemplateLibrary(const char *templateFile,
                                    char *errBuff,
                                    const int errSize) {

    RECERR rc = kRecLoadFormTemplateLibrary(0,
											templateFile,
											TRUE,
											&this->hFormTemplateArray,
											&this->hFormTemplateArrayLen);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecQuit();
        return -1;
    }
    return 0;
}

int nuance::PreprocessImgWithTemplate(const char *imgFile,
                                      char *errBuff,
                                      const int errSize) {
    RECERR rc = kRecLoadImgF(0, imgFile, &this->hPage, -1);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        RecQuitPlus();
        return -1;
    }

    rc = kRecPreprocessImg(0, this->hPage);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecFreeImg(this->hPage);
        RecQuitPlus();
        return -1;
    }

    FORMTEMPLATEMATCHINGID BestMatchingID;
    int Confidence;
    int nMatching;

    rc = kRecFindFormTemplate(0,
							  this->hPage,
							  this->hFormTemplateArray,
							  this->hFormTemplateArrayLen,
							  NULL,
							  -1,
							  -1,
							  &this->hFormTmplCollection,
							  &BestMatchingID,
							  &Confidence,
							  &nMatching);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecFreeImg(this->hPage);
        kRecQuit();
        return -1;
    }

    if(nMatching < 1) {
        errStrMsg("No matching template!", errBuff, errSize);
        kRecFreeFormTemplateCollection(this->hFormTmplCollection);
        kRecFreeFormTemplateArray(this->hFormTemplateArray,
                                  this->hFormTemplateArrayLen, TRUE);
        kRecFreeImg(this->hPage);
        kRecQuit();
        return -1;
    }

    LPSTR FullName;
    LPSTR TemplateName;
    int nPage, Count;
    // find best matching template
    rc = kRecGetMatchingInfo(BestMatchingID, &FullName, &TemplateName, &nPage, &Count);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecFreeFormTemplateCollection(this->hFormTmplCollection);
        kRecFreeImg(this->hPage);
        kRecQuit();
        return -1;
    }

    rc = kRecApplyFormTemplateEx(0, this->hPage, BestMatchingID);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecFreeFormTemplateCollection(this->hFormTmplCollection);
        kRecFreeImg(this->hPage);
        kRecQuit();
        return -1;
    }

    // Recognize fill zones
    rc = kRecRecognize(0, this->hPage, NULL);
    if (rc != REC_OK) {
        errMsg(rc, errBuff, errSize);
        kRecFreeImg(this->hPage);
        kRecQuit();
        return -1;
    }

    // Get zone content
    rc = kRecGetOCRZoneCount(this->hPage, &this->ZoneCount);
    /*
    for (int i = 0; i < ZoneCount; i++) {
        LPSTR Zonename;
        rc = kRecGetOCRZoneName(this->hPage, i, &Zonename);


        LPSTR Text = NULL;
        int tlen = 0;
        rc = kRecGetOCRZoneText(0, this->hPage, i, &Text, &tlen);

        if (tlen > 0 && Text[tlen-1] == '\n') {
            Text[tlen - 1] = 0;                 // the terminating \n is not printed
        }
        printf(">>>------> %s: %s\n", Zonename, Text ? Text : "");


        kRecFree(Zonename);
        if (Text) {
            kRecFree(Text);
        }
    }
    */
    
    return 0;
}

int nuance::getZoneData(const int zoneID,
                        char *zoneName,
                        const int zoneNameSize,
                        char *zoneText,
                        const int zoneTextSize) {
    LPSTR name;
    RECERR rc = kRecGetOCRZoneName(this->hPage, zoneID, &name);

    strncpy(zoneName, name, zoneNameSize);

    LPSTR Text = NULL;
    int tlen = 0;
    rc = kRecGetOCRZoneText(0, this->hPage, zoneID, &Text, &tlen);

    if (tlen > 0) {
        if (Text[tlen-1] == '\n') {
            Text[tlen - 1] = 0;    // the terminating \n is not printed
        }
        strncpy(zoneText, Text, zoneTextSize);
    }
    //printf(">>>------> %s: %s\n", name, Text ? Text : "");

    kRecFree(name);
    if (Text) {
        kRecFree(Text);
    }

}

int nuance::getZoneCount(void) {
    return this->ZoneCount;
}

int nuance::FreeImgWithTemplate(void) {
    kRecFreeFormTemplateCollection(this->hFormTmplCollection);
    kRecFreeFormTemplateArray(this->hFormTemplateArray,
                              this->hFormTemplateArrayLen, TRUE);
    kRecFreeImg(this->hPage);
    kRecQuit();
}
