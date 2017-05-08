/*
 * BMPHandler.cpp
 *
* This file is part of the IHMC Misc Library
 * Copyright (c) 2010-2016 IHMC.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 3 (GPLv3) as published by the Free Software Foundation.
 *
 * U.S. Government agencies and organizations may redistribute
 * and/or modify this program under terms equivalent to
 * "Government Purpose Rights" as defined by DFARS 
 * 252.227-7014(a)(12) (February 2014).
 *
 * Alternative licenses that allow for use within commercial products may be
 * available. Contact Niranjan Suri at IHMC (nsuri@ihmc.us) for details.
 */

#include "BMPHandler.h"

#include "BMPImage.h"
#include "Logger.h"
#include "NLFLib.h"

#include <assert.h>

using namespace IHMC_MISC;
using namespace NOMADSUtil;

#define checkAndLogMsg if (pLogger) pLogger->logMsg
#define nullSrcImgErrMsg Logger::L_MildError, "source image is null\n"
#define wrongBitsPerPixErrMsg(n) Logger::L_MildError, "cannot handle source image with %d bits/pixel\n",n
#define faildToInitBitmapErrMsg Logger::L_MildError, "failed to initialize a new bitmap of size %lux%lu and %d bits/pixel; rc = %d\n"

namespace IHMC_MISC
{
    class BMPHandler
    {
        public:
            static uint8 computeXIncrement (uint8 ui8TotalNoOfChunks);
            static uint8 computeYIncrement (uint8 ui8TotalNoOfChunks);
            static uint8 computeXOffset (uint8 ui8ChunkId, uint8 ui8TotalNoOfChunks);
            static uint8 computeYOffset (uint8 ui8ChunkId, uint8 ui8TotalNoOfChunks);
            static uint8 computeChunkIdForOffset (uint8 ui8XOffset, uint8 ui8YOffset, uint8 ui8TotalNoOfChunks);
    };
}

uint8 BMPHandler::computeXIncrement (uint8 ui8TotalNoOfChunks)
{
    switch (ui8TotalNoOfChunks) {
        case 2:
            return 1;
        case 4:
            return 2;
        case 8:
            return 2;
        case 16:
            return 4;
    }
    return 0;
}

uint8 BMPHandler::computeYIncrement (uint8 ui8TotalNoOfChunks)
{
    switch (ui8TotalNoOfChunks) {
        case 2:
            return 2;
        case 4:
            return 2;
        case 8:
            return 4;
        case 16:
            return 4;
    }
    return 0;
}

uint8 BMPHandler::computeXOffset (uint8 ui8ChunkId, uint8 ui8TotalNoOfChunks)
{
    if ((ui8ChunkId == 0) || (ui8ChunkId > ui8TotalNoOfChunks)) {
        return 0;
    }
    if (computeXIncrement (ui8TotalNoOfChunks) == 0) {
        return 0;
    }
    else {
        return (ui8ChunkId - 1) % computeXIncrement (ui8TotalNoOfChunks);
    }
}

uint8 BMPHandler::computeYOffset (uint8 ui8ChunkId, uint8 ui8TotalNoOfChunks)
{
    if ((ui8ChunkId == 0) || (ui8ChunkId > ui8TotalNoOfChunks)) {
        return 0;
    }
    if (computeYIncrement (ui8TotalNoOfChunks) == 0) {
        return 0;
    }
    else {
        return (ui8ChunkId - 1) / computeXIncrement (ui8TotalNoOfChunks);
    }
}

uint8 BMPHandler::computeChunkIdForOffset (uint8 ui8XOffset, uint8 ui8YOffset, uint8 ui8TotalNoOfChunks)
{
    return ui8XOffset + (ui8YOffset * computeXIncrement (ui8TotalNoOfChunks)) + 1;
}

