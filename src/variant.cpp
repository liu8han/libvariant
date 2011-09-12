/* 
 *  Copyright (c) 2011,
 *  Han Liu (cn.liuhan@gmail.com)
 *
 *  This file is changed from crtmpserver (http://www.rtmpd.com/)
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "defines.h"
#include "variant.h"
#include "variantmap.h"
#include <openssl/bio.h>
#include <openssl/engine.h>

#define TIXML_USE_STL
#include "../tinyxml/tinyxml.h"

string b64(string source);
string b64(uint8_t *pBuffer, uint32_t length);
string unb64(string source);
string unb64(uint8_t *pBuffer, uint32_t length);

string b64(string source) {
	return b64((uint8_t *) STR(source), source.size());
}

string b64(uint8_t *pBuffer, uint32_t length) {
	BIO *bmem;
	BIO *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());

	b64 = BIO_push(b64, bmem);
	BIO_write(b64, pBuffer, length);
	string result = "";
	if (BIO_flush(b64) == 1) {
		BIO_get_mem_ptr(b64, &bptr);
		result = string(bptr->data, bptr->length);
	}

	BIO_free_all(b64);

	replace(result, "\n", "");
	replace(result, "\r", "");

	return result;
}

string unb64(string source) {
	return unb64((uint8_t *) STR(source), source.length());
}

string unb64(uint8_t *pBuffer, uint32_t length) {
	// create a memory buffer containing base64 encoded data
	BIO* bmem = BIO_new_mem_buf((void *) pBuffer, length);

	// push a Base64 filter so that reading from buffer decodes it
	BIO *bioCmd = BIO_new(BIO_f_base64());
	// we don't want newlines
	BIO_set_flags(bioCmd, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_push(bioCmd, bmem);

	char *pOut = new char[length];

	int finalLen = BIO_read(bmem, (void*) pOut, length);
	BIO_free_all(bmem);
	string result(pOut, finalLen);
	delete[] pOut;
	return result;
}

Variant::Variant() {
	CONSTRUCTOR;
	_type = V_NULL;
	memset(&_value, 0, sizeof (_value));
}

Variant::Variant(const Variant &val) {
	CONSTRUCTOR;
	InternalCopy(val);
}

Variant::Variant(const bool &val) {
	CONSTRUCTOR;
	_type = V_BOOL;
	memset(&_value, 0, sizeof (_value));
	_value.b = val;
}

Variant::Variant(const int8_t &val) {
	CONSTRUCTOR;
	_type = V_INT8;
	memset(&_value, 0, sizeof (_value));
	_value.i8 = val;
}

Variant::Variant(const int16_t &val) {
	CONSTRUCTOR;
	_type = V_INT16;
	memset(&_value, 0, sizeof (_value));
	_value.i16 = val;
}

Variant::Variant(const int32_t &val) {
	CONSTRUCTOR;
	_type = V_INT32;
	memset(&_value, 0, sizeof (_value));
	_value.i32 = val;
}

Variant::Variant(const int64_t &val) {
	CONSTRUCTOR;
	_type = V_INT64;
	memset(&_value, 0, sizeof (_value));
	_value.i64 = val;
}

Variant::Variant(const uint8_t &val) {
	CONSTRUCTOR;
	_type = V_UINT8;
	memset(&_value, 0, sizeof (_value));
	_value.ui8 = val;
}

Variant::Variant(const uint16_t &val) {
	CONSTRUCTOR;
	_type = V_UINT16;
	memset(&_value, 0, sizeof (_value));
	_value.ui16 = val;
}

Variant::Variant(const uint32_t &val) {
	CONSTRUCTOR;
	_type = V_UINT32;
	memset(&_value, 0, sizeof (_value));
	_value.ui32 = val;
}

Variant::Variant(const uint64_t &val) {
	CONSTRUCTOR;
	_type = V_UINT64;
	memset(&_value, 0, sizeof (_value));
	_value.ui64 = val;
}

Variant::Variant(const double &val) {
	CONSTRUCTOR;
	_type = V_DOUBLE;
	memset(&_value, 0, sizeof (_value));
	_value.d = val;
}

Variant::Variant(const Timestamp &val) {
	CONSTRUCTOR;
	_type = V_TIMESTAMP;
	memset(&_value, 0, sizeof (_value));
	DYNAMIC_ALLOC("_value.t");
	_value.t = new Timestamp;
	*_value.t = val;
	NormalizeTs();
}

Variant::Variant(const uint16_t year, const uint8_t month, const uint8_t day) {
	CONSTRUCTOR;
	_type = V_DATE;
	memset(&_value, 0, sizeof (_value));
	DYNAMIC_ALLOC("_value.t");
	_value.t = new Timestamp;
	memset(_value.t, 0, sizeof (Timestamp));
	_value.t->tm_year = year - 1900;
	_value.t->tm_mon = month - 1;
	_value.t->tm_mday = day;
	_value.t->tm_hour = 0;
	_value.t->tm_min = 0;
	_value.t->tm_sec = 0;
	NormalizeTs();
}

Variant::Variant(const uint8_t hour, const uint8_t min, const uint8_t sec, const uint16_t m) {
	CONSTRUCTOR;
	_type = V_TIME;
	memset(&_value, 0, sizeof (_value));
	DYNAMIC_ALLOC("_value.t");
	_value.t = new Timestamp;
	memset(_value.t, 0, sizeof (Timestamp));
	_value.t->tm_year = 70;
	_value.t->tm_mon = 0;
	_value.t->tm_mday = 1;
	_value.t->tm_hour = hour;
	_value.t->tm_min = min;
	_value.t->tm_sec = sec;
	NormalizeTs();
}

Variant::Variant(const uint16_t year, const uint8_t month, const uint8_t day,
		const uint8_t hour, const uint8_t min, const uint8_t sec, const uint16_t m) {
	CONSTRUCTOR;
	_type = V_TIMESTAMP;
	memset(&_value, 0, sizeof (_value));
	DYNAMIC_ALLOC("_value.t");
	_value.t = new Timestamp;
	memset(_value.t, 0, sizeof (Timestamp));
	_value.t->tm_year = year - 1900;
	_value.t->tm_mon = month - 1;
	_value.t->tm_mday = day;
	_value.t->tm_hour = hour;
	_value.t->tm_min = min;
	_value.t->tm_sec = sec;
	NormalizeTs();
}

Variant::Variant(const char *pVal) {
	CONSTRUCTOR;
	_type = V_STRING;
	memset(&_value, 0, sizeof (_value));
	DYNAMIC_ALLOC("_value.s");
	_value.s = new string(pVal);
}

Variant::Variant(const string &val) {
	CONSTRUCTOR;
	_type = V_STRING;
	memset(&_value, 0, sizeof (_value));
	DYNAMIC_ALLOC("_value.s");
	_value.s = new string(val);
}

Variant::~Variant() {
	DESTRUCTOR;
	Reset();
}

void Variant::Reset(bool isUndefined) {
	switch (_type) {
		case V_DATE:
		case V_TIME:
		case V_TIMESTAMP:
		{
			DYNAMIC_FREE("_value.t");
			delete _value.t;
			break;
		}
		case V_BYTEARRAY:
		case V_STRING:
		{
			DYNAMIC_FREE("_value.s");
			delete _value.s;
			break;
		}
		case V_MAP:
		case V_TYPED_MAP:
		{
			DYNAMIC_FREE("_value.m");
			delete _value.m;
			break;
		}
		default:
		{
			break;
		}
	}
	if (isUndefined)
		_type = V_UNDEFINED;
	else
		_type = V_NULL;
	memset(&_value, 0, sizeof (_value));
}

string Variant::ToString(string name, uint32_t indent) {
	string result = "";
	string strIndent = string(indent * 4, ' ');
	switch (_type) {
		case V_NULL:
		{
			result += format("%s<NULL name=\"%s\"></NULL>",
					STR(strIndent), STR(name));
			break;
		}
		case V_UNDEFINED:
		{
			result += format("%s<UNDEFINED name=\"%s\"></UNDEFINED>",
					STR(strIndent), STR(name));
			break;
		}
		case V_BOOL:
		{
			result += format("%s<BOOL name=\"%s\">%s</BOOL>",
					STR(strIndent), STR(name), _value.b ? "true" : "false");
			break;
		}
		case V_INT8:
		{
			result += format("%s<INT8 name=\"%s\">%hhd</INT8>",
					STR(strIndent), STR(name), _value.i8);
			break;
		}
		case V_INT16:
		{
			result += format("%s<INT16 name=\"%s\">%hd</INT16>",
					STR(strIndent), STR(name), _value.i16);
			break;
		}
		case V_INT32:
		{
			result += format("%s<INT32 name=\"%s\">%d</INT32>",
					STR(strIndent), STR(name), _value.i32);
			break;
		}
		case V_INT64:
		{
			result += format("%s<INT64 name=\"%s\">%"PRId64"</INT64>",
					STR(strIndent), STR(name), _value.i64);
			break;
		}
		case V_UINT8:
		{
			result += format("%s<UINT8 name=\"%s\">%hhu</UINT8>",
					STR(strIndent), STR(name), _value.ui8);
			break;
		}
		case V_UINT16:
		{
			result += format("%s<UINT16 name=\"%s\">%hu</UINT16>",
					STR(strIndent), STR(name), _value.ui16);
			break;
		}
		case V_UINT32:
		{
			result += format("%s<UINT32 name=\"%s\">%u</UINT32>",
					STR(strIndent), STR(name), _value.ui32);
			break;
		}
		case V_UINT64:
		{
			result += format("%s<UINT64 name=\"%s\">%"PRIu64"</UINT64>",
					STR(strIndent), STR(name), _value.ui64);
			break;
		}
		case V_DOUBLE:
		{
			result += format("%s<DOUBLE name=\"%s\">%.03f</DOUBLE>",
					STR(strIndent), STR(name), _value.d);
			break;
		}
		case V_TIMESTAMP:
		{
			result += format("%s<TIMESTAMP name=\"%s\">%s</TIMESTAMP>",
					STR(strIndent), STR(name), STR(*this));
			break;
		}
		case V_DATE:
		{
			result += format("%s<DATE name=\"%s\">%s</DATE>",
					STR(strIndent), STR(name), STR(*this));
			break;
		}
		case V_TIME:
		{
			result += format("%s<TIME name=\"%s\">%s</TIME>",
					STR(strIndent), STR(name), STR(*this));
			break;
		}
		case V_STRING:
		{
			result += format("%s<STR name=\"%s\">%s</STR>",
					STR(strIndent), STR(name), STR(*_value.s));
			break;
		}
		case V_BYTEARRAY:
		{
			result += format("%s<BYTEARRAY name=\"%s\">%"PRIz"u bytes</BYTEARRAY>",
					STR(strIndent), STR(name), _value.s->length());
			break;
		}
		case V_TYPED_MAP:
		{
			result += format("%s<TYPED_MAP name=\"%s\" typename=\"%s\" isArray=\"%s\">\n",
					STR(strIndent), STR(name), STR(_value.m->typeName),
					_value.m->isArray ? "true" : "false");

			FOR_MAP(_value.m->children, string, Variant, i) {
				result += MAP_VAL(i).ToString(MAP_KEY(i), indent + 1) + "\n";
			}
			result += strIndent + "</TYPED_MAP>";
			break;
		}
		case V_MAP:
		{
			result += format("%s<MAP name=\"%s\" isArray=\"%s\">\n",
					STR(strIndent), STR(name),
					_value.m->isArray ? "true" : "false");

			FOR_MAP(_value.m->children, string, Variant, i) {
				result += MAP_VAL(i).ToString(MAP_KEY(i), indent + 1) + "\n";
			}
			result += strIndent + "</MAP>";
			break;
		}
		default:
		{
			assert(false);
		}
	}
	return result;
}

Variant& Variant::operator=(const Variant &val) {
	Reset();
	InternalCopy(val);
	return *this;
}

Variant& Variant::operator=(const bool &val) {
	Reset();
	_type = V_BOOL;
	_value.b = val;
	return *this;
}

Variant& Variant::operator=(const int8_t &val) {
	Reset();
	_type = V_INT8;
	_value.i8 = val;
	return *this;
}

Variant& Variant::operator=(const int16_t &val) {
	Reset();
	_type = V_INT16;
	_value.i16 = val;
	return *this;
}

Variant& Variant::operator=(const int32_t &val) {
	Reset();
	_type = V_INT32;
	_value.i32 = val;
	return *this;
}

Variant& Variant::operator=(const int64_t &val) {
	Reset();
	_type = V_INT64;
	_value.i64 = val;
	return *this;
}

Variant& Variant::operator=(const uint8_t &val) {
	Reset();
	_type = V_UINT8;
	_value.ui8 = val;
	return *this;
}

Variant& Variant::operator=(const uint16_t &val) {
	Reset();
	_type = V_UINT16;
	_value.ui16 = val;
	return *this;
}

Variant& Variant::operator=(const uint32_t &val) {
	Reset();
	_type = V_UINT32;
	_value.ui32 = val;
	return *this;
}

Variant& Variant::operator=(const uint64_t &val) {
	Reset();
	_type = V_UINT64;
	_value.ui64 = val;
	return *this;
}

Variant& Variant::operator=(const double &val) {
	Reset();
	_type = V_DOUBLE;
	_value.d = val;
	return *this;
}

Variant& Variant::operator=(const Timestamp &val) {
	Reset();
	_type = V_TIMESTAMP;
	DYNAMIC_ALLOC("_value.t");
	_value.t = new Timestamp;
	*_value.t = val;
	NormalizeTs();
	return *this;
}

Variant& Variant::operator=(const char *pVal) {
	Reset();
	_type = V_STRING;
	DYNAMIC_ALLOC("_value.s");
	_value.s = new string(pVal);
	return *this;
}

Variant& Variant::operator=(const string &val) {
	Reset();
	_type = V_STRING;
	DYNAMIC_ALLOC("_value.s");
	_value.s = new string(val);
	return *this;
}

Variant::operator VariantType() {
	return _type;
}

Variant::operator bool() {
	switch (_type) {
		case V_NULL:
		case V_UNDEFINED:
		{
			return false;
		}
		case V_BOOL:
		{
			return _value.b;
		}
		case V_INT8:
		case V_INT16:
		case V_INT32:
		case V_INT64:
		case V_UINT8:
		case V_UINT16:
		case V_UINT32:
		case V_UINT64:
		case V_DOUBLE:
		{
			bool result = false;
			result |= (_value.i8 != 0);
			result |= (_value.i16 != 0);
			result |= (_value.i32 != 0);
			result |= (_value.i64 != 0);
			result |= (_value.ui8 != 0);
			result |= (_value.ui16 != 0);
			result |= (_value.ui32 != 0);
			result |= (_value.ui64 != 0);
			return result;
		}
		case V_TIMESTAMP:
		case V_DATE:
		case V_TIME:
		case V_STRING:
		case V_TYPED_MAP:
		case V_MAP:
		default:
		{
			return false;
		}
	}
}

#define OPERATOR_DEF(ctype) \
Variant::operator ctype() {\
    switch (_type) {\
        case V_NULL:\
        case V_UNDEFINED:\
        {\
            return 0;\
        }\
        case V_BOOL:\
        {\
            return (ctype) _value.b;\
        }\
        case V_INT8:\
        {\
            return (ctype) _value.i8;\
        }\
        case V_INT16:\
        {\
            return (ctype) _value.i16;\
        }\
        case V_INT32:\
        {\
            return (ctype) _value.i32;\
        }\
        case V_INT64:\
        {\
            return (ctype) _value.i64;\
        }\
        case V_UINT8:\
        {\
            return (ctype) _value.ui8;\
        }\
        case V_UINT16:\
        {\
            return (ctype) _value.ui16;\
        }\
        case V_UINT32:\
        {\
            return (ctype) _value.ui32;\
        }\
        case V_UINT64:\
        {\
            return (ctype) _value.ui64;\
        }\
        case V_DOUBLE:\
        {\
            return (ctype) _value.d;\
        }\
        case V_TIMESTAMP:\
        case V_DATE:\
        case V_TIME:\
        case V_STRING:\
        case V_TYPED_MAP:\
        case V_MAP:\
        default:\
        {\
            return 0;\
        }\
    }\
}

OPERATOR_DEF(int8_t);
OPERATOR_DEF(int16_t);
OPERATOR_DEF(int32_t);
OPERATOR_DEF(int64_t);
OPERATOR_DEF(uint8_t);
OPERATOR_DEF(uint16_t);
OPERATOR_DEF(uint32_t);
OPERATOR_DEF(uint64_t);
OPERATOR_DEF(double);

Variant::operator Timestamp() {
	if (_type == V_DATE ||
			_type == V_TIME ||
			_type == V_TIMESTAMP) {
		return *_value.t;
	} else {
		Timestamp temp = Timestamp_init;
		return temp;
	}
}

Variant::operator string() {
	switch (_type) {
		case V_BOOL:
		{
			return _value.b ? "true" : "false";
		}
		case V_INT8:
		case V_INT16:
		case V_INT32:
		{
			return format("%d", this->operator int32_t());
		}
		case V_INT64:
		{
			return format("%"PRId64, this->operator int64_t());
		}
		case V_UINT8:
		case V_UINT16:
		case V_UINT32:
		{
			return format("%u", this->operator uint32_t());
		}
		case V_UINT64:
		{
			return format("%"PRIu64, this->operator uint64_t());
		}
		case V_DOUBLE:
		{
			return format("%.3f", this->operator double());
		}
		case V_TIMESTAMP:
		{
			char tempBuff[24] = {0};
			return string(tempBuff, strftime(tempBuff, 24, "%Y-%m-%dT%H:%M:%S.000", _value.t));
		}
		case V_DATE:
		{
			char tempBuff[24] = {0};
			return string(tempBuff, strftime(tempBuff, 24, "%Y-%m-%d", _value.t));
		}
		case V_TIME:
		{
			char tempBuff[24] = {0};
			return string(tempBuff, strftime(tempBuff, 24, "%H:%M:%S.000", _value.t));
		}
		case V_BYTEARRAY:
		case V_STRING:
		{
			return *_value.s;
		}
		case V_NULL:
		case V_UNDEFINED:
		case V_TYPED_MAP:
		case V_MAP:
		default:
		{
			return "";
		}
	}
	return "";
}

Variant& Variant::operator[](const string &key) {
	if ((_type != V_TYPED_MAP) &&
			(_type != V_MAP) &&
			(_type != V_NULL) &&
			(_type != V_UNDEFINED)) {
	}
	if ((_type == V_NULL) || (_type == V_UNDEFINED)) {
		_type = V_MAP;
		DYNAMIC_ALLOC("_value.m");
		_value.m = new VariantMap;
	}
	if (!MAP_HAS1(_value.m->children, key)) {
		_value.m->children[key] = Variant();
	}
	return _value.m->children[key];
}

Variant& Variant::operator[](const char *key) {
	return operator[](string(key));
}

Variant& Variant::operator[](const double &key) {
	stringstream ss;
	ss << VAR_INDEX_VALUE << key;
	return operator[](ss.str());
}

Variant& Variant::operator[](const uint32_t &key) {
	stringstream ss;
	ss << VAR_INDEX_VALUE << key;
	return operator[](ss.str());
}

Variant& Variant::operator[](Variant &key) {
	stringstream ss;
	switch (key._type) {
		case V_BOOL:
		case V_INT8:
		case V_INT16:
		case V_INT32:
		case V_INT64:
		case V_UINT8:
		case V_UINT16:
		case V_UINT32:
		case V_UINT64:
		case V_DOUBLE:
		{
			ss << VAR_INDEX_VALUE << STR(key);
			break;
		}
		case V_STRING:
		{
			ss << *key._value.s;
			break;
		}
		case V_NULL:
		case V_UNDEFINED:
		case V_DATE:
		case V_TIME:
		case V_TIMESTAMP:
		case V_MAP:
		case V_TYPED_MAP:
		default:
		{
			break;
		}
	}
	return operator[](ss.str());
}

Variant &Variant::GetValue(string key, bool caseSensitive) {
	if (caseSensitive) {
		return (*this)[key];
	} else {

		FOR_MAP(*this, string, Variant, i) {
			if (lowerCase(MAP_KEY(i)) == lowerCase(key))
				return MAP_VAL(i);
		}

		return (*this)[key];
	}
}

bool Variant::operator==(Variant variant) {
	return ToString() == variant.ToString();
}

bool Variant::operator!=(Variant variant) {
	return !operator==(variant);
}

bool Variant::operator==(VariantType type) {
	if (type == _V_NUMERIC)
		return _type == V_INT8 ||
			_type == V_INT8 ||
			_type == V_INT16 ||
			_type == V_INT32 ||
			_type == V_INT64 ||
			_type == V_UINT8 ||
			_type == V_UINT16 ||
			_type == V_UINT32 ||
			_type == V_UINT64 ||
			_type == V_DOUBLE;
	else
		return _type == type;
}

bool Variant::operator!=(VariantType type) {
	return !operator ==(type);
}

string Variant::GetTypeName() {
	if (_type != V_TYPED_MAP) {
		return "";
	}
	return _value.m->typeName;
}

void Variant::SetTypeName(string name) {
	if ((_type != V_TYPED_MAP) && (_type != V_MAP) &&
			(_type != V_UNDEFINED) && (_type != V_NULL)) {
		return;
	}
	if (_type == V_UNDEFINED || _type == V_NULL) {
		DYNAMIC_ALLOC("_value.m");
		_value.m = new VariantMap;
	}
	_type = V_TYPED_MAP;
	_value.m->typeName = name;
}

bool Variant::HasKey(const string &key, bool caseSensitive) {
	if (_type != V_TYPED_MAP && _type != V_MAP) {
		return false;
	}
	if (caseSensitive) {
		return MAP_HAS1(_value.m->children, key);
	} else {

		FOR_MAP(*this, string, Variant, i) {
			if (lowerCase(MAP_KEY(i)) == lowerCase(key))
				return true;
		}
		return false;
	}
}

bool Variant::HasKeyChain(VariantType end, bool caseSensitive, uint32_t depth, ...) {
	va_list arguments;
	va_start(arguments, depth);
	Variant *pCurrent = this;
	for (uint8_t i = 0; i < depth; i++) {
		const char *pPathElement = va_arg(arguments, const char *);
		if (!pCurrent->HasKey(pPathElement, caseSensitive)) {
			va_end(arguments);
			return false;
		}
		Variant *pValue = &pCurrent->GetValue(pPathElement, caseSensitive);
		if (i == depth - 1) {
			va_end(arguments);
			return *pValue == end;
		} else {
			if ((*pValue != V_MAP) && (*pValue != V_TYPED_MAP)) {
				va_end(arguments);
				return false;
			}
		}
		pCurrent = pValue;
	}
	return false;
}

void Variant::RemoveKey(const string &key) {
	if (_type != V_TYPED_MAP && _type != V_MAP) {
		return;
	}
	_value.m->children.erase(key);
}

void Variant::RemoveAt(const uint32_t index) {
	if (_type != V_TYPED_MAP && _type != V_MAP) {

		return;
	}
	_value.m->children.erase(format(VAR_INDEX_VALUE"%u", index));
}

void Variant::RemoveAllKeys() {
	if (_type != V_TYPED_MAP && _type != V_MAP) {
		return;
	}
	_value.m->children.clear();
}

uint32_t Variant::MapSize() {
	if (_type == V_NULL || _type == V_UNDEFINED)
		return 0;
	if (_type != V_TYPED_MAP && _type != V_MAP) {		
		return 0;
	}
	return (uint32_t) _value.m->children.size();
}

uint32_t Variant::MapDenseSize() {
	if (_type == V_NULL || _type == V_UNDEFINED)
		return 0;
	if (_type != V_TYPED_MAP && _type != V_MAP) {
		return 0;
	}

	uint32_t denseCount = 0;
	for (denseCount = 0; denseCount < MapSize(); denseCount++) {
		if (!MAP_HAS1(_value.m->children, format(VAR_INDEX_VALUE"%u", denseCount)))
			break;
	}

	return denseCount;
}

void Variant::PushToArray(Variant value) {
	if (_type != V_NULL && _type != V_MAP)
        {
            
        }
	IsArray(true);
	(*this)[(uint32_t)this->MapDenseSize()] = value;
}

map<string, Variant>::iterator Variant::begin() {
	if (_type != V_TYPED_MAP && _type != V_MAP) {	
		map<string, Variant> temp;
		return temp.begin();
	}
	return _value.m->children.begin();
}

map<string, Variant>::iterator Variant::end() {
	if (_type != V_TYPED_MAP && _type != V_MAP) {
		map<string, Variant> temp;
		return temp.end();
	}
	return _value.m->children.end();
}

bool Variant::IsTimestamp(VariantType &type) {
	Variant &temp = *this;

	if ((VariantType) temp != V_MAP)
		return false;

	bool hasDate = temp.HasKey("year") && temp.HasKey("month") && temp.HasKey("day");
	bool hasLongTime = temp.HasKey("hour") && temp.HasKey("min") && temp.HasKey("sec");
	bool hasShortTime = false;
	if (!hasLongTime)
		hasShortTime = temp.HasKey("hour") && temp.HasKey("min");
	bool hasIsdst = temp.HasKey("isdst");
	bool hasType = temp.HasKey("type");

	if (hasDate) {
		hasDate = hasDate && (temp["year"] == _V_NUMERIC);
		hasDate = hasDate && (temp["month"] == _V_NUMERIC);
		hasDate = hasDate && (temp["day"] == _V_NUMERIC);
	}

	if (hasLongTime) {
		hasLongTime = hasLongTime && (temp["hour"] == _V_NUMERIC);
		hasLongTime = hasLongTime && (temp["min"] == _V_NUMERIC);
		hasLongTime = hasLongTime && (temp["sec"] == _V_NUMERIC);
	} else if (hasShortTime) {
		hasShortTime = hasShortTime && (temp["hour"] == _V_NUMERIC);
		hasShortTime = hasShortTime && (temp["min"] == _V_NUMERIC);
	}
	bool hasTime = hasLongTime || hasShortTime;

	if (hasIsdst)
		hasIsdst = hasIsdst && (temp["isdst"] == V_BOOL);

	if ((!hasDate) && (!hasTime))
		return false;

	uint32_t size = 0;
	if (hasDate)
		size += 3;
	if (hasLongTime)
		size += 3;
	else if (hasShortTime)
		size += 2;
	if (hasType)
		size += 1;
	if (hasIsdst)
		size += 1;


	if (hasType) {
		if (temp["type"] == "date") {
			hasDate = true;
			hasTime = false;
		}
		if (temp["type"] == "time") {
			hasDate = false;
			hasTime = true;
		}
		if (temp["type"] == "timestamp") {
			hasDate = true;
			hasTime = true;
		}
	}

	if (hasDate && hasTime)
		type = V_TIMESTAMP;
	else if (hasDate)
		type = V_DATE;
	else
		type = V_TIME;

	return temp.MapSize() == size;
}

bool Variant::IsNumeric() {
	return _type == V_DOUBLE ||
			_type == V_INT16 ||
			_type == V_INT32 ||
			_type == V_INT64 ||
			_type == V_INT8 ||
			_type == V_UINT16 ||
			_type == V_UINT32 ||
			_type == V_UINT64 ||
			_type == V_UINT8;
}

bool Variant::IsArray() {
	if (_type == V_MAP)
		return _value.m->isArray;
	return false;
}

void Variant::IsArray(bool isArray) {
	if (_type == V_NULL) {
		_type = V_MAP;
		DYNAMIC_ALLOC("_value.m");
		_value.m = new VariantMap;
	}
	if (_type == V_MAP)
		_value.m->isArray = isArray;
}

bool Variant::IsByteArray() {
	return _type == V_BYTEARRAY;
}

void Variant::IsByteArray(bool isByteArray) {
	if (isByteArray) {
		if (_type == V_STRING) {
			_type = V_BYTEARRAY;
		}
	} else {
		if (_type == V_BYTEARRAY) {
			_type = V_STRING;
		}
	}
}

bool Variant::ConvertToTimestamp() {
	VariantType detectedType = V_NULL;
	if (!IsTimestamp(detectedType))
		return false;

	Timestamp temp = Timestamp_init;

	if (detectedType == V_DATE || detectedType == V_TIMESTAMP) {
		temp.tm_year = (int) ((int32_t) (*this)["year"] - 1900);
		temp.tm_mon = (int) ((int32_t) (*this)["month"]);
		temp.tm_mday = (int) ((int32_t) (*this)["day"]);
	} else {
		temp.tm_year = 70;
		temp.tm_mon = 0;
		temp.tm_mday = 1;
	}

	if (detectedType == V_TIME || detectedType == V_TIMESTAMP) {
		temp.tm_hour = (int) ((int32_t) (*this)["hour"]);
		temp.tm_min = (int) ((int32_t) (*this)["min"]);
		temp.tm_sec = (int) (HasKey("sec") ?
				(int32_t) (*this)["sec"] : 0);
		temp.tm_isdst = HasKey("isdst") ? (bool) ((*this)["isdst"]) : false;
	}

	if (mktime(&temp) < 0) {
		return false;
	}

	Reset();
	DYNAMIC_ALLOC("_value.t");
	_value.t = new Timestamp;
	*_value.t = temp;

	_type = detectedType;

	return true;
}

void Variant::Compact() {
	switch (_type) {
		case V_DOUBLE:
		{
			if ((((double) (*this)) < INT32_MIN)
					|| (((double) (*this)) > UINT32_MAX))
				break;
			Variant &variant = *this;
			double doubleVal = (double) variant;
			if ((int64_t) doubleVal != doubleVal)
				break;
			variant = (int64_t) doubleVal;
			variant.Compact();
			break;
		}
		case V_INT64:
		{
			Variant &variant = *this;
			int64_t val = (int64_t) variant;
			if ((val < INT32_MIN) || (val > UINT32_MAX))
				break;
			if (val < 0)
				variant = (int32_t) variant;
			else
				variant = (uint32_t) variant;
			Compact();
			break;
		}
		case V_INT32:
		{
			Variant &variant = *this;
			int32_t val = (int32_t) variant;
			if ((val < INT16_MIN) || (val > (int32_t) (UINT16_MAX)))
				break;
			if (val < 0)
				variant = (int16_t) variant;
			else
				variant = (uint16_t) variant;
			Compact();
			break;
		}
		case V_INT16:
		{
			Variant &variant = *this;
			int16_t val = (int16_t) variant;
			if ((val < INT8_MIN) || (val > (int16_t) (UINT8_MAX)))
				break;
			if (val < 0)
				variant = (int8_t) variant;
			else
				variant = (uint8_t) variant;
			Compact();
			break;
		}
		case V_UINT64:
		{
			Variant &variant = *this;
			if ((uint64_t) variant <= INT64_MAX) {
				variant = (int64_t) variant;
				Compact();
			}
			break;
		}
		case V_UINT32:
		{
			Variant &variant = *this;
			if ((uint32_t) variant <= INT32_MAX) {
				variant = (int32_t) variant;
				Compact();
			}
			break;
		}
		case V_UINT16:
		{
			Variant &variant = *this;
			if ((uint16_t) variant <= INT16_MAX) {
				variant = (int16_t) variant;
				Compact();
			}
			break;
		}
		case V_UINT8:
		{
			Variant &variant = *this;
			if ((uint8_t) variant <= INT8_MAX) {
				variant = (int8_t) variant;
				Compact();
			}
			break;
		}
		case V_MAP:
		case V_TYPED_MAP:
		{

			FOR_MAP(*this, string, Variant, i) {
				MAP_VAL(i).Compact();
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

bool Variant::DeserializeFromBin(uint8_t *pBuffer, uint32_t bufferLength,
		Variant &variant) {
	uint32_t cursor = 0;
	return DeserializeFromBin(pBuffer, bufferLength, variant, cursor);
}

bool Variant::DeserializeFromBin(string &data, Variant &variant) {
	return DeserializeFromBin((uint8_t *) data.c_str(), data.size(), variant);
}

bool Variant::SerializeToBin(string &result) {
	result += string(1, (char) _type);
	switch (_type) {
		case V_NULL:
		case V_UNDEFINED:
		{
			return true;
		}
		case V_BOOL:
		{
			result += string(1, (char) _value.b);
			return true;
		}
		case V_INT8:
		{
			result += string(1, (char) _value.i8);
			return true;
		}
		case V_INT16:
		{
			int16_t val = EHTONS(_value.i16); //----MARKED-SHORT----
			result += string((char *) & val, sizeof (int16_t));
			return true;
		}
		case V_INT32:
		{
			int32_t val = EHTONL(_value.i32); //----MARKED-LONG---
			result += string((char *) & val, sizeof (int32_t));
			return true;
		}
		case V_INT64:
		{
			int64_t val = EHTONLL(_value.i64);
			result += string((char *) & val, sizeof (int64_t));
			return true;
		}
		case V_UINT8:
		{
			result += string((char *) & _value.ui8, sizeof (uint8_t));
			return true;
		}
		case V_UINT16:
		{
			uint16_t val = EHTONS(_value.ui16); //----MARKED-SHORT----
			result += string((char *) & val, sizeof (uint16_t));
			return true;
		}
		case V_UINT32:
		{
			uint32_t val = EHTONL(_value.ui32); //----MARKED-LONG---
			result += string((char *) & val, sizeof (uint32_t));
			return true;
		}
		case V_UINT64:
		{
			uint64_t val = EHTONLL(_value.ui64);
			result += string((char *) & val, sizeof (uint64_t));
			return true;
		}
		case V_DOUBLE:
		{
			uint64_t val = 0;
			EHTOND(_value.d, val);
			result += string((char *) & val, sizeof (uint64_t));
			return true;
		}
		case V_TIMESTAMP:
		case V_DATE:
		case V_TIME:
		{
			uint64_t temp = EHTONLL((uint64_t) timegm(_value.t));
			result += string((char *) & temp, sizeof (uint64_t));
			return true;
		}
		case V_BYTEARRAY:
		case V_STRING:
		{
			uint32_t length = EHTONL((uint32_t) _value.s->size()); //----MARKED-LONG---
			result += string((char *) & length, sizeof (uint32_t));
			result += *(_value.s);
			return true;
		}
		case V_MAP:
		case V_TYPED_MAP:
		{
			bool isArray = IsArray();
			result += string(1, (char) isArray);

			uint32_t length = 0;
			if (_type == V_TYPED_MAP) {
				length = EHTONL((uint32_t) _value.m->typeName.size()); //----MARKED-LONG---
				result += string((char *) & length, sizeof (uint32_t));
				result += _value.m->typeName;
			}

			length = EHTONL(MapSize()); //----MARKED-LONG---
			result += string((char *) & length, sizeof (uint32_t));

			FOR_MAP(*this, string, Variant, i) {
				length = EHTONL((uint32_t) MAP_KEY(i).size()); //----MARKED-LONG---
				result += string((char *) & length, sizeof (uint32_t));
				result += MAP_KEY(i);
				string temp = "";
				if (!MAP_VAL(i).SerializeToBin(temp)) {
					result = "";
					return false;
				} else {
					result += temp;
				}
			}
			return true;
		}
		default:
		{
			result = "";
			return false;
		}
	}
	return true;
}

bool Variant::DeserializeFromXml(const uint8_t *pBuffer, uint32_t bufferLength,
		Variant &variant) {
	variant.Reset();
	if (bufferLength == 0) {
		return true;
	}

	uint8_t *pTemp = NULL;
	if (pBuffer[bufferLength - 1] != 0) {
		pTemp = new uint8_t[bufferLength + 1];
		memcpy(pTemp, pBuffer, bufferLength);
		pTemp[bufferLength] = 0;
		pBuffer = pTemp;
	}

	TiXmlDocument document;
	document.Parse((char *) pBuffer);
	if (document.Error()) {
		if (pTemp != NULL)
			delete[] pTemp;
		return false;
	}

	if (!DeserializeFromXml(document.RootElement(), variant)) {
		variant.Reset();
		if (pTemp != NULL)
			delete[] pTemp;
		return false;
	}

	if (pTemp != NULL)
		delete[] pTemp;
	return true;
}

bool Variant::DeserializeFromXml(string data, Variant &result) {
	return DeserializeFromXml((const uint8_t *) data.c_str(), data.size(), result);
}

bool Variant::SerializeToXml(string &result, bool prettyPrint) {
	result = "";
	string name = "";
	TiXmlElement *pElement = SerializeToXmlElement(name);
	if (pElement == NULL) {
		return false;
	}

	TiXmlDocument document;
	TiXmlDeclaration * pXmlDeclaration = new TiXmlDeclaration("1.0", "", "");
	document.LinkEndChild(pXmlDeclaration);
	document.LinkEndChild(pElement);

	if (prettyPrint) {
		TiXmlPrinter printer;
		document.Accept(&printer);
		result = printer.Str();
	} else {
		stringstream ss;
		ss << document;
		result = ss.str();
	}

	return true;
}

bool Variant::DeserializeFromJSON(string &raw, Variant &result, uint32_t &start) {
	result.Reset();
	if (start >= raw.size()) {
		return false;
	}

	if (!ReadJSONWhiteSpace(raw, start)) {
		return false;
	}

	switch (raw[start]) {
		case '\"':
		{
			return ReadJSONString(raw, result, start);
		}
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			return ReadJSONNumber(raw, result, start);
		}
		case '{':
		{
			return ReadJSONObject(raw, result, start);
		}
		case '[':
		{
			return ReadJSONArray(raw, result, start);
		}
		case 't':
		case 'T':
		{
			return ReadJSONBool(raw, result, start, "true");
		}
		case 'f':
		case 'F':
		{
			return ReadJSONBool(raw, result, start, "false");
		}
		case 'n':
		case 'N':
		{
			return ReadJSONNull(raw, result, start);
		}
		default:
		{
			result.Reset();
			return false;
		}
	}
}

bool Variant::SerializeToJSON(string &result) {
	switch (_type) {
		case V_NULL:
		case V_UNDEFINED:
		{
			result += "null";
			break;
		}
		case V_BOOL:
		{
			result += ((bool)(*this)) ? "true" : "false";
			break;
		}
		case V_INT8:
		case V_INT16:
		case V_INT32:
		case V_INT64:
		{
			int64_t value = (int64_t) (*this);
			result += format("%"PRId64, value);
			break;
		}
		case V_UINT8:
		case V_UINT16:
		case V_UINT32:
		case V_UINT64:
		{
			uint64_t value = (uint64_t) (*this);
			result += format("%"PRIu64, value);
			break;
		}
		case V_DOUBLE:
		{
			result += format("%.4f", (double) (*this));
			break;
		}
		case V_TIMESTAMP:
		case V_DATE:
		case V_TIME:
		case V_TYPED_MAP:
		case V_BYTEARRAY:
		{
			result += "\"V_TIMESTAMP,V_DATE,V_TIME,V_TYPED_MAP and V_BYTEARRAY not supported by JSON\"";
			break;
		}
		case V_STRING:
		{
			string value = (string) (*this);
			EscapeJSON(value);
			result += value;
			break;
		}
		case V_MAP:
		{
			result += IsArray() ? "[" : "{";

			FOR_MAP(_value.m->children, string, Variant, i) {
				if (!IsArray()) {
					string key = MAP_KEY(i);
					EscapeJSON(key);
					result += key + ":";
				}
				if (!MAP_VAL(i).SerializeToJSON(result)) {
					return false;
				}
				result += ",";
			}
			if (_value.m->children.size() > 0) {
				result[result.size() - 1] = IsArray() ? ']' : '}';
			} else {
				result += IsArray() ? "]" : "}";
			}
			break;
		}
		default:
		{
			break;
		}
	}
	return true;
}

bool Variant::DeserializeFromCmdLineArgs(uint32_t count, char **pArguments,
		Variant &result) {
	if (count < 1) {
		return false;
	}
	result.Reset();
	result["program"] = pArguments[0];
	result["arguments"].IsArray(false);
	for (uint32_t i = 1; i < count; i++) {
		string keyValue = pArguments[i];
		string::size_type separatorPos = string::npos;
		if ((separatorPos = keyValue.find('=')) == string::npos) {
			result["arguments"][keyValue] = (bool)true;
		} else {
			string key = keyValue.substr(0, separatorPos);
			string value = keyValue.substr(separatorPos + 1,
					keyValue.size() - separatorPos);
			result["arguments"][key] = value;
		}
	}
	return true;
}

TiXmlElement *Variant::SerializeToXmlElement(string &name) {
	TiXmlElement *pResult = NULL;
	switch (_type) {
		case V_NULL:
		{
			pResult = new TiXmlElement("NULL");
			break;
		}
		case V_UNDEFINED:
		{
			pResult = new TiXmlElement("UNDEFINED");
			break;
		}
		case V_BOOL:
		{
			pResult = new TiXmlElement("BOOL");
			pResult->LinkEndChild(new TiXmlText(_value.b ? "true" : "false"));
			break;
		}
		case V_INT8:
		{
			pResult = new TiXmlElement("INT8");
			pResult->LinkEndChild(new TiXmlText(format("%hhd", _value.i8)));
			break;
		}
		case V_INT16:
		{
			pResult = new TiXmlElement("INT16");
			pResult->LinkEndChild(new TiXmlText(format("%hd", _value.i16)));
			break;
		}
		case V_INT32:
		{
			pResult = new TiXmlElement("INT32");
			pResult->LinkEndChild(new TiXmlText(format("%d", _value.i32)));
			break;
		}
		case V_INT64:
		{
			pResult = new TiXmlElement("INT64");
			pResult->LinkEndChild(new TiXmlText(format("%"PRId64, _value.i64)));
			break;
		}
		case V_UINT8:
		{
			pResult = new TiXmlElement("UINT8");
			pResult->LinkEndChild(new TiXmlText(format("%hhu", _value.ui8)));
			break;
		}
		case V_UINT16:
		{
			pResult = new TiXmlElement("UINT16");
			pResult->LinkEndChild(new TiXmlText(format("%hu", _value.ui16)));
			break;
		}
		case V_UINT32:
		{
			pResult = new TiXmlElement("UINT32");
			pResult->LinkEndChild(new TiXmlText(format("%u", _value.ui32)));
			break;
		}
		case V_UINT64:
		{
			pResult = new TiXmlElement("UINT64");
			pResult->LinkEndChild(new TiXmlText(format("%"PRIu64, _value.ui64)));
			break;
		}
		case V_DOUBLE:
		{
			pResult = new TiXmlElement("DOUBLE");
			pResult->LinkEndChild(new TiXmlText(format("%.03f", _value.d)));
			break;
		}
		case V_TIMESTAMP:
		{
			pResult = new TiXmlElement("TIMESTAMP");
			pResult->LinkEndChild(new TiXmlText(STR(*this)));
			break;
		}
		case V_DATE:
		{
			pResult = new TiXmlElement("DATE");
			pResult->LinkEndChild(new TiXmlText(STR(*this)));
			break;
		}
		case V_TIME:
		{
			pResult = new TiXmlElement("TIME");
			pResult->LinkEndChild(new TiXmlText(STR(*this)));
			break;
		}
		case V_STRING:
		{
			pResult = new TiXmlElement("STR");
			pResult->LinkEndChild(new TiXmlText(STR(*_value.s)));
			break;
		}
		case V_BYTEARRAY:
		{
			pResult = new TiXmlElement("BYTEARRAY");
			pResult->LinkEndChild(new TiXmlText(STR(b64(*_value.s))));
			break;
		}
		case V_TYPED_MAP:
		case V_MAP:
		{
			if (_type == V_MAP) {
				pResult = new TiXmlElement("MAP");
			} else {
				pResult = new TiXmlElement("TYPED_MAP");
				pResult->SetAttribute("typeName", _value.m->typeName);
			}
			pResult->SetAttribute("isArray", _value.m->isArray ? "true" : "false");

			FOR_MAP(_value.m->children, string, Variant, i) {
				string key = MAP_KEY(i);
				TiXmlElement *pElement = MAP_VAL(i).SerializeToXmlElement(key);
				if (pElement == NULL) {
					delete pResult;
					pResult = NULL;
					break;
				}
				pResult->LinkEndChild(pElement);
			}

			break;
		}
		default:
		{
			return NULL;
		}
	}
	if (pResult != NULL) {
		pResult->SetAttribute("name", name);
	}
	return pResult;
}

#define VARIANT_CHECK_BOUNDS(s) \
do {\
	if(s>bufferSize-cursor) \
	{ \
		return false; \
	} \
}\
while(0)

#define PTR (pBuffer+cursor)

bool Variant::DeserializeFromBin(uint8_t *pBuffer, uint32_t bufferSize,
		Variant &variant, uint32_t &cursor) {
	VARIANT_CHECK_BOUNDS(1);
	VariantType type = (VariantType) PTR[0];
	cursor += 1;

	switch (type) {
		case V_NULL:
		{
			variant.Reset();
			return true;
		}
		case V_UNDEFINED:
		{
			variant.Reset(true);
			return true;
		}
		case V_BOOL:
		{
			VARIANT_CHECK_BOUNDS(1);
			variant = (bool)(PTR[0] != 0);
			cursor += 1;
			return true;
		}
		case V_INT8:
		{
			VARIANT_CHECK_BOUNDS(1);
			variant = *((int8_t *) PTR);
			cursor += 1;
			return true;
		}
		case V_INT16:
		{
			VARIANT_CHECK_BOUNDS(2);
			uint16_t val = ENTOHSP(PTR); //----MARKED-SHORT----
			cursor += 2;
			variant = *((int16_t *) & val);
			return true;
			break;
		}
		case V_INT32:
		{
			VARIANT_CHECK_BOUNDS(4);
			uint32_t val = ENTOHLP(PTR); //----MARKED-LONG---
			cursor += 4;
			variant = *((int32_t *) & val);
			return true;
		}
		case V_INT64:
		{
			VARIANT_CHECK_BOUNDS(8);
			uint64_t val = ENTOHLLP(PTR); //----MARKED-LONG---
			cursor += 8;
			variant = *((int64_t *) & val);
			return true;
		}
		case V_UINT8:
		{
			VARIANT_CHECK_BOUNDS(1);
			variant = *((uint8_t *) PTR);
			cursor += 1;
			return true;
		}
		case V_UINT16:
		{
			VARIANT_CHECK_BOUNDS(2);
			variant = ENTOHSP(PTR); //----MARKED-SHORT----
			cursor += 2;
			return true;
		}
		case V_UINT32:
		{
			VARIANT_CHECK_BOUNDS(4);
			variant = ENTOHLP(PTR); //----MARKED-LONG---
			cursor += 4;
			return true;
		}
		case V_UINT64:
		{
			VARIANT_CHECK_BOUNDS(8);
			variant = (uint64_t) ENTOHLLP(PTR); //----MARKED-LONG---
			cursor += 8;
			return true;
		}
		case V_DOUBLE:
		{
			VARIANT_CHECK_BOUNDS(8);
			double temp = 0;
			ENTOHDP(PTR, temp);
			cursor += 8;
			variant = (double) temp;
			return true;
		}
		case V_TIMESTAMP:
		case V_DATE:
		case V_TIME:
		{
			VARIANT_CHECK_BOUNDS(8);
			time_t val = (time_t) ENTOHLLP(PTR); //----MARKED-LONG---
			cursor += 8;
			variant = *((Timestamp *) gmtime(&val));
			variant._type = type;
			return true;
		}
		case V_BYTEARRAY:
		case V_STRING:
		{
			VARIANT_CHECK_BOUNDS(4);
			uint32_t length = ENTOHLP(PTR); //----MARKED-LONG---
			cursor += 4;
			VARIANT_CHECK_BOUNDS(length);
			if (length > 1024 * 128) {
				return false;
			}
			variant = string((char *) PTR, length);
			cursor += length;
			variant.IsByteArray(type == V_BYTEARRAY);
			return true;
		}
		case V_MAP:
		case V_TYPED_MAP:
		{
			VARIANT_CHECK_BOUNDS(1);
			bool isArray = (PTR[0] != 0);
			cursor += 1;
			variant.IsArray(isArray);

			uint32_t length = 0;
			if (type == V_TYPED_MAP) {
				VARIANT_CHECK_BOUNDS(4);
				length = ENTOHLP(PTR); //----MARKED-LONG---
				cursor += 4;
				VARIANT_CHECK_BOUNDS(length);
				if (length > 1024 * 128) {
					return false;
				}
				string name = string((char *) PTR, length);
				cursor += length;
				variant.SetTypeName(name);
			}

			VARIANT_CHECK_BOUNDS(4);
			length = ENTOHLP(PTR); //----MARKED-LONG---
			if (length > 1024) {
				return false;
			}
			cursor += 4;

			for (uint32_t i = 0; i < length; i++) {
				string key;
				uint32_t keyLength;

				VARIANT_CHECK_BOUNDS(4);
				keyLength = ENTOHLP(PTR); //----MARKED-LONG---
				cursor += 4;
				VARIANT_CHECK_BOUNDS(keyLength);
				if (keyLength > 1024 * 128) {
					return false;
				}
				key = string((char *) PTR, keyLength);
				cursor += keyLength;

				if (!DeserializeFromBin(pBuffer, bufferSize, variant[key], cursor)) {
					return false;
				}
			}
			return true;
		}
		default:
		{
			return false;
		}
	}
}

bool Variant::DeserializeFromXml(TiXmlElement *pNode, Variant &variant) {
	string nodeName = lowerCase(pNode->ValueStr());

	union {
		int64_t i64;
		uint64_t ui64;
		double d;
		Timestamp t;
	} val;

	const char *pText = pNode->GetText();
	string text = pText == NULL ? "" : pText;

	if (nodeName == "bool") {
		variant = (bool)(lowerCase(text) == "true");
		return true;
	} else if (nodeName == "null") {
		variant.Reset();
		return true;
	} else if (nodeName == "undefined") {
		variant.Reset(true);
		return true;
	} else if (nodeName == "int8") {
		if (sscanf(STR(text), "%"PRId64, &val.i64) != 1) {
			return false;
		}
		variant = (int8_t) val.i64;
		return true;
	} else if (nodeName == "int16") {
		if (sscanf(STR(text), "%"PRId64, &val.i64) != 1) {
			return false;
		}
		variant = (int16_t) val.i64;
		return true;
	} else if (nodeName == "int32") {
		if (sscanf(STR(text), "%"PRId64, &val.i64) != 1) {
			return false;
		}
		variant = (int32_t) val.i64;
		return true;
	} else if (nodeName == "int64") {
		if (sscanf(STR(text), "%"PRId64, &val.i64) != 1) {
			return false;
		}
		variant = (int64_t) val.i64;
		return true;
	} else if (nodeName == "uint8") {
		if (sscanf(STR(text), "%"PRIu64, &val.ui64) != 1) {
			return false;
		}
		variant = (uint8_t) val.ui64;
		return true;
	} else if (nodeName == "uint16") {
		if (sscanf(STR(text), "%"PRIu64, &val.ui64) != 1) {
			return false;
		}
		variant = (uint16_t) val.ui64;
		return true;
	} else if (nodeName == "uint32") {
		if (sscanf(STR(text), "%"PRIu64, &val.ui64) != 1) {
			return false;
		}
		variant = (uint32_t) val.ui64;
		return true;
	} else if (nodeName == "uint64") {
		if (sscanf(STR(text), "%"PRIu64, &val.ui64) != 1) {
			return false;
		}
		variant = (uint64_t) val.ui64;
		return true;
	} else if (nodeName == "double") {
		if (sscanf(STR(text), "%lf", &val.d) != 1) {
			return false;
		}
		variant = (double) val.d;
		return true;
	} else if (nodeName == "timestamp") {
		if (strptime(STR(text), "%Y-%m-%dT%H:%M:%S.000", &val.t) == NULL) {
			return false;
		}
		variant = (Timestamp) val.t;
		variant._type = V_TIMESTAMP;
		return true;
	} else if (nodeName == "date") {
		if (strptime(STR(text), "%Y-%m-%u", &val.t) == NULL) {
			return false;
		}
		variant = (Timestamp) val.t;
		variant._type = V_DATE;
		return true;
	} else if (nodeName == "time") {
		if (strptime(STR(text), "%H:%M:%S.000", &val.t) == NULL) {
			return false;
		}
		variant = (Timestamp) val.t;
		variant._type = V_TIME;
		return true;
	} else if (nodeName == "str") {
		variant = text;
		return true;
	} else if (nodeName == "bytearray") {
		variant = unb64(text);
		variant.IsByteArray(true);
		return true;
	} else if (nodeName == "map" || nodeName == "typed_map") {
		TiXmlAttribute *pAttribute = pNode->FirstAttribute();

		//isArray and typename
		variant.IsArray(false);
		for (TiXmlAttribute *pI = pAttribute; pI != NULL; pI = pI->Next()) {
			if (lowerCase(pI->NameTStr()) == "isarray") {
				variant.IsArray(lowerCase(pI->ValueStr()) == "true");
			} else if (lowerCase(pI->NameTStr()) == "typename") {
				variant.SetTypeName(pI->Value());
			}
		}

		//childs
		for (TiXmlElement *pE = pNode->FirstChildElement(); pE != NULL; pE = pE->NextSiblingElement()) {
			//search for the name
			string key = "";
			for (TiXmlAttribute *pA = pE->FirstAttribute(); pA != NULL; pA = pA->Next()) {
				if (lowerCase(pA->NameTStr()) == "name") {
					key = pA->ValueStr();
					break;
				}
			}

			if (!DeserializeFromXml(pE, variant[key])) {
				return false;
			}
		}

		return true;
	} else {
		return false;
	}
}

void Variant::InternalCopy(const Variant &val) {
	_type = val._type;
	memset(&_value, 0, sizeof (_value));
	switch (val._type) {
		case V_DATE:
		case V_TIME:
		case V_TIMESTAMP:
		{
			DYNAMIC_ALLOC("_value.t");
			_value.t = new Timestamp(*val._value.t);
			break;
		}
		case V_BYTEARRAY:
		case V_STRING:
		{
			DYNAMIC_ALLOC("_value.s");
			_value.s = new string(*val._value.s);
			break;
		}
		case V_MAP:
		case V_TYPED_MAP:
		{
			DYNAMIC_ALLOC("_value.m");
			_value.m = new VariantMap(*val._value.m);
			break;
		}
		default:
		{
			memcpy(&_value, &val._value, sizeof (_value));
			break;
		}
	}
}

void Variant::NormalizeTs() {
	time_t val = timegm(_value.t);
	if (val < 0) {
		val = 0;
	}
	gmtime_r(&val, _value.t);
}

void Variant::EscapeJSON(string &value) {
	replace(value, "\\", "\\\\");
	replace(value, "/", "\\/");
	replace(value, "\"", "\\\"");
	replace(value, "\b", "\\b");
	replace(value, "\f", "\\f");
	replace(value, "\n", "\\n");
	replace(value, "\r", "\\r");
	replace(value, "\t", "\\t");
	value = "\"" + value + "\"";
}

void Variant::UnEscapeJSON(string &value) {
	replace(value, "\\/", "/");
	replace(value, "\\\"", "\"");
	replace(value, "\\b", "\b");
	replace(value, "\\f", "\f");
	replace(value, "\\n", "\n");
	replace(value, "\\r", "\r");
	replace(value, "\\t", "\t");
	replace(value, "\\\\", "\\");
}

bool Variant::ReadJSONWhiteSpace(string &raw, uint32_t &start) {
	for (; start < raw.length(); start++) {
		if ((raw[start] != ' ')
				&& (raw[start] != '\t')
				&& (raw[start] != '\r')
				&& (raw[start] != '\n'))
			break;
	}
	return true;
}

bool Variant::ReadJSONDelimiter(string &raw, uint32_t &start, char &c) {
	if (!ReadJSONWhiteSpace(raw, start)) {
		return false;
	}
	if ((raw.size() - start) < 1) {
		return false;
	}
	c = raw[start];
	start++;
	return ReadJSONWhiteSpace(raw, start);
}

bool Variant::ReadJSONString(string &raw, Variant &result, uint32_t &start) {
	if ((raw.size() - start) < 2) {
		return false;
	}
	if (raw[start] != '\"') {
		return false;
	}
	start++;
	string::size_type pos = start;
	while (true) {
		pos = raw.find('\"', pos);
		if (pos == string::npos) {
			return false;
		}
		if (raw[pos - 1] == '\\') {
			pos++;
		} else {
			string value = raw.substr(start, pos - start);
			UnEscapeJSON(value);
			result = value;
			start = pos + 1;
			return true;
		}
	}
}

bool Variant::ReadJSONNumber(string &raw, Variant &result, uint32_t &start) {
	string str = "";
	for (; start < raw.length(); start++) {
		if ((raw[start] < '0')
				|| (raw[start] > '9')) {
			break;
		}
		str += raw[start];
	}
	if (str == "") {
		return false;
	}
	result = (int64_t) atoll(STR(str));
	return true;
}

bool Variant::ReadJSONObject(string &raw, Variant &result, uint32_t &start) {
	result.Reset();
	result.IsArray(false);
	if ((raw.size() - start) < 2) {
		return false;
	}
	if (raw[start] != '{') {
		return false;
	}
	start++;
	char c;
	while (start < raw.length()) {
		if (raw[start] == '}') {
			start++;
			return true;
		}
		Variant key;
		if (!Variant::DeserializeFromJSON(raw, key, start)) {
			return false;
		}

		if (!ReadJSONDelimiter(raw, start, c)) {
			return false;
		}
		if (c != ':') {
			return false;
		}

		Variant value;
		if (!Variant::DeserializeFromJSON(raw, value, start)) {
			return false;
		}
		result[key] = value;


		if (!ReadJSONDelimiter(raw, start, c)) {
			return false;
		}
		if (c == '}') {
			return true;
		} else if (c == ',') {
			continue;
		} else {
			return false;
		}
	}
	return false;
}

bool Variant::ReadJSONArray(string &raw, Variant &result, uint32_t &start) {
	result.Reset();
	result.IsArray(true);
	if ((raw.size() - start) < 2) {
		return false;
	}
	if (raw[start] != '[') {
		return false;
	}
	start++;
	char c;
	while (start < raw.length()) {
		if (raw[start] == ']') {
			start++;
			return true;
		}
		Variant value;
		if (!Variant::DeserializeFromJSON(raw, value, start)) {
			return false;
		}
		result.PushToArray(value);

		if (!ReadJSONDelimiter(raw, start, c)) {
			return false;
		}
		if (c == ']') {
			return true;
		} else if (c == ',') {
			continue;
		} else {
			return false;
		}
	}
	return false;
}

bool Variant::ReadJSONBool(string &raw, Variant &result, uint32_t &start, string wanted) {
	if ((raw.size() - start) < wanted.size()) {
		return false;
	}
	string temp = lowerCase(raw.substr(start, wanted.size()));
	if (temp != wanted) {
		return false;
	}
	start += wanted.size();
	result = (bool)(wanted == "true");
	return true;
}

bool Variant::ReadJSONNull(string &raw, Variant &result, uint32_t &start) {
	if ((raw.size() - start) < 4) {
		return false;
	}
	string temp = lowerCase(raw.substr(start, 4));
	if (temp != "null") {
		return false;
	}
	start += 4;
	result.Reset();
	return true;
}
