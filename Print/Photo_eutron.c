#include "TypeDef.h"

CONST struct GRAPH_DEFINE graph_info_eutron = {
		/*.cGrapCols =*/ 16,
		/*.cLabels =*/ 4,
		/*.cPrintAt =*/ 0,
		/*.cWidthAChar */ 12,
		/*.cHighAChar */ 32,
		/*.cByteALine =*/ 24,
		/*.cGrapSize =*/ 3072+25*4   //(cByteALine+1)*cLabels,
};

CONST BYTE photo_data_eutron[] = {
//图片自带的叠加文字区,共cLabels行,每行字符数为cByteALine+1,每行最后一个字节为有效打印的字符数目
"Very Good               \x00"\
"Very Good               \x00"\
"Very Good               \x00"\
"The Best, The EUTRON    \x18"\
//Bytes:3072
"\x00\x00\x08\x70\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\xF1\xFE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x03\xE3\xFC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x0F\xC7\xF8\xE0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x3F\x8F\xF1\xF0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x7F\x1F\xE3\xF8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\xFE\x3F\xC7\xF8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x01\xFC\x7F\x8F\xF8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x03\xF8\xFF\x1F\xF1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x07\xF1\xFE\x3F\xE3\x80\x01\xFF\xFE\x1F\xF8\x3E\xBF\xFF\xF8\xFF\xF8\x00\x03\xFE\x00\xFE\x01\xF0"\
"\x07\xE3\xFC\x7F\xC7\xC0\x00\xFC\x1E\x07\xE0\x0C\x3C\x7E\x38\x3F\x1F\x00\x0F\x07\x80\x7F\x00\x60"\
"\x0F\xC7\xF8\xFF\x8F\xC0\x00\x7C\x06\x03\xE0\x04\x30\x7C\x18\x3E\x07\x80\x1E\x03\xC0\x3F\x00\x40"\
"\x0F\x8F\xF1\xFF\x1F\xE0\x00\x7C\x02\x03\xE0\x04\x30\x7C\x08\x3E\x07\xC0\x3C\x01\xE0\x3F\x80\x40"\
"\x1F\x1F\xE3\xFE\x3F\xC0\x00\x7C\x02\x03\xE0\x04\x20\x7C\x08\x3E\x07\xE0\x7C\x01\xF0\x3F\xC0\x40"\
"\x1E\x3F\xC7\xFC\x7F\x80\x00\x7C\x02\x03\xE0\x04\x00\x7C\x00\x3E\x03\xE0\xF8\x00\xF8\x2F\xE0\x40"\
"\x3C\x7F\x8F\xF8\xFF\x10\x00\x7C\x20\x03\xE0\x04\x00\x7C\x00\x3E\x03\xE0\xF8\x00\xF8\x27\xF0\x40"\
"\x38\xFF\x1F\xF1\xFE\x30\x00\x7C\x20\x03\xE0\x04\x00\x7C\x00\x3E\x03\xE0\xF8\x00\xF8\x23\xF0\x40"\
"\x31\xFE\x3F\xE3\xFC\x78\x00\x7C\x60\x03\xE0\x04\x00\x7C\x00\x3E\x07\xC1\xF8\x00\xFC\x21\xF8\x40"\
"\x23\xFC\x7F\xC7\xF8\xF8\x00\x7C\x60\x03\xE0\x04\x00\x7C\x00\x3E\x07\x81\xF8\x00\xFC\x20\xFC\x40"\
"\x07\xF8\xFF\x8F\xF1\xF8\x00\x7F\xE0\x03\xE0\x04\x00\x7C\x00\x3E\x1F\x01\xF8\x00\xFC\x20\xFE\x40"\
"\x0F\xF1\xFF\x1F\xE3\xF8\x00\x7C\xE0\x03\xE0\x04\x00\x7C\x00\x3F\xFC\x01\xF8\x00\xFC\x20\x7E\x40"\
"\x1F\xE3\xFE\x3F\xC7\xF8\x00\x7C\x60\x03\xE0\x04\x00\x7C\x00\x3E\x7E\x01\xF8\x00\xFC\x20\x3F\x40"\
"\x3F\xC7\xFC\x7F\x8F\xF8\x00\x7C\x20\x03\xE0\x04\x00\x7C\x00\x3E\x3F\x00\xF8\x00\xFC\x20\x1F\xC0"\
"\x7F\x8F\xF8\xFF\x1F\xE0\x00\x7C\x20\x03\xE0\x04\x00\x7C\x00\x3E\x1F\x00\xF8\x00\xF8\x20\x0F\xC0"\
"\x7F\x1F\xF1\xFE\x3F\xC0\x00\x7C\x20\x03\xE0\x04\x00\x7C\x00\x3E\x1F\x80\xF8\x00\xF8\x20\x0F\xC0"\
"\x7E\x3F\xE3\xFC\x7F\x88\x00\x7C\x00\x83\xE0\x0C\x00\x7C\x00\x3E\x0F\xC0\x78\x00\xF0\x20\x07\xC0"\
"\x3C\x7F\xC7\xF8\xFF\x18\x00\x7C\x01\x03\xE0\x08\x00\x7C\x00\x3E\x07\xE0\x7C\x01\xF0\x20\x03\xC0"\
"\x38\xFF\x8F\xF1\xFE\x38\x00\x7C\x03\x03\xE0\x08\x00\x7C\x00\x3E\x07\xE0\x3C\x01\xE0\x20\x01\xC0"\
"\x31\xFF\x1F\xE3\xFC\x78\x00\x7C\x07\x01\xF0\x10\x00\x7C\x00\x3E\x03\xF0\x1E\x03\xC0\x20\x00\xC0"\
"\x23\xFE\x3F\xC7\xF8\xF0\x00\xFC\x1F\x00\xF8\x20\x00\x7E\x00\x3F\x01\xF0\x0F\x07\x80\x30\x00\xC0"\
"\x07\xFC\x7F\x8F\xF1\xF0\x01\xFF\xFF\x00\x7F\xC0\x02\xFF\x80\x7F\xC1\xF8\x03\xFE\x00\xFC\x00\x40"\
"\x0F\xF8\xFF\x1F\xE3\xF0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x1F\xF1\xFE\x3F\xC7\xE0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x0F\xE3\xFC\x7F\x8F\xE0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x0F\xC7\xF8\xFF\x1F\xC0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x07\x8F\xF1\xFE\x3F\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x03\x1F\xE3\xFC\x7F\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x3F\xC7\xF8\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x7F\x8F\xF1\xFE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x7F\x1F\xE3\xFC\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x3E\x3F\xC7\xF8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x1C\x7F\x8F\xF0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\xFF\x1F\xC0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x01\xFE\x3F\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x7C\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0C\x00\x00\x00\x00\x00\x00\x20\x00\x04\x00\x00\x00\x00"\
"\x00\x00\x04\x00\x00\x80\x00\x00\x00\x00\x3C\x00\x00\x21\x00\x00\x60\x38\x00\x06\x00\x00\x08\x00"\
"\x00\x0C\x07\x00\x00\xC0\x00\xC4\x00\x01\x60\x00\x00\x73\x00\x00\x60\x10\x00\x06\x00\x06\x0C\x00"\
"\x00\x0C\x03\x00\x00\xC0\x00\xC6\x00\x00\x20\x00\x00\x63\x38\x00\xE0\x10\x0C\x24\x00\x03\x0C\x00"\
"\x00\x1C\x02\x01\x8C\x80\x00\xC6\x60\x00\x3F\x80\x00\x63\x18\x00\xF9\x10\x0C\x6F\x80\x00\x0B\x80"\
"\x00\x1F\x22\x01\x8C\xF0\x01\x86\x30\x03\xF0\x00\x00\xC3\x00\x01\x9D\x90\x00\x7E\x00\x03\xC8\xC0"\
"\x00\x33\xB2\x00\x0F\xC0\x03\x06\x00\x0C\xFC\x00\x01\x83\x30\x03\x05\x90\x00\x44\x00\xFE\x18\x00"\
"\x00\x60\xB2\x00\x0C\x80\x03\x04\x60\x01\xE7\x00\x01\xC3\xF0\x02\x01\x90\x00\x87\xE0\x00\x1F\x00"\
"\x00\x40\x32\x00\x10\xBC\x07\x0F\xC0\x03\x23\xC0\x03\xDF\x00\x06\xF1\x90\x1C\xBE\x00\x0F\x78\x00"\
"\x00\xEF\x32\x03\x97\xE0\x07\x3C\x00\x06\x20\xF0\x06\xC2\x80\x0B\x31\x90\x3C\xC0\x00\x33\x1C\x00"\
"\x01\x66\x32\x0F\x98\x00\x0D\x0F\x00\x18\x3C\x00\x0C\xC6\xC0\x13\x31\x98\x08\x0F\x80\x33\x14\x00"\
"\x02\x66\x33\x01\x01\xF0\x11\x09\x00\x03\xCC\x00\x08\xC6\x80\x01\x31\x98\x08\x31\x80\x1C\x34\x00"\
"\x00\x26\x33\x01\x06\x30\x21\x1B\x00\x00\xC8\x00\x00\xCC\x80\x03\x71\x98\x08\x21\x00\x0C\x34\x00"\
"\x00\x6E\x33\x01\x04\x20\x01\x1B\x00\x00\xDA\x00\x00\xCC\x82\x01\x24\x18\x0C\x3F\x00\x47\xEC\x20"\
"\x00\x24\x83\x01\x87\xE0\x01\x33\x04\x01\x9F\x80\x00\xD8\x83\x01\x04\x18\x04\x30\x00\x44\xEC\x20"\
"\x00\x20\x83\x00\x86\x00\x01\x23\x04\x01\x03\x00\x00\xB0\x83\x01\xCC\x18\x7F\xC0\x00\xCC\xCC\x20"\
"\x00\x31\x83\x0F\xF4\x00\x03\x43\x0C\x03\x03\x00\x00\xE0\xFF\x00\xF8\xD8\x00\x7E\x78\x0D\x84\x70"\
"\x00\x1F\x0F\x00\x0F\xCF\x83\x81\xFC\x06\x03\x00\x00\xC0\x3C\x00\x00\x70\x00\x0F\xF8\x09\x07\xE0"\
"\x00\x00\x0E\x00\x01\xFE\x03\x00\xF0\x0C\x1E\x00\x00\x00\x00\x00\x00\x30\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x06\x00\x00\x00\x00\x00\x00\x30\x0E\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0C\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

};
