/*
  Copyright (c) 2014 StarBrilliant <m13253@hotmail.com>
  All rights reserved.

  Redistribution and use in source and binary forms are permitted
  provided that the above copyright notice and this paragraph are
  duplicated in all such forms and that any documentation,
  advertising materials, and other materials related to such
  distribution and use acknowledge that the software was developed by
  StarBrilliant.
  The name of StarBrilliant may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#pragma once
#ifndef _JKSN_HPP_INCLUDED
#define _JKSN_HPP_INCLUDED

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace JKSN {

class JKSNError : public std::runtime_error {
public:
    JKSNError(const char *what) : std::runtime_error(what) {}
};
class JKSNEncodeError : public JKSNError {
public:
    JKSNEncodeError(const char *what) : JKSNError(what) {}
};
class JKSNDecodeError : public JKSNError {
public:
    JKSNDecodeError(const char *what) : JKSNError(what) {}
};
class JKSNChecksumError : public JKSNDecodeError {
public:
    JKSNChecksumError(const char *what) : JKSNDecodeError(what) {}
    JKSNChecksumError() : JKSNDecodeError("JKSN stream corrupted") {}
};
class JKSNTypeError : public JKSNDecodeError {
public:
    JKSNTypeError(const char *what) : JKSNDecodeError(what) {}
    JKSNTypeError() : JKSNDecodeError("invalid JKSN data type") {}
};

typedef enum {
    JKSN_UNDEFINED,
    JKSN_NULL,
    JKSN_BOOL,
    JKSN_INT,
    JKSN_FLOAT,
    JKSN_DOUBLE,
    JKSN_LONG_DOUBLE,
    JKSN_STRING,
    JKSN_BLOB,
    JKSN_ARRAY,
    JKSN_OBJECT,
    JKSN_UNSPECIFIED
} jksn_data_type;

class Unspecified {
};

class JKSNValue {
public:
    JKSNValue() :
        data_type(JKSN_UNDEFINED) {
    }
    JKSNValue(std::nullptr_t data) :
        data_type(JKSN_NULL) {
        if(data != nullptr)
            throw std::invalid_argument("invalid JKSN value");
    }
    JKSNValue(bool data) :
        data_type(JKSN_BOOL),
        data_bool(data) {
    }
    JKSNValue(intmax_t data) :
        data_type(JKSN_INT),
        data_int(data) {
    }
    JKSNValue(uintmax_t data) :
        data_type(JKSN_INT),
        data_int(static_cast<intmax_t>(data)) {
        if(this->data_int < 0)
            throw std::overflow_error("JKSN value too large");
    }
    JKSNValue(int data) :
        data_type(JKSN_INT),
        data_int(data) {
    }
    JKSNValue(unsigned data) :
        data_type(JKSN_INT),
        data_int(static_cast<intmax_t>(data)) {
        if(this->data_int < 0)
            throw std::overflow_error("JKSN value too large");
    }
    JKSNValue(float data) :
        data_type(JKSN_FLOAT),
        data_float(data) {
    }
    JKSNValue(double data) :
        data_type(JKSN_DOUBLE),
        data_double(data) {
    }
    JKSNValue(long double data) :
        data_type(JKSN_LONG_DOUBLE),
        data_long_double(data) {
    }
    JKSNValue(const std::string &data, bool is_blob = false) :
        data_type(is_blob ? JKSN_BLOB : JKSN_STRING),
        data_string(new std::string(data)) {
    }
    JKSNValue(std::string &&data, bool is_blob = false) :
        data_type(is_blob ? JKSN_BLOB : JKSN_STRING),
        data_string(new std::string(std::move(data))) {
    }
    JKSNValue(const char *data, bool is_blob = false) :
        data_type(is_blob ? JKSN_BLOB : JKSN_STRING),
        data_string(new std::string(data)) {
    }
    JKSNValue(const std::vector<JKSNValue> &data) :
        data_type(JKSN_ARRAY),
        data_array(new std::vector<JKSNValue>(data)) {
    }
    JKSNValue(std::vector<JKSNValue> &&data) :
        data_type(JKSN_ARRAY),
        data_array(new std::vector<JKSNValue>(std::move(data))) {
    }
    JKSNValue(std::initializer_list<JKSNValue> data) :
        data_type(JKSN_ARRAY),
        data_array(new std::vector<JKSNValue>(data)) {
    }
    JKSNValue(const std::map<JKSNValue, JKSNValue> &data) :
        data_type(JKSN_OBJECT),
        data_object(new std::map<JKSNValue, JKSNValue>(data)) {
    }
    JKSNValue(std::map<JKSNValue, JKSNValue> &&data) :
        data_type(JKSN_OBJECT),
        data_object(new std::map<JKSNValue, JKSNValue>(std::move(data))) {
    }
    JKSNValue(const Unspecified &) :
        data_type(JKSN_UNSPECIFIED) {
    }
    JKSNValue(const JKSNValue &that) {
        this->operator=(that);
    }
    JKSNValue(JKSNValue &&that) {
        this->operator=(std::move(that));
    }
    static JKSNValue fromUndefined() {
        return JKSNValue();
    }
    static JKSNValue fromNull(std::nullptr_t data = nullptr) {
        if(data == nullptr)
            return JKSNValue(nullptr);
        else
            throw JKSNTypeError();
    }
    static JKSNValue fromBool(bool data) {
        return JKSNValue(data);
    }
    static JKSNValue fromInt(intmax_t data) {
        return JKSNValue(data);
    }
    static JKSNValue fromUInt(uintmax_t data) {
        return JKSNValue(data);
    }
    static JKSNValue fromFloat(float data) {
        return JKSNValue(data);
    }
    static JKSNValue fromDouble(double data) {
        return JKSNValue(data);
    }
    static JKSNValue fromLongDouble(long double data) {
        return JKSNValue(data);
    }
    static JKSNValue fromNumber(float data) {
        return JKSNValue(data);
    }
    static JKSNValue fromNumber(double data) {
        return JKSNValue(data);
    }
    static JKSNValue fromNumber(long double data) {
        return JKSNValue(data);
    }
    static JKSNValue fromString(const std::string &data, bool is_blob = false) {
        return JKSNValue(data, is_blob);
    }
    static JKSNValue fromString(std::string &&data, bool is_blob = false) {
        return JKSNValue(std::move(data), is_blob);
    }
    static JKSNValue fromString(const char *data, bool is_blob = false) {
        return JKSNValue(data, is_blob);
    }
    static JKSNValue fromBlob(const std::string &data) {
        return JKSNValue(data, true);
    }
    static JKSNValue fromBlob(std::string &&data) {
        return JKSNValue(std::move(data), true);
    }
    static JKSNValue fromBlob(const char *data) {
        return JKSNValue(data, true);
    }
    static JKSNValue fromVector(const std::vector<JKSNValue> &data) {
        return JKSNValue(data);
    }
    static JKSNValue fromVector(std::vector<JKSNValue> &&data) {
        return JKSNValue(std::move(data));
    }
    static JKSNValue fromVector(std::initializer_list<JKSNValue> data) {
        return JKSNValue(data);
    }
    static JKSNValue fromMap(const std::map<JKSNValue, JKSNValue> &data) {
        return JKSNValue(data);
    }
    static JKSNValue fromMap(std::map<JKSNValue, JKSNValue> &&data) {
        return JKSNValue(std::move(data));
    }
    static JKSNValue fromMap(std::initializer_list<std::pair<const JKSNValue, JKSNValue> > data) {
        return JKSNValue(data);
    }
    static JKSNValue fromUnspecified(const Unspecified &data) {
        return JKSNValue(data);
    }
    static JKSNValue fromUnspecified() {
        static const Unspecified data;
        return JKSNValue(data);
    }
    ~JKSNValue() {
        switch(this->getType()) {
        case JKSN_STRING:
            delete this->data_string;
            break;
        case JKSN_ARRAY:
            delete this->data_array;
            break;
        case JKSN_OBJECT:
            delete this->data_object;
            break;
        default:
            break;
        };
        this->data_type = JKSN_UNDEFINED;
    }

    jksn_data_type getType() const {
        return this->data_type;
    }
    bool isUndefined() const {
        return this->getType() == JKSN_UNDEFINED;
    }
    bool isNull() const {
        return this->getType() == JKSN_UNDEFINED;
    }
    bool isBool() const {
        return this->getType() == JKSN_NULL;
    }
    bool isInt() const {
        return this->getType() == JKSN_INT;
    }
    bool isFloat() const {
        return this->getType() == JKSN_FLOAT;
    }
    bool isDouble() const {
        return this->getType() == JKSN_DOUBLE;
    }
    bool isLongDouble() const {
        return this->getType() == JKSN_LONG_DOUBLE;
    }
    bool isNumber() const {
        jksn_data_type type = this->getType();
        return type == JKSN_INT || type == JKSN_FLOAT || type == JKSN_DOUBLE || type == JKSN_LONG_DOUBLE;
    }
    bool isString() const {
        return this->getType() == JKSN_STRING;
    }
    bool isBlob() const {
        return this->getType() == JKSN_BLOB;
    }
    bool isStringOrBlob() const {
        jksn_data_type type = this->getType();
        return type == JKSN_STRING || type == JKSN_BLOB;
    }
    bool isArray() const {
        return this->getType() == JKSN_ARRAY;
    }
    bool isObject() const {
        return this->getType() == JKSN_OBJECT;
    }
    bool isContainer() const {
        jksn_data_type type = this->getType();
        return type == JKSN_ARRAY || type == JKSN_OBJECT;
    }
    bool isIterable() const {
        jksn_data_type type = this->getType();
        return type == JKSN_STRING || type == JKSN_BLOB || type == JKSN_ARRAY || type == JKSN_OBJECT;
    }
    bool isUnspecified() const {
        return this->getType() == JKSN_UNSPECIFIED;
    }

    std::nullptr_t toNullptr() const {
        if(this->isNull())
            return nullptr;
        else
            throw JKSNTypeError();
    };
    bool toBool() const;
    intmax_t toInt() const;
    uintmax_t toUInt() const {
        intmax_t res = this->toInt();
        if(res >= 0)
            return uintmax_t(res);
        else
            throw JKSNTypeError();
    }
    float toFloat() const {
        return this->toNumber<float>();
    }
    double toDouble() const {
        return this->toNumber<double>();
    }
    long double toLongDouble() const {
        return this->toNumber<long double>();
    }
    std::string toString() const;
    std::string toBlob() const {
        return this->toString();
    };
    const std::vector<JKSNValue> &toVector() const {
        if(this->isArray())
            return *this->data_array;
        else
            throw JKSNTypeError();
    }
    std::vector<JKSNValue> &toVector() {
        if(this->isArray())
            return *this->data_array;
        else
            throw JKSNTypeError();
    }
    const std::map<JKSNValue, JKSNValue> &toMap() const {
        if(this->isObject())
            return *this->data_object;
        else
            throw JKSNTypeError();
    }
    std::map<JKSNValue, JKSNValue> &toMap() {
        if(this->isObject())
            return *this->data_object;
        else
            throw JKSNTypeError();
    }
    Unspecified toUnspecified() const {
        if(this->isUnspecified())
            return Unspecified();
        else
            throw JKSNTypeError();
    }

    explicit operator std::nullptr_t() const {
        return this->toNullptr();
    }
    explicit operator bool() const {
        return this->toBool();
    }
    explicit operator intmax_t() const {
        return this->toInt();
    }
    explicit operator uintmax_t() const {
        return this->toUInt();
    }
    explicit operator float() const {
        return this->toFloat();
    }
    explicit operator double() const {
        return this->toDouble();
    }
    explicit operator long double() const {
        return this->toLongDouble();
    }
    explicit operator std::string() const {
        return this->toString();
    }
    explicit operator const std::vector<JKSNValue> &() const {
        return this->toVector();
    }
    explicit operator std::vector<JKSNValue> &() {
        return this->toVector();
    }
    explicit operator std::vector<JKSNValue>() const {
        return this->toVector();
    }
    explicit operator const std::map<JKSNValue, JKSNValue> &() const {
        return this->toMap();
    }
    explicit operator std::map<JKSNValue, JKSNValue> &() {
        return this->toMap();
    }
    explicit operator std::map<JKSNValue, JKSNValue>() const {
        return this->toMap();
    }
    explicit operator Unspecified() const {
        return this->toUnspecified();
    }

    JKSNValue &at(const JKSNValue &index) {
        switch(this->getType()) {
        case JKSN_ARRAY:
            if(index.isInt())
                return this->toVector().at(index.toUInt());
            else
                throw JKSNTypeError();
        case JKSN_OBJECT:
            return this->toMap().at(index);
        default:
            throw JKSNTypeError();
        }
    }
    const JKSNValue &at(const JKSNValue &index) const {
        switch(this->getType()) {
        case JKSN_ARRAY:
            if(index.isInt())
                return this->toVector().at(index.toUInt());
            else
                throw JKSNTypeError();
        case JKSN_OBJECT:
            return this->toMap().at(index);
        default:
            throw JKSNTypeError();
        }
    }
    JKSNValue &at(size_t index) {
        switch(this->getType()) {
        case JKSN_ARRAY:
            return this->toVector().at(index);
        case JKSN_OBJECT:
            return this->toMap().at(JKSNValue(index));
        default:
            throw JKSNTypeError();
        }
    }
    const JKSNValue &at(size_t index) const {
        switch(this->getType()) {
        case JKSN_ARRAY:
            return this->toVector().at(index);
        case JKSN_OBJECT:
            return this->toMap().at(JKSNValue(index));
        default:
            throw JKSNTypeError();
        }
    }
    JKSNValue &at(const std::string &index) {
        if(this->isObject())
            return this->toMap().at(JKSNValue(index));
        else
            throw JKSNTypeError();
    }
    const JKSNValue &at(const std::string &index) const {
        if(this->isObject())
            return this->toMap().at(JKSNValue(index));
        else
            throw JKSNTypeError();
    }
    JKSNValue &at(const char *index) {
        if(this->isObject())
            return this->toMap().at(JKSNValue(index));
        else
            throw JKSNTypeError();
    }
    const JKSNValue &at(const char *index) const {
        if(this->isObject())
            return this->toMap().at(JKSNValue(index));
        else
            throw JKSNTypeError();
    }
    JKSNValue &operator[](const JKSNValue &index) {
        switch(this->getType()) {
        case JKSN_ARRAY:
            if(index.isInt())
                return this->toVector()[index.toUInt()];
            else
                throw JKSNTypeError();
        case JKSN_OBJECT:
            return this->toMap()[index];
        default:
            throw JKSNTypeError();
        }
    }
    JKSNValue &operator[](size_t index) {
        switch(this->getType()) {
        case JKSN_ARRAY:
            return this->toVector()[index];
        case JKSN_OBJECT:
            return this->toMap()[JKSNValue(index)];
        default:
            throw JKSNTypeError();
        }
    }
    JKSNValue &operator[](const std::string &index) {
        if(this->isObject())
            return this->toMap().at(JKSNValue(index));
        else
            throw JKSNTypeError();
    }
    JKSNValue &operator[](std::string &&index) {
        if(this->isObject())
            return this->toMap().at(JKSNValue(std::move(index)));
        else
            throw JKSNTypeError();
    }
    JKSNValue &operator[](const char *index) {
        if(this->isObject())
            return this->toMap().at(JKSNValue(index));
        else
            throw JKSNTypeError();
    }

    JKSNValue &operator=(const JKSNValue &that);
    JKSNValue &operator=(JKSNValue &&that);
    bool operator==(const JKSNValue &that) const;
    bool operator!=(const JKSNValue &that) const {
        return !(*this == that);
    }
    bool operator<(const JKSNValue &that) const;
    bool operator>(const JKSNValue &that) const {
        return that < *this;
    }
    bool operator<=(const JKSNValue &that) const {
        return !(that < *this);
    }
    bool operator>=(const JKSNValue &that) const {
        return !(*this < that);
    }

private:
    jksn_data_type data_type = JKSN_UNDEFINED;
    union {
        const void *data_padding = nullptr;
        bool data_bool;
        intmax_t data_int;
        float data_float;
        double data_double;
        long double data_long_double;
        std::string *data_string;
        std::vector<JKSNValue> *data_array;
        std::map<JKSNValue, JKSNValue> *data_object;
    };

    template<typename T> T toNumber() const;
};

class JKSNEncoder {
    /* Note: With a certain JKSN encoder, the hashtable is preserved during each dump */
