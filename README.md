# libchunking
Library to split (some) media objects into individually intelligible chunks that may contain an incomplete and/or lower
resolution version of the original object.
The splitting process is done in such a way that any set of the resulting chunks can be reassembled to produce an object that is lower resolution than the original one, but higher resolution than an object reassembled with a lower number of chunks.
All of the chunks can be reassembled to restore the full quality of the original object.
The library also supports extraction of portions of the original object that can be "overlapped" over lower resolution chunks.

Currently only images (bitmaps, jpeg, png) are fully supported.  MPEG 1 videos (without audio) are partially supported.

## Using this Library
### Building
Makefiles are provided to build the library for linux and android, moreover a Visual Studio 2015 solution is provided to build the libary on Microsoft Windows. They can be found at`chunking/linux`, `chunking/android`, and `chunking\win32` respectively.

### Chunking

The `Chunker` class offers the `chunking` static member function:

```
static Fragments * fragmentBuffer (const void *pBuf, uint32 ui32Len, Type inputObjectType,
                                   uint8 ui8NoOfChunks, Type outputChunkType,
                                   uint8 ui8ChunkCompressionQuality);
```

The media object to be chunked will be contained in a buffer of length `ui32Len`, pointed by `pBuf`.  `inputObjectType` is the format of original object, while `outputChunkType` is the format of the generated chunks.`ui8ChunkCompressionQuality`specifies the maximum level of compression that can be used to compress the generated chunks when lossy compression is used. It is expressed as percentage, and therefore it ranges from 0 to 100.

The `Chunker` class also offers a static member function to "extract" a high resolution portion of the original object.
```
Fragment * extractFromBuffer (const void *pBuf, uint32 ui32Len, Type inputObjectType, Type outputChunkType,
                              uint8 ui8ChunkCompressionQuality, Interval **ppPortionIntervals)
```
The parameters are similar to the `fragmentBuffer` member function, except for the `ppPortionIntervals` parameter that specifies the area to be extracted.

### Reassembling

The `ChunkReassembler` object offers the member functions that follows:
```
int incorporateChunk (const void *pBuf, uint32 ui32BufLen, Chunker::Type chunkType, uint8 ui8ChunkId);
int incorporateAnnotation (Chunker::Interval **ppIntervals, const void *pBuf, uint32 ui32BufLen,
                           Chunker::Type chunkType);

NOMADSUtil::BufferReader * getReassembledObject (Chunker::Type outputType, uint8 ui8CompressionQuality);
NOMADSUtil::BufferReader * getAnnotatedObject (Chunker::Type outputType, uint8 ui8CompressionQuality);
```
After initializing a `ChunkReassembler` object with the `init` member function, chunks and high resolution portions of the original objects (annotations) can be added by using the `incorporateChunk`, and `incorporateAnnotation` member functions respectively.  After adding chunks and annotations, the `getReassembledObject` and `getAnnotatedObject` member functions can be used to retrieved the reassembled object.

