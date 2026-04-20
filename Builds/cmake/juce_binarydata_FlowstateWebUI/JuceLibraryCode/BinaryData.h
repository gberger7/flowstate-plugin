/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   index_html;
    const int            index_htmlSize = 67675;

    extern const char*   RubikGlitchRegular_woff2;
    const int            RubikGlitchRegular_woff2Size = 150860;

    extern const char*   RajdhaniRegular_woff2;
    const int            RajdhaniRegular_woff2Size = 52100;

    extern const char*   RajdhaniMedium_woff2;
    const int            RajdhaniMedium_woff2Size = 52668;

    extern const char*   RajdhaniSemiBold_woff2;
    const int            RajdhaniSemiBold_woff2Size = 53588;

    extern const char*   RajdhaniBold_woff2;
    const int            RajdhaniBold_woff2Size = 49460;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 6;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