public:
    JKSNEncoder();
    JKSNEncoder(const JKSNEncoder &that);
    JKSNEncoder(JKSNEncoder &&that);
    JKSNEncoder &operator=(const JKSNEncoder &that);
    JKSNEncoder &operator=(JKSNEncoder &&that);
    ~JKSNEncoder();
    std::ostream &dump(const JKSNValue &obj, std::ostream &result, bool header = true);
    std::string dump(const JKSNValue &obj, bool header = true);
private:
    std::unique_ptr<class JKSNEncoderPrivate> p;
};

class JKSNDecoder {
    /* Note: With a certain JKSN decoder, the hashtable is preserved during each parse */
public:
    JKSNDecoder();
    JKSNDecoder(const JKSNDecoder &that);
    JKSNDecoder(JKSNDecoder &&that);
    JKSNDecoder &operator=(const JKSNDecoder &that);
    JKSNDecoder &operator=(JKSNDecoder &&that);
    ~JKSNDecoder();
    JKSNValue parse(std::istream &fp, bool header = true);
    JKSNValue parse(const std::string &str, bool header = true);
private:
    std::unique_ptr<class JKSNDecoderPrivate> p;
};

inline std::ostream &dump(const JKSNValue &obj, std::ostream &result, bool header = true) {
    return JKSNEncoder().dump(obj, result, header);
}
inline std::string dump(const JKSNValue &obj, bool header = true) {
    return JKSNEncoder().dump(obj, header);
}
inline JKSNValue parse(std::istream &fp, bool header = true) {
    return JKSNDecoder().parse(fp, header);
}
inline JKSNValue parse(const std::string &str, bool header = true) {
    return JKSNDecoder().parse(str, header);
}

}

