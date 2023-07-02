/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>
 * */
#include "config.h"
#ifdef WITH_OPENSSL

#include <string>
#include <openssl/bn.h>
#include "hp/hp_log.h"
#include "cpp_test.h"

class MyBigNum{
protected:
	BIGNUM* _bn;
public:
	virtual ~MyBigNum() { BN_free(_bn); }
    MyBigNum() : _bn(BN_new()) {}
    MyBigNum(const char* str, int base = 0) : _bn(BN_new()) {
        BN_dec2bn(&this->_bn, str);
    }
    MyBigNum(const MyBigNum& other) : _bn(BN_new()) {
        BN_copy(this->_bn, other._bn);
    }
   MyBigNum& operator=(const MyBigNum& other) {
        BN_copy(this->_bn, other._bn);
        return *this;
    }

    MyBigNum& operator+=(const MyBigNum& other) {
        BN_add(this->_bn, this->_bn, other._bn);
        return *this;
    }

    MyBigNum& operator-=(const MyBigNum& other) {
        BN_sub(this->_bn, this->_bn, other._bn);
        return *this;
    }

    MyBigNum& operator*=(const MyBigNum& other) {
        BN_CTX* ctx = BN_CTX_new();
        BN_mul(this->_bn, this->_bn, other._bn, ctx);
        BN_CTX_free(ctx);
        return *this;
    }

    MyBigNum& operator/=(const MyBigNum& other) {
        BN_CTX* ctx = BN_CTX_new();
        BN_div(this->_bn, nullptr, this->_bn, other._bn, ctx);
        BN_CTX_free(ctx);
        return *this;
    }

    friend MyBigNum operator+(const MyBigNum& a, const MyBigNum& b) {
        MyBigNum result(a);
        result += b;
        return result;
    }

    friend MyBigNum operator-(const MyBigNum& a, const MyBigNum& b) {
        MyBigNum result(a);
        result -= b;
        return result;
    }

    friend MyBigNum operator*(const MyBigNum& a, const MyBigNum& b) {
        MyBigNum result(a);
        result *= b;
        return result;
    }

    friend MyBigNum operator/(const MyBigNum& a, const MyBigNum& b) {
        MyBigNum result(a);
        result /= b;
        return result;
    }

    friend bool operator==(const MyBigNum& a, const MyBigNum& b) {
        return BN_cmp(a._bn, b._bn) == 0;
    }

    friend bool operator!=(const MyBigNum& a, const MyBigNum& b) {
        return !(a == b);
    }

    friend bool operator<(const MyBigNum& a, const MyBigNum& b) {
        return BN_cmp(a._bn, b._bn) < 0;
    }

    friend bool operator<=(const MyBigNum& a, const MyBigNum& b) {
        return BN_cmp(a._bn, b._bn) <= 0;
    }

    friend bool operator>(const MyBigNum& a, const MyBigNum& b) {
        return BN_cmp(a._bn, b._bn) > 0;
    }

    friend bool operator>=(const MyBigNum& a, const MyBigNum& b) {
        return BN_cmp(a._bn, b._bn) >= 0;
    }

    std::string to_string(int base = 10) const {
        char* str = BN_bn2dec(this->_bn);
        std::string result(str);
        OPENSSL_free(str);
        return result;
    }
    operator char const *() const { return this->to_string().c_str(); }
};


int test_openssl_main(int argc, char ** argv)
{
	MyBigNum a("123456"), b("7890"), c = a + b;
	hp_log(stdout, "%s:BIGNUM + BIGNUM: %s + %s = %s\n", __FUNCTION__,
			a.to_string().c_str(), b.to_string().c_str(),
			(char const *)c);

	return 0;
}

#endif //
