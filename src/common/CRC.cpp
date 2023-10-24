/*
 *  Copyright (c) 2023 Tair
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include "common/CRC.hpp"

namespace tair::common {

// clang-format off
static const uint16_t crc16tab[256]= {
        0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
        0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
        0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
        0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
        0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
        0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
        0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
        0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
        0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
        0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
        0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
        0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
        0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
        0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
        0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
        0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
        0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
        0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
        0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
        0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
        0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
        0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
        0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
        0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
        0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
        0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
        0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
        0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
        0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
        0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
        0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
        0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};
// clang-format on

uint16_t CRC::crc16(const char *buf, int len) {
    int counter;
    uint16_t crc = 0;
    for (counter = 0; counter < len; counter++) {
        crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ *buf++) & 0x00FF];
    }
    return crc;
}

// clang-format off
static const uint64_t crc64_tab[256] = {
        UINT64_C(0x0000000000000000), UINT64_C(0x7ad870c830358979),
        UINT64_C(0xf5b0e190606b12f2), UINT64_C(0x8f689158505e9b8b),
        UINT64_C(0xc038e5739841b68f), UINT64_C(0xbae095bba8743ff6),
        UINT64_C(0x358804e3f82aa47d), UINT64_C(0x4f50742bc81f2d04),
        UINT64_C(0xab28ecb46814fe75), UINT64_C(0xd1f09c7c5821770c),
        UINT64_C(0x5e980d24087fec87), UINT64_C(0x24407dec384a65fe),
        UINT64_C(0x6b1009c7f05548fa), UINT64_C(0x11c8790fc060c183),
        UINT64_C(0x9ea0e857903e5a08), UINT64_C(0xe478989fa00bd371),
        UINT64_C(0x7d08ff3b88be6f81), UINT64_C(0x07d08ff3b88be6f8),
        UINT64_C(0x88b81eabe8d57d73), UINT64_C(0xf2606e63d8e0f40a),
        UINT64_C(0xbd301a4810ffd90e), UINT64_C(0xc7e86a8020ca5077),
        UINT64_C(0x4880fbd87094cbfc), UINT64_C(0x32588b1040a14285),
        UINT64_C(0xd620138fe0aa91f4), UINT64_C(0xacf86347d09f188d),
        UINT64_C(0x2390f21f80c18306), UINT64_C(0x594882d7b0f40a7f),
        UINT64_C(0x1618f6fc78eb277b), UINT64_C(0x6cc0863448deae02),
        UINT64_C(0xe3a8176c18803589), UINT64_C(0x997067a428b5bcf0),
        UINT64_C(0xfa11fe77117cdf02), UINT64_C(0x80c98ebf2149567b),
        UINT64_C(0x0fa11fe77117cdf0), UINT64_C(0x75796f2f41224489),
        UINT64_C(0x3a291b04893d698d), UINT64_C(0x40f16bccb908e0f4),
        UINT64_C(0xcf99fa94e9567b7f), UINT64_C(0xb5418a5cd963f206),
        UINT64_C(0x513912c379682177), UINT64_C(0x2be1620b495da80e),
        UINT64_C(0xa489f35319033385), UINT64_C(0xde51839b2936bafc),
        UINT64_C(0x9101f7b0e12997f8), UINT64_C(0xebd98778d11c1e81),
        UINT64_C(0x64b116208142850a), UINT64_C(0x1e6966e8b1770c73),
        UINT64_C(0x8719014c99c2b083), UINT64_C(0xfdc17184a9f739fa),
        UINT64_C(0x72a9e0dcf9a9a271), UINT64_C(0x08719014c99c2b08),
        UINT64_C(0x4721e43f0183060c), UINT64_C(0x3df994f731b68f75),
        UINT64_C(0xb29105af61e814fe), UINT64_C(0xc849756751dd9d87),
        UINT64_C(0x2c31edf8f1d64ef6), UINT64_C(0x56e99d30c1e3c78f),
        UINT64_C(0xd9810c6891bd5c04), UINT64_C(0xa3597ca0a188d57d),
        UINT64_C(0xec09088b6997f879), UINT64_C(0x96d1784359a27100),
        UINT64_C(0x19b9e91b09fcea8b), UINT64_C(0x636199d339c963f2),
        UINT64_C(0xdf7adabd7a6e2d6f), UINT64_C(0xa5a2aa754a5ba416),
        UINT64_C(0x2aca3b2d1a053f9d), UINT64_C(0x50124be52a30b6e4),
        UINT64_C(0x1f423fcee22f9be0), UINT64_C(0x659a4f06d21a1299),
        UINT64_C(0xeaf2de5e82448912), UINT64_C(0x902aae96b271006b),
        UINT64_C(0x74523609127ad31a), UINT64_C(0x0e8a46c1224f5a63),
        UINT64_C(0x81e2d7997211c1e8), UINT64_C(0xfb3aa75142244891),
        UINT64_C(0xb46ad37a8a3b6595), UINT64_C(0xceb2a3b2ba0eecec),
        UINT64_C(0x41da32eaea507767), UINT64_C(0x3b024222da65fe1e),
        UINT64_C(0xa2722586f2d042ee), UINT64_C(0xd8aa554ec2e5cb97),
        UINT64_C(0x57c2c41692bb501c), UINT64_C(0x2d1ab4dea28ed965),
        UINT64_C(0x624ac0f56a91f461), UINT64_C(0x1892b03d5aa47d18),
        UINT64_C(0x97fa21650afae693), UINT64_C(0xed2251ad3acf6fea),
        UINT64_C(0x095ac9329ac4bc9b), UINT64_C(0x7382b9faaaf135e2),
        UINT64_C(0xfcea28a2faafae69), UINT64_C(0x8632586aca9a2710),
        UINT64_C(0xc9622c4102850a14), UINT64_C(0xb3ba5c8932b0836d),
        UINT64_C(0x3cd2cdd162ee18e6), UINT64_C(0x460abd1952db919f),
        UINT64_C(0x256b24ca6b12f26d), UINT64_C(0x5fb354025b277b14),
        UINT64_C(0xd0dbc55a0b79e09f), UINT64_C(0xaa03b5923b4c69e6),
        UINT64_C(0xe553c1b9f35344e2), UINT64_C(0x9f8bb171c366cd9b),
        UINT64_C(0x10e3202993385610), UINT64_C(0x6a3b50e1a30ddf69),
        UINT64_C(0x8e43c87e03060c18), UINT64_C(0xf49bb8b633338561),
        UINT64_C(0x7bf329ee636d1eea), UINT64_C(0x012b592653589793),
        UINT64_C(0x4e7b2d0d9b47ba97), UINT64_C(0x34a35dc5ab7233ee),
        UINT64_C(0xbbcbcc9dfb2ca865), UINT64_C(0xc113bc55cb19211c),
        UINT64_C(0x5863dbf1e3ac9dec), UINT64_C(0x22bbab39d3991495),
        UINT64_C(0xadd33a6183c78f1e), UINT64_C(0xd70b4aa9b3f20667),
        UINT64_C(0x985b3e827bed2b63), UINT64_C(0xe2834e4a4bd8a21a),
        UINT64_C(0x6debdf121b863991), UINT64_C(0x1733afda2bb3b0e8),
        UINT64_C(0xf34b37458bb86399), UINT64_C(0x8993478dbb8deae0),
        UINT64_C(0x06fbd6d5ebd3716b), UINT64_C(0x7c23a61ddbe6f812),
        UINT64_C(0x3373d23613f9d516), UINT64_C(0x49aba2fe23cc5c6f),
        UINT64_C(0xc6c333a67392c7e4), UINT64_C(0xbc1b436e43a74e9d),
        UINT64_C(0x95ac9329ac4bc9b5), UINT64_C(0xef74e3e19c7e40cc),
        UINT64_C(0x601c72b9cc20db47), UINT64_C(0x1ac40271fc15523e),
        UINT64_C(0x5594765a340a7f3a), UINT64_C(0x2f4c0692043ff643),
        UINT64_C(0xa02497ca54616dc8), UINT64_C(0xdafce7026454e4b1),
        UINT64_C(0x3e847f9dc45f37c0), UINT64_C(0x445c0f55f46abeb9),
        UINT64_C(0xcb349e0da4342532), UINT64_C(0xb1eceec59401ac4b),
        UINT64_C(0xfebc9aee5c1e814f), UINT64_C(0x8464ea266c2b0836),
        UINT64_C(0x0b0c7b7e3c7593bd), UINT64_C(0x71d40bb60c401ac4),
        UINT64_C(0xe8a46c1224f5a634), UINT64_C(0x927c1cda14c02f4d),
        UINT64_C(0x1d148d82449eb4c6), UINT64_C(0x67ccfd4a74ab3dbf),
        UINT64_C(0x289c8961bcb410bb), UINT64_C(0x5244f9a98c8199c2),
        UINT64_C(0xdd2c68f1dcdf0249), UINT64_C(0xa7f41839ecea8b30),
        UINT64_C(0x438c80a64ce15841), UINT64_C(0x3954f06e7cd4d138),
        UINT64_C(0xb63c61362c8a4ab3), UINT64_C(0xcce411fe1cbfc3ca),
        UINT64_C(0x83b465d5d4a0eece), UINT64_C(0xf96c151de49567b7),
        UINT64_C(0x76048445b4cbfc3c), UINT64_C(0x0cdcf48d84fe7545),
        UINT64_C(0x6fbd6d5ebd3716b7), UINT64_C(0x15651d968d029fce),
        UINT64_C(0x9a0d8ccedd5c0445), UINT64_C(0xe0d5fc06ed698d3c),
        UINT64_C(0xaf85882d2576a038), UINT64_C(0xd55df8e515432941),
        UINT64_C(0x5a3569bd451db2ca), UINT64_C(0x20ed197575283bb3),
        UINT64_C(0xc49581ead523e8c2), UINT64_C(0xbe4df122e51661bb),
        UINT64_C(0x3125607ab548fa30), UINT64_C(0x4bfd10b2857d7349),
        UINT64_C(0x04ad64994d625e4d), UINT64_C(0x7e7514517d57d734),
        UINT64_C(0xf11d85092d094cbf), UINT64_C(0x8bc5f5c11d3cc5c6),
        UINT64_C(0x12b5926535897936), UINT64_C(0x686de2ad05bcf04f),
        UINT64_C(0xe70573f555e26bc4), UINT64_C(0x9ddd033d65d7e2bd),
        UINT64_C(0xd28d7716adc8cfb9), UINT64_C(0xa85507de9dfd46c0),
        UINT64_C(0x273d9686cda3dd4b), UINT64_C(0x5de5e64efd965432),
        UINT64_C(0xb99d7ed15d9d8743), UINT64_C(0xc3450e196da80e3a),
        UINT64_C(0x4c2d9f413df695b1), UINT64_C(0x36f5ef890dc31cc8),
        UINT64_C(0x79a59ba2c5dc31cc), UINT64_C(0x037deb6af5e9b8b5),
        UINT64_C(0x8c157a32a5b7233e), UINT64_C(0xf6cd0afa9582aa47),
        UINT64_C(0x4ad64994d625e4da), UINT64_C(0x300e395ce6106da3),
        UINT64_C(0xbf66a804b64ef628), UINT64_C(0xc5bed8cc867b7f51),
        UINT64_C(0x8aeeace74e645255), UINT64_C(0xf036dc2f7e51db2c),
        UINT64_C(0x7f5e4d772e0f40a7), UINT64_C(0x05863dbf1e3ac9de),
        UINT64_C(0xe1fea520be311aaf), UINT64_C(0x9b26d5e88e0493d6),
        UINT64_C(0x144e44b0de5a085d), UINT64_C(0x6e963478ee6f8124),
        UINT64_C(0x21c640532670ac20), UINT64_C(0x5b1e309b16452559),
        UINT64_C(0xd476a1c3461bbed2), UINT64_C(0xaeaed10b762e37ab),
        UINT64_C(0x37deb6af5e9b8b5b), UINT64_C(0x4d06c6676eae0222),
        UINT64_C(0xc26e573f3ef099a9), UINT64_C(0xb8b627f70ec510d0),
        UINT64_C(0xf7e653dcc6da3dd4), UINT64_C(0x8d3e2314f6efb4ad),
        UINT64_C(0x0256b24ca6b12f26), UINT64_C(0x788ec2849684a65f),
        UINT64_C(0x9cf65a1b368f752e), UINT64_C(0xe62e2ad306bafc57),
        UINT64_C(0x6946bb8b56e467dc), UINT64_C(0x139ecb4366d1eea5),
        UINT64_C(0x5ccebf68aecec3a1), UINT64_C(0x2616cfa09efb4ad8),
        UINT64_C(0xa97e5ef8cea5d153), UINT64_C(0xd3a62e30fe90582a),
        UINT64_C(0xb0c7b7e3c7593bd8), UINT64_C(0xca1fc72bf76cb2a1),
        UINT64_C(0x45775673a732292a), UINT64_C(0x3faf26bb9707a053),
        UINT64_C(0x70ff52905f188d57), UINT64_C(0x0a2722586f2d042e),
        UINT64_C(0x854fb3003f739fa5), UINT64_C(0xff97c3c80f4616dc),
        UINT64_C(0x1bef5b57af4dc5ad), UINT64_C(0x61372b9f9f784cd4),
        UINT64_C(0xee5fbac7cf26d75f), UINT64_C(0x9487ca0fff135e26),
        UINT64_C(0xdbd7be24370c7322), UINT64_C(0xa10fceec0739fa5b),
        UINT64_C(0x2e675fb4576761d0), UINT64_C(0x54bf2f7c6752e8a9),
        UINT64_C(0xcdcf48d84fe75459), UINT64_C(0xb71738107fd2dd20),
        UINT64_C(0x387fa9482f8c46ab), UINT64_C(0x42a7d9801fb9cfd2),
        UINT64_C(0x0df7adabd7a6e2d6), UINT64_C(0x772fdd63e7936baf),
        UINT64_C(0xf8474c3bb7cdf024), UINT64_C(0x829f3cf387f8795d),
        UINT64_C(0x66e7a46c27f3aa2c), UINT64_C(0x1c3fd4a417c62355),
        UINT64_C(0x935745fc4798b8de), UINT64_C(0xe98f353477ad31a7),
        UINT64_C(0xa6df411fbfb21ca3), UINT64_C(0xdc0731d78f8795da),
        UINT64_C(0x536fa08fdfd90e51), UINT64_C(0x29b7d047efec8728),
};
// clang-format on

uint64_t CRC::crc64(uint64_t crc, const char *s, uint64_t l) {
    uint64_t j;
    for (j = 0; j < l; j++) {
        uint8_t byte = s[j];
        crc = crc64_tab[(uint8_t)crc ^ byte] ^ (crc >> 8);
    }
    return crc;
}

} // namespace tair::common