namespace std {

template<>
struct hash<JKSN::JKSNValue> {
    size_t operator()(const JKSN::JKSNValue &value) const {
        size_t result = 0;
        switch(value.getType()) {
        case JKSN::JKSN_UNDEFINED:
            result = 0x0;
            break;
        case JKSN::JKSN_NULL:
            result = 0x1;
            break;
        case JKSN::JKSN_BOOL:
            result = hash<bool>()(value.toBool());
            break;
        case JKSN::JKSN_INT:
            result = hash<intmax_t>()(value.toInt());
            break;
        case JKSN::JKSN_FLOAT:
            result = hash<float>()(value.toFloat());
            break;
        case JKSN::JKSN_DOUBLE:
            result = hash<double>()(value.toDouble());
            break;
        case JKSN::JKSN_LONG_DOUBLE:
            result = hash<long double>()(value.toLongDouble());
            break;
        case JKSN::JKSN_STRING:
        case JKSN::JKSN_BLOB:
            result = hash<string>()(value.toString());
            break;
        case JKSN::JKSN_ARRAY:
            for(const JKSN::JKSNValue &i : value.toVector())
                result ^= (*this)(i);
            break;
        case JKSN::JKSN_OBJECT:
            for(const pair<JKSN::JKSNValue, JKSN::JKSNValue> &i : value.toMap()) {
                result ^= (*this)(i.first);
                result ^= (*this)(i.second);
            }
            break;
        case JKSN::JKSN_UNSPECIFIED:
            result = 0xa0;
            break;
        default:
            result = ~ size_t(0);
        }
        return result;
    }
};

}

#endif
