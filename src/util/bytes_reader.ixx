export module bytes_reader;

import std.core;

export class BytesReader {
    const uint8_t *bytes;
    size_t len; // bytes len
    std::endian endian;

    size_t saved_pc = 0;
};