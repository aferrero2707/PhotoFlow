FILE(GLOB SOURCES
  "BitPumpJPEG.h"
  "BitPumpLSB.h"
  "BitPumpMSB.h"
  "BitPumpMSB16.h"
  "BitPumpMSB32.h"
  "BitStream.cpp"
  "BitStream.h"
  "Buffer.h"
  "ByteStream.h"
  "Endianness.h"
  "FileIO.h"
  "FileIOException.h"
  "FileReader.cpp"
  "FileReader.h"
  "FileWriter.cpp"
  "FileWriter.h"
  "IOException.h"
)

target_sources(rawspeed PRIVATE
  ${SOURCES}
)