BMPImage * BMPChunker::fragmentBMP (BMPImage *pSourceImage, uint8 ui8DesiredChunkId, uint8 ui8TotalNoOfChunks)
{
    if (pSourceImage == NULL) {
        checkAndLogMsg("BMPChunker::fragmentBMP", nullSrcImgErrMsg);
        return NULL;
    }
    if (pSourceImage->getBitsPerPixel() != 24) {
        checkAndLogMsg ("BMPChunker::fragmentBMP", wrongBitsPerPixErrMsg
                        ((int)pSourceImage->getBitsPerPixel()));
        return NULL;
    }
    if (ui8DesiredChunkId == 0) {
        checkAndLogMsg ("BMPChunker::fragmentBMP", Logger::L_MildError,
                        "desired chunk id of 0 is invalid\n");
        return NULL;
    }
    if (ui8DesiredChunkId > ui8TotalNoOfChunks) {
        checkAndLogMsg ("BMPChunker::fragmentBMP", Logger::L_MildError,
                        "desired chunk id <%d> exceeds total number of chunks <%d>\n",
                        (int) ui8DesiredChunkId, (int) ui8TotalNoOfChunks);
        return NULL;
    }
    int rc;
    uint8 ui8XIncr = BMPHandler::computeXIncrement (ui8TotalNoOfChunks);
    uint8 ui8YIncr = BMPHandler::computeYIncrement (ui8TotalNoOfChunks);
    if ((ui8XIncr == 0) || (ui8YIncr == 0)) {
        checkAndLogMsg ("BMPChunker::fragmentBMP", Logger::L_MildError,
                        "cannot handle chunking BMP into %d chunks\n",
                        (int) ui8TotalNoOfChunks);
        return NULL;
    }
    uint32 ui32NewWidth = ceiling ((pSourceImage->getWidth() / ui8XIncr), ui8XIncr);
    uint32 ui32NewHeight = ceiling ((pSourceImage->getHeight() / ui8YIncr), ui8YIncr);
    BMPImage *pChunkedImage = new BMPImage (true);
    if (0 != (rc = pChunkedImage->initNewImage (ui32NewWidth, ui32NewHeight, pSourceImage->getBitsPerPixel()))) {
        checkAndLogMsg ("BMPChunker::fragmentBMP", Logger::L_MildError,
                        "could not initialize a new bitmap of size %lux%lu and %d bits/pixel; rc = %d\n",
                        ui32NewWidth, ui32NewHeight, pSourceImage->getBitsPerPixel());
        return NULL;
    }
    uint8 ui8XOff = BMPHandler::computeXOffset (ui8DesiredChunkId, ui8TotalNoOfChunks);
    uint8 ui8YOff = BMPHandler::computeYOffset (ui8DesiredChunkId, ui8TotalNoOfChunks);
    uint32 ui32XSrc = 0, ui32XDest = 0;
    while (ui32XSrc < pSourceImage->getWidth()) {
        uint32 ui32YSrc = 0, ui32YDest = 0;
        while (ui32YSrc < pSourceImage->getHeight()) {
            uint8 ui8Red, ui8Green, ui8Blue;
            pSourceImage->getPixel (ui32XSrc + ui8XOff, ui32YSrc + ui8YOff, &ui8Red, &ui8Green, &ui8Blue);
            pChunkedImage->setPixel (ui32XDest, ui32YDest, ui8Red, ui8Green, ui8Blue);
            ui32YSrc += ui8YIncr;
            ui32YDest++;
        }
        ui32XSrc += ui8XIncr;
        ui32XDest++;
    }
    return pChunkedImage;
}

BMPImage * BMPChunker::extractFromBMP (BMPImage *pSourceImage, uint32 ui32StartX, uint32 ui32EndX, uint32 ui32StartY, uint32 ui32EndY)
{
    if (pSourceImage == NULL) {
        checkAndLogMsg ("BMPChunker::extractFromBMP", nullSrcImgErrMsg);
        return NULL;
    }
    if (pSourceImage->getBitsPerPixel() != 24) {
        checkAndLogMsg ("BMPChunker::extractFromBMP", wrongBitsPerPixErrMsg
                        ((int)pSourceImage->getBitsPerPixel()));
        return NULL;
    }
    if ((ui32EndX <= ui32StartX) || (ui32StartX > pSourceImage->getWidth()) || (ui32EndX > pSourceImage->getWidth())) {
        return NULL;
    }
    if ((ui32EndY <= ui32StartY) || (ui32StartY > pSourceImage->getHeight()) || (ui32EndY > pSourceImage->getHeight())) {
        return NULL;
    }

    int rc = 0;
    const uint32 ui32NewWidth = ui32EndX - ui32StartX + 1;
    const uint32 ui32NewHeight = ui32EndY - ui32StartY + 1;
    BMPImage *pExtractedImage = new BMPImage (false);
    if (0 != (rc = pExtractedImage->initNewImage (ui32NewWidth, ui32NewHeight, pSourceImage->getBitsPerPixel()))) {
        checkAndLogMsg ("BMPChunker::extractFromBMP", Logger::L_MildError,
                        "could not initialize a new bitmap of size %lux%lu and %d bits/pixel; rc = %d\n",
                        ui32NewWidth, ui32NewHeight, pSourceImage->getBitsPerPixel());
        return NULL;
    }
    for (uint32 x = ui32StartX; x <= ui32EndX; x++) {
        for (uint32 y = ui32StartY; y <= ui32EndY; y++) {
            uint8 ui8Red, ui8Green, ui8Blue;
            assert (pSourceImage->getPixel (x, y, &ui8Red, &ui8Green, &ui8Blue) == 0);
            assert (pExtractedImage->setPixel ((x - ui32StartX), (y - ui32StartY), ui8Red, ui8Green, ui8Blue) == 0);
        }
    }
    return pExtractedImage;
}

