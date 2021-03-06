
#ifndef I_SOUND_ENGINE_PACKAGEENCODER_H
#define I_SOUND_ENGINE_PACKAGEENCODER_H

#include <vector>
#include <string>
#include "AudioFormats/WavFile.h"
#include "ErrorList.h"
#include "SoundData.h"
// TODO make system to handle all errors




class PackageEncoder
{
public:

    //PackageEncoder();

    /*!
     * Adds a new pcm(wav) file to the bank
     * @param wav The wave file to encode
     * @param id Id that events will refer to this audio file as
     * @param format Wether file should be encoded to be in opus or pcm
     * @return Error
     */
    ErrorNum AddFile(WavFile &wav, uint64_t id, Encoding format);

    ErrorNum AddFile(const std::string &wav, uint64_t id, Encoding format);

     /*!
      * Generates the entire bank then writes it
      * @param path The output path + file name for the bank
      * @return Error
      */
     ErrorNum WritePackage(std::string path);

private:

    uint64_t bufferSize = 0;

    struct FileInfo
    {
        std::string wavPath;
        uint64_t id;
        Encoding encoding;
    };

    std::vector<FileInfo> filesToEncode;

    int WritePCM(char* buffer, FileInfo& file);
    int WriteOpus(char* buffer, FileInfo& file);

};


#endif //I_SOUND_ENGINE_PACKAGEENCODER_H
