#include "CSBlob.h"
#include "CMSDecoding.h"

int main(int argc, char *argv[]) {

    // Sanity check passed arguments
    if (argc != 2) {
        printf("Usage: %s <path to MachO file>\n", argv[0]);
        return 1;
    }

    // Initialise the MachO structure
    printf("Initialising MachO structure from %s.\n", argv[1]);
    MachO macho;
    if (initMachOWithPath(argv[1], &macho) != 0) { return -1; }

    // Parse the code signature blob
    printf("Parsing CMS superblobs from MachO.\n");
    for (int sliceIndex = 0; sliceIndex < macho._sliceCount; sliceIndex++) {
        if (parseSuperBlob(&macho, NULL, sliceIndex) != 0) {
            if (macho._sliceCount > 1) {
                if (macho._slices[sliceIndex]._isValid) {
                    printf("Slice %d does not contain a code signature.\n", sliceIndex + 1);
                }
            } else {
                printf("Binary does not contain a code signature.\n");
                return -1; 
            }
        }
    }

    // Extract CMS data to file
    printf("Extracting CMS data from first slice to file.\n");
    CS_SuperBlob superblob;
    if (parseSuperBlob(&macho, &superblob, 0) == 0) {
        extractCMSToFile(&macho, &superblob, 0);

        // TODO: Extract this from the CMS data
        FILE *cmsDERFile = fopen("CMS-DER", "rb");
        fseek(cmsDERFile, 0, SEEK_END);
        size_t cmsDERLength = ftell(cmsDERFile);
        fseek(cmsDERFile, 0, SEEK_SET);
        uint8_t *cmsDERData = malloc(cmsDERLength);
        fread(cmsDERData, cmsDERLength, 1, cmsDERFile);
        fclose(cmsDERFile);

        decodeCMSData(cmsDERData, cmsDERLength);

        // Clean up
        free(cmsDERData);
    } else {
        if (macho._sliceCount > 1) {
            if (macho._slices[0]._isValid) {
                printf("First slice does not contain a code signature.\n");
            } else {
                printf("Could not parse CMS data for ARMv7 slice.\n");
            }
        } else {
            printf("Binary does not contain a code signature.\n");
            return -1; 
        }
    }
    freeMachO(&macho);

    return 0;
    
}