BMPReassembler::BMPReassembler (void)
    : _addedAnnotations (false),
      _ui8TotalNoOfChunks (0),
      _ui8XIncr (0),
      _ui8YIncr (0),
      _pResultingImage (NULL)
{
    for (int i = 0; i < 256; i++) {
        _abAddedChunks [i] = false;
    }
}

BMPReassembler::~BMPReassembler (void)
{
    if (_pResultingImage != NULL) {
        delete _pResultingImage;
        _pResultingImage = NULL;
    }
}

int BMPReassembler::init (uint8 ui8TotalNoOfChunks)
{
    _ui8XIncr = BMPHandler::computeXIncrement (ui8TotalNoOfChunks);
    _ui8YIncr = BMPHandler::computeYIncrement (ui8TotalNoOfChunks);
    if ((_ui8XIncr == 0) || (_ui8YIncr == 0)) {
        checkAndLogMsg ("BMPReassembler::init", Logger::L_MildError,
                        "cannot handle reassembling BMP with %d chunks\n",
                        (int) ui8TotalNoOfChunks);
        return -1;
    }
    _ui8TotalNoOfChunks = ui8TotalNoOfChunks;
    if (_pResultingImage != NULL) {
        delete _pResultingImage;
        _pResultingImage = NULL;
    }
    return 0;
}

int BMPReassembler::incorporateChunk (BMPImage *pChunk, uint8 ui8ChunkId)
{
    const char *pszMethodName = "BMPReassembler::incorporateChunk";
    int rc;
    if ((_ui8XIncr == 0) || (_ui8YIncr == 0)) {
        checkAndLogMsg (pszMethodName, Logger::L_MildError,
                        "BMPReassembler has not been initialized\n");
        return -1;
    }
    if (pChunk == NULL) {
        checkAndLogMsg (pszMethodName, Logger::L_MildError,
                        "chunk parameter is NULL\n");
        return -2;
    }
    if ((ui8ChunkId == 0) || (ui8ChunkId > _ui8TotalNoOfChunks)) {
        checkAndLogMsg (pszMethodName, Logger::L_MildError,
                        "chunk id of <%s> is invalid\n", ui8ChunkId);
        return -3;
    }
    if (_pResultingImage == NULL) {
        _pResultingImage = new BMPImage (true);
        if (0 != (rc = _pResultingImage->initNewImage (pChunk->getWidth() * _ui8XIncr, pChunk->getHeight() * _ui8YIncr, pChunk->getBitsPerPixel()))) {
            checkAndLogMsg (pszMethodName, faildToInitBitmapErrMsg, pChunk->getWidth() * _ui8XIncr,
                            pChunk->getHeight() * _ui8YIncr, (int) pChunk->getBitsPerPixel());
            return -4;
        }
    }
    else if ((_pResultingImage->getWidth() != (pChunk->getWidth() * _ui8XIncr)) ||
            (_pResultingImage->getHeight() != (pChunk->getHeight() * _ui8YIncr))) {
        checkAndLogMsg (pszMethodName, Logger::L_MildError, "chunk size of %lux%lu is not compatible "
                        "with reassembled image size of %lux%lu composed of %d chunks\n", pChunk->getWidth(),
                        pChunk->getHeight(), _pResultingImage->getWidth(), _pResultingImage->getHeight(),
                        (int) _ui8TotalNoOfChunks);
            return -5;
    }
    uint8 ui8XOff = BMPHandler::computeXOffset (ui8ChunkId, _ui8TotalNoOfChunks);
    uint8 ui8YOff = BMPHandler::computeYOffset (ui8ChunkId, _ui8TotalNoOfChunks);
    uint32 ui32XSrc = 0, ui32XDest = 0;
    while (ui32XSrc < pChunk->getWidth()) {
        uint32 ui32YSrc = 0, ui32YDest = 0;
        while (ui32YSrc < pChunk->getHeight()) {
            uint8 ui8Red, ui8Green, ui8Blue;
            pChunk->getPixel (ui32XSrc, ui32YSrc, &ui8Red, &ui8Green, &ui8Blue);
            _pResultingImage->setPixel (ui32XDest+ui8XOff, ui32YDest+ui8YOff, ui8Red, ui8Green, ui8Blue);
            ui32YSrc++;
            ui32YDest += _ui8YIncr;
        }
        ui32XSrc++;
        ui32XDest += _ui8XIncr;
    }
    _abAddedChunks[ui8ChunkId] = true;
    return 0;
}

int BMPReassembler::incorporateChunk (BMPImage *pChunk, uint32 ui32StartX, uint32 ui32EndX, uint32 ui32StartY, uint32 ui32EndY)
{
    const char *pszMethodName = "BMPReassembler::incorporateChunk";
    if (pChunk == NULL) {
        checkAndLogMsg (pszMethodName, Logger::L_MildError, "chunk parameter is NULL\n");
        return -2;
    }
    if (_pResultingImage == NULL) {
        return -3;
    }
    if ((_pResultingImage->getWidth() < pChunk->getWidth()) || (_pResultingImage->getHeight() < pChunk->getHeight())) {
        checkAndLogMsg (pszMethodName, Logger::L_MildError, "chunk size of %lux%lu is not compatible "
                        "with reassembled image size of %lux%lu\n", pChunk->getWidth(),
                        pChunk->getHeight(), _pResultingImage->getWidth(), _pResultingImage->getHeight());
        return -4;
    }
    if (!_addedAnnotations) {
        if (interpolate() < 0) {
            return -5;
        }
        _addedAnnotations = true;
    }
    checkAndLogMsg (pszMethodName, Logger::L_Info, "%u %u %u %u\n", ui32StartX, ui32EndX, ui32StartY, ui32EndY);
    for (uint32 x = 0; x < pChunk->getWidth(); x++) {
        for (uint32 y = 0; y < pChunk->getHeight(); y++) {
            uint8 ui8Red, ui8Green, ui8Blue;
            pChunk->getPixel (x, y, &ui8Red, &ui8Green, &ui8Blue);
            _pResultingImage->setPixel ((x + ui32StartX), (y + ui32StartY), ui8Red, ui8Green, ui8Blue);
        }
    }
    return 0;
}

const BMPImage * BMPReassembler::getAnnotatedImage (void)
{
    if (_addedAnnotations) {
        return _pResultingImage;
    }
    return NULL;
}

const BMPImage * BMPReassembler::getReassembledImage (void)
{
    if (interpolate() < 0) {
        return NULL;
    }
    return _pResultingImage;
}

int BMPReassembler::interpolate (void)
{
    if ((_ui8XIncr == 0) || (_ui8YIncr == 0)) {
        checkAndLogMsg ("BMPReassmebler::interpolate", Logger::L_MildError,
                        "BMPReassembler has not been initialized\n");
        return -1;
    }
    if (_pResultingImage == NULL) {
        checkAndLogMsg ("BMPReassembler::interpolate", Logger::L_MildError,
                        "no chunks have been incorporated\n");
        return -2;
    }
    // Do interpolation
    uint32 ui32X = 0;
    while (ui32X < _pResultingImage->getWidth()) {
        uint32 ui32Y = 0;
        while (ui32Y < _pResultingImage->getHeight()) {
            uint8 ui8SampleCount = 0;
            uint16 ui16SumR = 0, ui16SumG = 0, ui16SumB = 0;
            for (uint8 ui8X = 0; ui8X < _ui8XIncr; ui8X++) {
                for (uint8 ui8Y = 0; ui8Y < _ui8YIncr; ui8Y++) {
                    if (_abAddedChunks[BMPHandler::computeChunkIdForOffset (ui8X, ui8Y, _ui8TotalNoOfChunks)]) {
                        uint8 ui8R, ui8G, ui8B;
                        _pResultingImage->getPixel (ui32X + ui8X, ui32Y + ui8Y, &ui8R, &ui8G, &ui8B);
                        ui16SumR += ui8R;
                        ui16SumG += ui8G;
                        ui16SumB += ui8B;
                        ui8SampleCount++;
                    }
                }
            }
            if (ui8SampleCount == 0) {
                // Should not have happened
                checkAndLogMsg ("BMPReassembler::interpolate", Logger::L_MildError,
                                "internal error - no samples found\n");
                return -3;
            }
            uint8 ui8AvgR = ui16SumR / ui8SampleCount;
            uint8 ui8AvgG = ui16SumG / ui8SampleCount;
            uint8 ui8AvgB = ui16SumB / ui8SampleCount;
            for (uint8 ui8X = 0; ui8X < _ui8XIncr; ui8X++) {
                for (uint8 ui8Y = 0; ui8Y < _ui8YIncr; ui8Y++) {
                    if (!_abAddedChunks[BMPHandler::computeChunkIdForOffset (ui8X, ui8Y, _ui8TotalNoOfChunks)]) {
                        _pResultingImage->setPixel (ui32X + ui8X, ui32Y + ui8Y, ui8AvgR, ui8AvgG, ui8AvgB);
                    }
                }
            }
            ui32Y += _ui8YIncr;
        }
        ui32X += _ui8XIncr;
    }
    return 0;
}

